#pragma once
#include "UI/PluginEditor.h"

inline void workspaceChecks()
{
    using namespace rv;
    for (const char *asset : {"chassis_png", "knob_png", "button_png", "rack_png", "glass_png"})
        check(hardware::artwork(asset).isValid(), "All five hardware materials are embedded in the plugin");
    RVocoderProcessor p;
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    auto *keyboard = editor->findChildWithID("performance-keyboard");
    auto *canvas = dynamic_cast<SoundCanvas *>(editor->findChildWithID("sound-canvas"));
    auto *presets = dynamic_cast<juce::ComboBox *>(editor->findChildWithID("preset-selector"));
    check(keyboard != nullptr && canvas != nullptr && presets != nullptr,
          "Permanent performance controls and compact preset selector exist");
    std::vector<ParameterRack *> racks;
    for (auto *child : editor->getChildren())
        if (auto *rack = dynamic_cast<ParameterRack *>(child))
            racks.push_back(rack);
    check(racks.size() == 4, "All four rack categories are visible together");
    std::array<int, parameterCount> counts{};
    std::function<void(juce::Component *, bool)> inspect = [&](juce::Component *parent, bool geometry)
    {
        check(dynamic_cast<juce::Viewport *>(parent) == nullptr &&
                  dynamic_cast<juce::ListBox *>(parent) == nullptr,
              "No scrolling panels or permanent preset browser in the editor");
        if (auto *control = dynamic_cast<ParameterControl *>(parent); control && !geometry)
            ++counts[index(control->parameterId())];
        for (auto *child : parent->getChildren())
        {
            if (geometry && child->isVisible())
                check(parent->getLocalBounds().contains(child->getBounds()),
                      "Every visible control fits its panel without clipping");
            if (!geometry || child->isVisible())
                inspect(child, geometry);
        }
    };
    inspect(editor.get(), false);
    for (std::size_t i = 0; i < parameterCount; ++i)
    {
        const auto id = static_cast<P>(i);
        const bool global = id == P::route || id == P::bypass || id == P::voiceMode || id == P::synthMode ||
                            id == P::midiGate || id == P::wetOnly;
        check(counts[i] == (global ? 0 : 1),
              "Each existing parameter has exactly one permanent control or a global selector");
    }
    for (const auto size :
         {juce::Point<int>{1080, 760}, juce::Point<int>{1320, 840}, juce::Point<int>{1800, 1100}})
    {
        editor->setSize(size.x, size.y);
        for (auto *rack : racks)
            for (int page = 0; page < rack->pageCount(); ++page)
            {
                rack->selectPage(page);
                inspect(editor.get(), true);
                check(keyboard->isVisible() && keyboard->getHeight() >= 60,
                      "Keyboard remains visible for every rack page and size");
                check(canvas->isVisible() && !canvas->getBounds().intersects(keyboard->getBounds()),
                      "Graph never replaces or overlaps the keyboard");
            }
    }
    for (auto *rack : racks)
        rack->selectPage(0);
    auto *next = dynamic_cast<juce::TextButton *>(editor->findChildWithID("preset-next"));
    auto *prev = dynamic_cast<juce::TextButton *>(editor->findChildWithID("preset-prev"));
    check(next && prev, "Preset arrows exist");
    p.setCurrentProgram(0);
    next->onClick();
    check(p.presetId().toStdString() == p.presets.all()[1].id, "Next preset loads the adjacent sound");
    prev->onClick();
    check(p.presetId().toStdString() == p.presets.all()[0].id, "Previous returns to the original preset");
    prev->onClick();
    check(p.presetId().toStdString() == p.presets.all().back().id, "Preset arrows wrap around the library");
    presets->setSelectedId(11, juce::dontSendNotification);
    presets->onChange();
    check(p.presetId().toStdString() == p.presets.all()[10].id,
          "Categorized dropdown loads the selected preset");
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
