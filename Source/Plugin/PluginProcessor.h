#pragma once
#include "DSP/VocoderEngine.h"
#include "MidiMonitor.h"
#include "Presets/PresetManager.h"
#include "Routing.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
class RVocoderProcessor final : public juce::AudioProcessor, private juce::MidiKeyboardState::Listener
{
  public:
    RVocoderProcessor();
    ~RVocoderProcessor() override;
    void prepareToPlay(double, int) override;
    void reset() override
    {
        engine.reset();
        midiMonitor.reset();
    }
    void releaseResources() override
    {
        midiMonitor.reset();
    }
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    void processBlockBypassed(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override
    {
        return true;
    }
    const juce::String getName() const override
    {
        return JucePlugin_Name;
    }
    bool acceptsMidi() const override
    {
        return true;
    }
    bool producesMidi() const override
    {
        return false;
    }
    bool isMidiEffect() const override
    {
        return false;
    }
    double getTailLengthSeconds() const override
    {
        return 12.0;
    }
    int getNumPrograms() override
    {
        return static_cast<int>(factory.size());
    }
    int getCurrentProgram() override
    {
        return currentProgram.load();
    }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int i) override;
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
    juce::AudioProcessorParameter *getBypassParameter() const override
    {
        return apvts.getParameter("bypass");
    }
    rv::Params readParameters() const;
    void loadPreset(const rv::Preset &, bool preserveRouting = true);
    void resetSound();
    void setRouting(rv::Routing);
    static constexpr bool midiEdition = RV_MIDI_EDITION != 0;
    juce::String presetName() const;
    juce::String presetId() const;
    juce::AudioProcessorValueTreeState apvts;
    rv::dsp::Meters meters;
    rv::PresetManager presets;
    juce::MidiKeyboardState keyboard;
    rv::MidiMonitor midiMonitor;

  private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    void process(juce::AudioBuffer<float> &, juce::MidiBuffer &, bool);
    void dispatchMidi(const std::uint8_t *, int);
    void handleNoteOn(juce::MidiKeyboardState *, int, int, float) override;
    void handleNoteOff(juce::MidiKeyboardState *, int, int, float) override;
    void enqueue(bool, int, int, float);
    std::array<std::atomic<float> *, rv::parameterCount> raw{};
    std::vector<rv::Preset> factory = rv::factoryPresets();
    std::atomic<int> currentProgram{0};
    // Serializes state writers only. The audio callback never takes this lock.
    mutable juce::CriticalSection stateLock;
    std::atomic<unsigned> stateSequence{0};
    rv::Params snapshot;
    rv::dsp::VocoderEngine engine;
    struct KeyEvent
    {
        std::uint8_t data[3]{};
    };
    std::array<KeyEvent, 128> uiMidi{};
    std::atomic<unsigned> writePos{0}, readPos{0};
    std::atomic<bool> midiOverflow{false};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVocoderProcessor)
};
