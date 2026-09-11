#pragma once
#include "UI/PluginEditor.h"

inline void workspaceChecks()
{
    using namespace rv;
    RVocoderProcessor p;
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    auto click = [&](const char *id)
    {
        auto *button = dynamic_cast<juce::TextButton *>(editor->findChildWithID(id));
        check(button != nullptr && button->onClick != nullptr, "Inspector button exists");
        button->onClick();
    };
    auto *keyboard = editor->findChildWithID("performance-keyboard");
    auto *canvas = dynamic_cast<SoundCanvas *>(editor->findChildWithID("sound-canvas"));
    check(keyboard != nullptr && canvas != nullptr, "Keyboard and sound canvas exist in the opening view");
    for (const auto size :
         {juce::Point<int>{1080, 760}, juce::Point<int>{1320, 840}, juce::Point<int>{1800, 1200}})
    {
        editor->setSize(size.x, size.y);
        for (int i = 0; i < 5; ++i)
        {
            const auto id = "detail-" + juce::String(i);
            click(id.toRawUTF8());
            check(keyboard->isVisible() && keyboard->getHeight() >= 60,
                  "Keyboard remains visible in every inspector and size");
            check(canvas->isVisible() && !canvas->getBounds().intersects(keyboard->getBounds()),
                  "Visual editing never overlaps the keyboard");
            for (auto *child : editor->getChildren())
                if (child->isVisible())
                    check(editor->getLocalBounds().contains(child->getBounds()),
                          "Every top-level control stays inside the editor");
        }
    }
    canvas->setMode(false);
    const auto before = p.readParameters();
    check(canvas->keyPressed(juce::KeyPress(juce::KeyPress::rightKey)),
          "XY accepts keyboard horizontal adjustment");
    check(p.readParameters()[P::formant] > before[P::formant],
          "XY updates the actual automated Formant parameter");
    canvas->keyPressed(juce::KeyPress(juce::KeyPress::upKey));
    check(p.readParameters()[P::character] > before[P::character],
          "XY updates the actual Character parameter");
    canvas->setMode(true);
    canvas->keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
    canvas->keyPressed(juce::KeyPress(juce::KeyPress::upKey));
    check(p.readParameters()[P::freqMin] < before[P::freqMin] &&
              p.readParameters()[P::freqMax] > before[P::freqMax],
          "Spectrum changes the filter bank bounds");
    auto mouse = [&](juce::Point<float> point, bool dragged)
    {
        const auto time = juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), point,
                                juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier), 1.f, 0.f, 0.f,
                                0.f, 0.f, canvas, canvas, time, point, time, 1, dragged);
    };
    canvas->setMode(false);
    const auto area = canvas->getLocalBounds().toFloat().reduced(26, 38);
    canvas->mouseDown(mouse(area.getCentre(), false));
    const auto destination =
        juce::Point<float>(area.getX() + area.getWidth() * .75f, area.getY() + area.getHeight() * .2f);
    canvas->mouseDrag(mouse(destination, true));
    canvas->mouseUp(mouse(destination, true));
    check(std::abs(p.readParameters()[P::formant] - 6.f) < .001f &&
              std::abs(p.readParameters()[P::character] - .8f) < .001f,
          "Dragging the XY point changes both real audio parameters with the correct axis direction");
    canvas->setMode(true);
    const auto high = juce::Point<float>(area.getRight() - 2, area.getCentreY());
    canvas->mouseDown(mouse(high, false));
    canvas->mouseDrag(mouse(area.getCentre(), true));
    canvas->mouseUp(mouse(area.getCentre(), true));
    check(p.readParameters()[P::freqMax] == 2000.f,
          "Spectrum edge dragging clamps safely to the supported frequency range");
    auto set = [&](P id, float value)
    {
        auto *param = p.apvts.getParameter(definitions[index(id)].id);
        param->setValueNotifyingHost(param->convertTo0to1(value));
    };
    set(P::midiGate, 0);
    set(P::wetOnly, 0);
    set(P::midiGateRelease, 150);
    for (int i = 0; i < 100; ++i)
    {
        p.setCurrentProgram(i);
        const auto state = p.readParameters();
        check(state[P::midiGate] == 0 && state[P::wetOnly] == 0 && state[P::midiGateRelease] == 150,
              "Preset changes preserve performance and mix preferences");
    }
    set(P::midiGate, 1);
    set(P::wetOnly, 1);
    juce::MemoryBlock saved;
    p.getStateInformation(saved);
    RVocoderProcessor restored;
    restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    check(restored.readParameters()[P::midiGate] == 1 && restored.readParameters()[P::wetOnly] == 1,
          "New songs restore their gate and wet-only preferences");
    auto old = p.apvts.copyState();
    old.removeChild(old.getChildWithProperty("id", "midiGate"), nullptr);
    old.removeChild(old.getChildWithProperty("id", "wetOnly"), nullptr);
    juce::MemoryBlock legacy;
    juce::AudioProcessor::copyXmlToBinary(*old.createXml(), legacy);
    restored.setStateInformation(legacy.getData(), static_cast<int>(legacy.getSize()));
    check(restored.readParameters()[P::midiGate] == 0 && restored.readParameters()[P::wetOnly] == 0,
          "Older songs preserve their intentional original mix until the user enables the new controls");

    // Exercise the host's timestamped events with deliberate dry voice leakage.
    p.setRateAndBufferSizeDetails(48000, 256);
    set(P::synthMode, 1);
    set(P::midiGate, 1);
    set(P::wetOnly, 0);
    set(P::mix, 0);
    set(P::gateOn, 0);
    set(P::route, 0);
    p.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> audio(2, 256);
    juce::MidiBuffer midi;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < 256; ++i)
            audio.setSample(c, i, .2f);
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, juce::uint8(100)), 100);
    p.processBlock(audio, midi);
    check(audio.getMagnitude(0, 100) == 0,
          "Host MIDI gate opens at the event offset, never at the beginning of the block");
    check(audio.getMagnitude(101, 155) > .01f, "Host MIDI event opens output within the same block");
    p.releaseResources();
    check(p.meters.output.load() == 0 && p.meters.carrier.load() == 0 && !p.meters.midiHeld.load(),
          "Stopped audio clears output and note-state meters instead of showing stale activity");
}
