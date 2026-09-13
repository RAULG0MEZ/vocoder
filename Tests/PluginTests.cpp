#include "Plugin/PluginProcessor.h"
#include "ProcessorChecks.h"
#include "RenderPresets.h"
#include "UserFlowChecks.h"
#include "WorkspaceChecks.h"
#include <iostream>
#include <set>
int checks = 0;
void check(bool ok, const char *what)
{
    ++checks;
    if (!ok)
    {
        std::cerr << "FAIL: " << what << "\n";
        std::exit(1);
    }
}
int main(int argc, char **argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc == 4 && (juce::String(argv[1]) == "--render" || juce::String(argv[1]) == "--render-voice"))
        return renderPresets(juce::File(argv[2]), juce::File(argv[3]),
                             juce::String(argv[1]) == "--render-voice");
    processorChecks();
    userFlowChecks();
    workspaceChecks();
    RVocoderProcessor p;
    check(p.getNumPrograms() == 100, "100 factory programs");
    std::set<std::string> names, signatures;
    for (const auto &preset : p.presets.all())
    {
        if (preset.user)
            continue;
        names.insert(preset.name);
        std::string signature;
        for (float v : preset.parameters.values)
            signature += std::to_string(v) + ",";
        signatures.insert(signature);
    }
    check(names.size() == 100 && signatures.size() == 100, "Unique factory names and parameter signatures");
    for (int channels : {1, 2})
        for (int output : {1, 2})
            for (int side : {0, 1, 2})
            {
                auto layout = p.getBusesLayout();
                layout.inputBuses.set(0, juce::AudioChannelSet::canonicalChannelSet(channels));
                layout.outputBuses.set(0, juce::AudioChannelSet::canonicalChannelSet(output));
                layout.inputBuses.set(1, juce::AudioChannelSet::canonicalChannelSet(side));
                if (channels == 2 && output == 1)
                {
                    check(!p.isBusesLayoutSupported(layout), "Reject stereo to mono layout");
                    continue;
                }
                check(p.setBusesLayout(layout), "Host bus layout accepted");
                for (double sr : {44100., 48000., 88200., 96000.})
                    for (int block : {32, 64, 128, 256, 512, 1024, 2048})
                    {
                        p.setRateAndBufferSizeDetails(sr, block);
                        p.prepareToPlay(sr, block);
                        juce::AudioBuffer<float> b(std::max(channels + side, output), block);
                        juce::MidiBuffer midi;
                        b.clear();
                        p.processBlock(b, midi);
                        for (int c = 0; c < output; ++c)
                            for (int i = 0; i < block; ++i)
                                check(std::isfinite(b.getSample(c, i)), "Bus render finite");
                    }
            }
    p.setCurrentProgram(23);
    auto *formant = p.apvts.getParameter("formant");
    formant->setValueNotifyingHost(formant->convertTo0to1(3.5f));
    juce::MemoryBlock state;
    p.getStateInformation(state);
    RVocoderProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    const auto a = p.readParameters(), b = restored.readParameters();
    for (std::size_t i = 0; i < rv::parameterCount; ++i)
        check(std::abs(a.values[i] - b.values[i]) < 1e-5f, "State parameter restore");
    check(restored.presetName() == p.presetName(), "Preset metadata restore");
    for (int i = 0; i < 100; ++i)
    {
        p.setCurrentProgram(i);
        juce::MemoryBlock data;
        p.getStateInformation(data);
        restored.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
        check(restored.getCurrentProgram() == i, "All factory states restore");
    }
    const auto testDirectory =
        juce::File::getCurrentWorkingDirectory().getChildFile("preset-test-" + juce::Uuid().toString());
    {
        rv::PresetManager manager(testDirectory);
        check(manager.save("Test / Unicode ñ", a).wasOk(), "Save user preset");
        check(manager.all().size() == 101, "User preset available");
        auto id = manager.all().back().id;
        rv::PresetManager reopened(testDirectory);
        check(reopened.all().back().parameters.values == a.values, "User preset values persist");
        check(reopened.toggleFavorite(id).wasOk(), "Favorite saved");
        rv::PresetManager again(testDirectory);
        check(again.favorite(id), "Favorite survives reopening");
        check(again.remove("factory-1").failed(), "Factory deletion refused");
        check(again.remove(id).wasOk(), "User deletion");
    }
    testDirectory.deleteRecursively();
    if (argc > 1 && (juce::String(argv[1]) == "--screenshot" || juce::String(argv[1]) == "--screenshots"))
    {
        p.setCurrentProgram(10);
        p.setRateAndBufferSizeDetails(48000, 256);
        p.prepareToPlay(48000, 256);
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        editor->setSize(1320, 840);
        auto image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 2);
        juce::FileOutputStream out(
            juce::File::getCurrentWorkingDirectory().getChildFile("build/ui-preview.png"));
        out.setPosition(0);
        out.truncate();
        juce::PNGImageFormat{}.writeImageToStream(image, out);
        out.flush();
        if (juce::String(argv[1]) == "--screenshots")
        {
            auto shot = [&](const juce::String &name)
            {
                auto snapshot = editor->createComponentSnapshot(editor->getLocalBounds(), true, 2);
                juce::FileOutputStream file(
                    juce::File::getCurrentWorkingDirectory().getChildFile("build/" + name));
                file.setPosition(0);
                file.truncate();
                juce::PNGImageFormat{}.writeImageToStream(snapshot, file);
            };
            auto click = [&](const juce::String &name)
            {
                for (auto *child : editor->getChildren())
                    if (auto *button = dynamic_cast<juce::TextButton *>(child))
                        if (button->getButtonText() == name && button->onClick)
                        {
                            button->onClick();
                            return;
                        }
                check(false, "Screenshot button exists");
            };
            juce::AudioBuffer<float> audio(
                std::max(p.getTotalNumInputChannels(), p.getTotalNumOutputChannels()), 256);
            juce::MidiBuffer notes;
            p.apvts.getParameter("synthMode")->setValueNotifyingHost(1);
            p.prepareToPlay(48000, 256);
            for (int block = 0; block < 80; ++block)
            {
                for (int i = 0; i < 256; ++i)
                    for (int c = 0; c < audio.getNumChannels(); ++c)
                        audio.setSample(c, i, .2f * std::sin(float(block * 256 + i) * .03f));
                notes.clear();
                if (block == 0)
                    for (int n : {48, 52, 55})
                        notes.addEvent(juce::MidiMessage::noteOn(1, n, juce::uint8(100)), 0);
                p.processBlock(audio, notes);
            }
            click("Synth");
            shot("ui-midi.png");
            click("Voz");
            click("Sidechain");
            shot("ui-voice.png");
            click("Espectro");
            shot("ui-spectrum.png");
            click("XY");
            editor->setSize(1080, 760);
            shot("ui-minimum.png");
            for (auto *child : editor->getChildren())
                if (auto *rack = dynamic_cast<ParameterRack *>(child))
                    rack->selectPage(rack->pageCount() - 1);
            shot("ui-advanced-minimum.png");
        }
    }
    std::cout << "PASS " << checks << " plugin integration checks\n";
}
