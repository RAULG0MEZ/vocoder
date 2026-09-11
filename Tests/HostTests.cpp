#include <cmath>
#include <iostream>
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>

namespace
{
void require(bool ok, const juce::String &message)
{
    if (!ok)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
void parameter(juce::AudioPluginInstance &plugin, const juce::String &name, float value)
{
    for (auto *p : plugin.getParameters())
        if (p->getName(128) == name)
        {
            p->setValueNotifyingHost(value);
            return;
        }
    require(false, "Parameter not found: " + name);
}
} // namespace

// Loads the delivered wrappers, without linking the plugin implementation.
int main(int argc, char **argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    require(argc > 1, "Pass the absolute paths of the VST3/AU bundles to validate");
    juce::AudioPluginFormatManager formats;
    formats.addFormat(std::make_unique<juce::VST3PluginFormatHeadless>());
#if JUCE_PLUGINHOST_AU
    formats.addFormat(std::make_unique<juce::AudioUnitPluginFormatHeadless>());
#endif
    for (int arg = 1; arg < argc; ++arg)
    {
        juce::OwnedArray<juce::PluginDescription> descriptions;
        for (auto *format : formats.getFormats())
            format->findAllTypesForFile(descriptions, argv[arg]);
        require(descriptions.size() == 1, "Exactly one plugin found: " + juce::String(argv[arg]));
        for (double sampleRate : {44100., 48000., 88200., 96000.})
        {
            juce::String error;
            auto plugin = formats.createPluginInstance(*descriptions[0], sampleRate, 256, error);
            require(plugin != nullptr, "Load plugin: " + error);
            const bool midiEdition = descriptions[0]->name.contains("MIDI");
            require(plugin->getBusCount(true) == (midiEdition ? 1 : 2),
                    "MIDI AU has one voice input; effect exposes main and auxiliary buses");
            auto buses = plugin->getBusesLayout();
            buses.inputBuses.set(0, juce::AudioChannelSet::stereo());
            if (!midiEdition)
                buses.inputBuses.set(1, juce::AudioChannelSet::stereo());
            buses.outputBuses.set(0, juce::AudioChannelSet::stereo());
            require(plugin->setBusesLayout(buses), "Enable external sidechain");
            parameter(*plugin, "Mix", 0);
            const int before = plugin->getLatencySamples();
            plugin->prepareToPlay(sampleRate, 256);
            require(plugin->getLatencySamples() == 16, "Host latency after prepare is 16 samples");
            const int voiceChannel = midiEdition ? 0 : 2;
            juce::AudioBuffer<float> audio(midiEdition ? 2 : 4, 256);
            juce::MidiBuffer midi;
            audio.clear();
            audio.setSample(0, 0, .25f);
            audio.setSample(1, 0, .25f);
            if (!midiEdition)
                audio.setSample(2, 0, .5f);
            plugin->processBlock(audio, midi);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 256; ++i)
                    require(std::abs(audio.getSample(c, i) - (i == 16 ? .25f : 0.f)) < 1e-5f,
                            "Dry impulse delay agrees with reported host latency");
            plugin->releaseResources();

            parameter(*plugin, "Mix", 1);
            parameter(*plugin, "Sources", 1);   // Sidechain voice + internal carrier.
            parameter(*plugin, "Play mode", 0); // Drone, including the MIDI AU edition.
            plugin->prepareToPlay(sampleRate, 256);
            double energy = 0;
            for (int block = 0; block < 80; ++block)
            {
                audio.clear();
                for (int i = 0; i < 256; ++i)
                {
                    const float voice = .3f * std::sin(float(2 * juce::MathConstants<double>::pi * 220 *
                                                             (block * 256 + i) / sampleRate));
                    audio.setSample(voiceChannel, i, voice);
                    audio.setSample(voiceChannel + 1, i, voice);
                }
                midi.clear();
                plugin->processBlock(audio, midi);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < 256; ++i)
                    {
                        const float v = audio.getSample(c, i);
                        require(std::isfinite(v) && std::abs(v) <= 1.001f, "Finite bounded sidechain output");
                        energy += v * v;
                    }
            }
            require(energy > .01, "Sidechain-only voice produces vocoded output through wrapper");
            plugin->releaseResources();
            // Exercise actual MIDI through the AU/VST3 wrapper, with voice only
            // on the input used by Logic's MIDI-controlled instrument slot.
            parameter(*plugin, "Play mode", 1);
            for (const auto *name :
                 {"Clarity", "Sibilance", "Unvoiced", "Breath", "Voice mix", "Carrier mix"})
                parameter(*plugin, name, 0);
            auto render = [&](bool sendNote, bool feedVoice)
            {
                plugin->prepareToPlay(sampleRate, 256);
                double result = 0;
                for (int block = 0; block < 100; ++block)
                {
                    audio.clear();
                    if (feedVoice)
                        for (int i = 0; i < 256; ++i)
                        {
                            const auto t = (block * 256 + i) / sampleRate;
                            const float voice =
                                .25f * float(std::sin(2 * juce::MathConstants<double>::pi * 220 * t) +
                                             .5 * std::sin(2 * juce::MathConstants<double>::pi * 660 * t));
                            audio.setSample(voiceChannel, i, voice);
                            audio.setSample(voiceChannel + 1, i, voice);
                        }
                    midi.clear();
                    if (block == 0 && sendNote)
                    {
                        midi.addEvent(juce::MidiMessage::noteOn(1, 48, juce::uint8(100)), 0);
                        midi.addEvent(juce::MidiMessage::noteOn(1, 55, juce::uint8(100)), 0);
                    }
                    plugin->processBlock(audio, midi);
                    for (int c = 0; c < 2; ++c)
                        for (int i = 0; i < 256; ++i)
                        {
                            const float value = audio.getSample(c, i);
                            require(std::isfinite(value) && std::abs(value) <= 1.001f,
                                    "MIDI and original voice render finite");
                            result += value * value;
                        }
                }
                plugin->releaseResources();
                return result;
            };
            const auto noNotes = render(false, true);
            const auto withNotes = render(true, true);
            std::cout << "MIDI probe " << descriptions[0]->pluginFormatName << " " << descriptions[0]->name
                      << ": acceptsMidi=" << plugin->acceptsMidi() << ", noNotes=" << noNotes
                      << ", withNotes=" << withNotes << '\n';
            if (plugin->acceptsMidi())
                require(noNotes < 1e-7 && withNotes > .1,
                        "MIDI chord activates vocoder through real wrapper");
            parameter(*plugin, "Original voice", 1);
            const auto original = render(false, true);
            const auto originalWithNotes = render(true, true);
            const auto noVoice = render(true, false);
            require(original > 1 && std::abs(original - originalWithNotes) < 1e-6 && noVoice < 1e-7,
                    "Original voice needs audio, never needs or adds a synthesiser waveform");
            std::cout << "PASS " << descriptions[0]->pluginFormatName << " " << descriptions[0]->name << " @ "
                      << sampleRate << " Hz: cached latency before prepare=" << before
                      << ", running latency=16, measured dry impulse=16, sidechain energy=" << energy
                      << ", MIDI chord energy=" << withNotes << ", original voice energy=" << original
                      << '\n';
        }
    }
}
