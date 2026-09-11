#include "PluginProcessor.h"
#include "UI/PluginEditor.h"
using namespace rv;
juce::AudioProcessorValueTreeState::ParameterLayout RVocoderProcessor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for (std::size_t i = 0; i < parameterCount; ++i)
    {
        const auto &d = definitions[i];
        float initial = d.initial;
#if RV_MIDI_EDITION
        if (i == index(P::route))
            initial = 3;
        if (i == index(P::synthMode))
            initial = 1;
#endif
        // Physical values are retained even for choices such as octave (-2..2).
        juce::NormalisableRange<float> range(d.min, d.max, d.choices[0] ? 1.0f : 0.0f, d.skew);
        if (i == index(P::rootNote) || i == index(P::bitDepth))
            range.interval = 1;
        const auto choices = juce::StringArray::fromTokens(d.choices, "|", "");
        auto attrs = juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [choices, d](float v, int)
            {
                if (choices.size() > 0)
                    return choices[juce::jlimit(0, choices.size() - 1,
                                                static_cast<int>(std::round(v - d.min)))];
                return juce::String(v, std::abs(v) >= 100 ? 0 : 1);
            });
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(d.id, 1), d.name, range,
                                                               initial, attrs));
    }
    return result;
}
RVocoderProcessor::RVocoderProcessor()
    : AudioProcessor(BusesProperties()
#if RV_MIDI_EDITION
                         // In Logic's instrument slot the Side Chain selector feeds
                         // AU input element zero, not a second effect input bus.
                         .withInput("Voice Sidechain", juce::AudioChannelSet::stereo(), true)
#else
                         .withInput("Main Input", juce::AudioChannelSet::stereo(), true)
                         .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)
#endif
                         .withOutput("Main Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "RVocoderState", layout())
{
    for (std::size_t i = 0; i < parameterCount; ++i)
        raw[i] = apvts.getRawParameterValue(definitions[i].id);
    apvts.state.setProperty("schema", 1, nullptr);
    apvts.state.setProperty("presetName", "Init / Ready to sing", nullptr);
    apvts.state.setProperty("presetId", "init", nullptr);
    keyboard.addListener(this);
    setLatencySamples(rv::dsp::VocoderEngine::latency);
}
RVocoderProcessor::~RVocoderProcessor()
{
    keyboard.removeListener(this);
}
bool RVocoderProcessor::isBusesLayoutSupported(const BusesLayout &b) const
{
    if (b.inputBuses.size() != (midiEdition ? 1 : 2) || b.outputBuses.size() != 1)
        return false;
    const auto main = b.getMainInputChannelSet(), out = b.getMainOutputChannelSet();
    const auto mono = juce::AudioChannelSet::mono(), stereo = juce::AudioChannelSet::stereo();
    if (out != mono && out != stereo)
        return false;
    if (main != mono && main != stereo)
        return false;
    if (main == stereo && out == mono)
        return false;
    if (midiEdition)
        return true;
    const auto sc = b.getChannelSet(true, 1);
    return sc.isDisabled() || sc == mono || sc == stereo;
}
Params RVocoderProcessor::readParameters() const
{
    Params p;
    for (std::size_t i = 0; i < parameterCount; ++i)
        p.values[i] = raw[i]->load(std::memory_order_relaxed);
    p.sanitize();
    if (midiEdition)
        p[P::route] = 3;
    return p;
}
void RVocoderProcessor::prepareToPlay(double sampleRate, int)
{
    snapshot = readParameters();
    engine.prepare(sampleRate, snapshot);
    midiMonitor.reset();
    setLatencySamples(rv::dsp::VocoderEngine::latency);
}
void RVocoderProcessor::processBlock(juce::AudioBuffer<float> &b, juce::MidiBuffer &m)
{
    process(b, m, false);
}
void RVocoderProcessor::processBlockBypassed(juce::AudioBuffer<float> &b, juce::MidiBuffer &m)
{
    process(b, m, true);
}
void RVocoderProcessor::process(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midi, bool bypassed)
{
    juce::ScopedNoDenormals noDenormals;
    const auto before = stateSequence.load(std::memory_order_acquire);
    if ((before & 1u) == 0)
    {
        auto fresh = readParameters();
        if (stateSequence.load(std::memory_order_acquire) == before)
            snapshot = fresh;
    }
    auto p = snapshot;
    if (bypassed)
        p[P::bypass] = 1;
    double bpm = 120, ppq = -1;
    bool playing = false;
    if (auto *playhead = getPlayHead())
        if (const auto position = playhead->getPosition())
        {
            if (auto v = position->getBpm())
                bpm = *v;
            if (auto v = position->getPpqPosition())
                ppq = *v;
            playing = position->getIsPlaying();
        }
    engine.beginBlock(p, bpm, ppq, playing);
    if (midiOverflow.exchange(false))
    {
        const std::uint8_t panic[]{0xb0, 120, 0};
        for (int c = 0; c < 16; ++c)
        {
            std::uint8_t msg[]{static_cast<std::uint8_t>(panic[0] + c), 120, 0};
            dispatchMidi(msg, 3);
        }
    }
    auto read = readPos.load(std::memory_order_relaxed);
    const auto end = writePos.load(std::memory_order_acquire);
    while (read != end)
    {
        dispatchMidi(uiMidi[read % uiMidi.size()].data, 3);
        ++read;
    }
    readPos.store(read, std::memory_order_release);
    if (buffer.getNumChannels() < std::max(getTotalNumInputChannels(), getTotalNumOutputChannels()))
    {
        // A mismatched host buffer cannot satisfy the negotiated bus layout.
        // Keep MIDI state coherent, but never index missing audio channels.
        for (const auto event : midi)
            dispatchMidi(event.data, event.numBytes);
        midi.clear();
        buffer.clear();
        meters.input.store(0);
        meters.modulator.store(0);
        meters.carrier.store(0);
        meters.output.store(0);
        return;
    }
    auto main = getBusBuffer(buffer, true, 0);
    auto output = getBusBuffer(buffer, false, 0);
    auto sc = getBusBuffer(buffer, true, midiEdition ? 0 : 1);
    const int ins = main.getNumChannels(), outs = output.getNumChannels(), sides = sc.getNumChannels();
    auto event = midi.cbegin();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        while (event != midi.cend() && (*event).samplePosition <= i)
        {
            const auto e = *event;
            dispatchMidi(e.data, e.numBytes);
            ++event;
        }
        const rv::dsp::Stereo x{ins > 0 ? main.getSample(0, i) : 0,
                                ins > 1 ? main.getSample(1, i) : (ins > 0 ? main.getSample(0, i) : 0)};
        const rv::dsp::Stereo s{sides > 0 ? sc.getSample(0, i) : 0,
                                sides > 1 ? sc.getSample(1, i) : (sides > 0 ? sc.getSample(0, i) : 0)};
        const auto y = engine.process(x, s, sides > 0);
        if (outs == 1)
            output.setSample(0, i, (y.l + y.r) * 0.5f);
        else if (outs >= 2)
        {
            output.setSample(0, i, y.l);
            output.setSample(1, i, y.r);
        }
    }
    // Hosts may deliver control/MIDI in an empty audio block. Keep those notes,
    // including note-off, rather than clearing them without dispatching.
    while (event != midi.cend())
    {
        const auto e = *event;
        dispatchMidi(e.data, e.numBytes);
        ++event;
    }
    engine.publish(meters);
    midi.clear();
}
void RVocoderProcessor::dispatchMidi(const std::uint8_t *data, int length)
{
    midiMonitor.observe(data, length);
    engine.midi(data, length);
}
void RVocoderProcessor::loadPreset(const Preset &preset, bool preserveRouting)
{
    const juce::ScopedLock lock(stateLock);
    auto p = preset.parameters;
    const auto old = readParameters();
    if (preserveRouting)
        for (auto id : {P::route, P::voiceMode, P::synthMode, P::inputGain, P::outputGain, P::bypass})
            p[id] = old[id];
    if (midiEdition)
        p[P::route] = 3;
    stateSequence.fetch_add(1, std::memory_order_acq_rel);
    for (std::size_t i = 0; i < parameterCount; ++i)
    {
        auto *parameter = apvts.getParameter(definitions[i].id);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(p.values[i]));
        parameter->endChangeGesture();
    }
    apvts.state.setProperty("presetName", juce::String(preset.name), nullptr);
    apvts.state.setProperty("presetId", juce::String(preset.id), nullptr);
    for (std::size_t i = 0; i < factory.size(); ++i)
        if (factory[i].id == preset.id)
            currentProgram.store(static_cast<int>(i));
    stateSequence.fetch_add(1, std::memory_order_release);
}
void RVocoderProcessor::setRouting(Routing routing)
{
    const juce::ScopedLock lock(stateLock);
    auto p = readParameters();
    if (midiEdition)
    {
        routing.input = VoiceInput::sidechain;
        if (routing.sound == SoundSource::external)
            routing.sound = SoundSource::synth;
    }
    routing.apply(p);
    stateSequence.fetch_add(1, std::memory_order_acq_rel);
    for (auto id : {P::route, P::voiceMode})
    {
        auto *parameter = apvts.getParameter(definitions[index(id)].id);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(p[id]));
        parameter->endChangeGesture();
    }
    stateSequence.fetch_add(1, std::memory_order_release);
}
void RVocoderProcessor::resetSound()
{
    Preset init;
    init.id = "init";
    init.name = "Init / Ready to sing";
    loadPreset(init);
}
void RVocoderProcessor::setCurrentProgram(int i)
{
    if (i >= 0 && i < static_cast<int>(factory.size()))
    {
        currentProgram.store(i);
        loadPreset(factory[static_cast<std::size_t>(i)]);
    }
}
const juce::String RVocoderProcessor::getProgramName(int i)
{
    return i >= 0 && i < static_cast<int>(factory.size())
               ? juce::String(factory[static_cast<std::size_t>(i)].name)
               : juce::String{};
}
juce::String RVocoderProcessor::presetName() const
{
    const juce::ScopedLock lock(stateLock);
    return apvts.state.getProperty("presetName").toString();
}
juce::String RVocoderProcessor::presetId() const
{
    const juce::ScopedLock lock(stateLock);
    return apvts.state.getProperty("presetId").toString();
}
void RVocoderProcessor::getStateInformation(juce::MemoryBlock &dest)
{
    const juce::ScopedLock lock(stateLock);
    auto state = apvts.copyState();
    state.setProperty("program", currentProgram.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, dest);
}
void RVocoderProcessor::setStateInformation(const void *data, int size)
{
    const juce::ScopedLock lock(stateLock);
    if (size <= 0 || size > 4 * 1024 * 1024)
        return;
    if (auto xml = getXmlFromBinary(data, size))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        if (!state.hasType("RVocoderState") || static_cast<int>(state.getProperty("schema", 1)) > 1)
            return;
        // Sanitize every known value before JUCE installs it; missing IDs retain defaults.
        juce::ValueTree clean("RVocoderState");
        clean.copyPropertiesFrom(state, nullptr);
        for (const auto &d : definitions)
        {
            auto child = state.getChildWithProperty("id", d.id);
            const float value =
                child.isValid() ? static_cast<float>(child.getProperty("value", d.initial)) : d.initial;
            juce::ValueTree parameter("PARAM");
            parameter.setProperty("id", d.id, nullptr);
            parameter.setProperty("value", std::isfinite(value) ? std::clamp(value, d.min, d.max) : d.initial,
                                  nullptr);
            clean.appendChild(parameter, nullptr);
        }
        stateSequence.fetch_add(1, std::memory_order_acq_rel);
        apvts.replaceState(clean);
        currentProgram.store(juce::jlimit(0, 99, static_cast<int>(state.getProperty("program", 0))));
        stateSequence.fetch_add(1, std::memory_order_release);
    }
}
void RVocoderProcessor::enqueue(bool on, int channel, int note, float velocity)
{
    const auto w = writePos.load(std::memory_order_relaxed);
    if (w - readPos.load(std::memory_order_acquire) >= uiMidi.size())
    {
        midiOverflow.store(true);
        return;
    }
    uiMidi[w % uiMidi.size()] = {
        {static_cast<std::uint8_t>((on ? 0x90 : 0x80) | (juce::jlimit(1, 16, channel) - 1)),
         static_cast<std::uint8_t>(note),
         static_cast<std::uint8_t>(juce::jlimit(0, 127, static_cast<int>(velocity * 127)))}};
    writePos.store(w + 1, std::memory_order_release);
}
void RVocoderProcessor::handleNoteOn(juce::MidiKeyboardState *, int c, int n, float v)
{
    enqueue(true, c, n, v);
}
void RVocoderProcessor::handleNoteOff(juce::MidiKeyboardState *, int c, int n, float v)
{
    enqueue(false, c, n, v);
}
juce::AudioProcessorEditor *RVocoderProcessor::createEditor()
{
    return new RVocoderEditor(*this);
}
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new RVocoderProcessor;
}
