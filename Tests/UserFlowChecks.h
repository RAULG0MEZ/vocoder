#pragma once
#include "UI/MidiKeyboard.h"

inline void userFlowChecks()
{
    using namespace rv;
    RVocoderProcessor processor;
    processor.setRateAndBufferSizeDetails(48000, 256);
    auto set = [&](P id, float value)
    {
        auto *p = processor.apvts.getParameter(definitions[index(id)].id);
        p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    set(P::synthMode, 1);
    processor.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> audio(2, 256);
    juce::MidiBuffer midi;
    midi.ensureSize(1024);
    auto event = [&](juce::MidiMessage message, bool empty = false)
    {
        audio.setSize(2, empty ? 0 : 256, false, true, true);
        audio.clear();
        midi.clear();
        midi.addEvent(message, 0);
        const auto before = allocationCount;
        watchAllocations = true;
        processor.processBlock(audio, midi);
        watchAllocations = false;
        check(allocationCount == before,
              "MIDI dispatch and keyboard telemetry allocate no audio-thread memory");
    };
    RVKeyboard keys(processor.keyboard, processor.midiMonitor);
    keys.setAvailableRange(48, 84);
    keys.setLowestVisibleKey(48);
    keys.setSize(600, 70);
    keys.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0xffd7ed8d));
    auto pixel = [&]
    {
        const auto image = keys.createComponentSnapshot(keys.getLocalBounds());
        const auto rectangle = keys.getRectangleForKey(60);
        return image.getPixelAt(static_cast<int>(rectangle.getCentreX()), 60);
    };
    const auto unpressed = pixel();
    event(juce::MidiMessage::noteOn(1, 60, juce::uint8(100)));
    check(processor.midiMonitor.isDown(60), "Incoming host note reaches the keyboard monitor");
    check(pixel() != unpressed, "Incoming MIDI visibly highlights the rendered white key");
    check(!processor.keyboard.isNoteOn(1, 60), "Host display does not enqueue a duplicate UI note");
    event(juce::MidiMessage::noteOn(16, 60, juce::uint8(100)));
    event(juce::MidiMessage::noteOff(1, 60));
    check(processor.midiMonitor.isDown(60), "Releasing one channel does not hide another held note");
    event(juce::MidiMessage::noteOn(16, 60, juce::uint8(0)), true);
    check(!processor.midiMonitor.isDown(60), "Velocity-zero note-off in empty audio block clears the key");
    check(pixel() == unpressed, "Released host note restores the unpressed key drawing");
    event(juce::MidiMessage::noteOn(3, 61, juce::uint8(100)));
    event(juce::MidiMessage::allNotesOff(3));
    check(!processor.midiMonitor.isDown(61), "All-notes-off clears keyboard feedback");
    event(juce::MidiMessage::noteOn(4, 90, juce::uint8(100)));
    processor.releaseResources();
    check(!processor.midiMonitor.isDown(90), "Releasing the processor clears held-key feedback");
    juce::AudioBuffer<float> invalidHostBuffer(1, 32);
    invalidHostBuffer.clear();
    midi.clear();
    midi.addEvent(juce::MidiMessage::noteOn(2, 67, juce::uint8(100)), 0);
    processor.processBlock(invalidHostBuffer, midi);
    check(invalidHostBuffer.getMagnitude(0, 32) == 0 && processor.midiMonitor.isDown(67),
          "Unexpected host channel count fails silent without losing MIDI state");
    processor.prepareToPlay(48000, 256);
    processor.keyboard.noteOn(1, 64, .8f);
    event(juce::MidiMessage::pitchWheel(1, 8192));
    check(processor.midiMonitor.isDown(64), "On-screen keyboard still sends notes to the audio engine");
    processor.keyboard.noteOff(1, 64, 0);
    event(juce::MidiMessage::pitchWheel(1, 8192));
    check(!processor.midiMonitor.isDown(64), "On-screen key release still reaches the engine");

    processor.setRouting({VoiceInput::sidechain, SoundSource::voice});
    for (int i = 0; i < 100; ++i)
    {
        processor.setCurrentProgram(i);
        const auto routing = Routing::from(processor.readParameters());
        check(routing.input == VoiceInput::sidechain && routing.sound == SoundSource::voice,
              "Every factory preset preserves original voice and chosen input");
    }
    juce::MemoryBlock state;
    processor.getStateInformation(state);
    RVocoderProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    const auto routing = Routing::from(restored.readParameters());
    check(routing.input == VoiceInput::sidechain && routing.sound == SoundSource::voice,
          "Project restore preserves original voice and sidechain selection");
}
