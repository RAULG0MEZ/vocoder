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
            require(plugin->getBusCount(true) == 2, "Main and sidechain input buses exposed");
            auto buses = plugin->getBusesLayout();
            buses.inputBuses.set(0, juce::AudioChannelSet::stereo());
            buses.inputBuses.set(1, juce::AudioChannelSet::stereo());
            buses.outputBuses.set(0, juce::AudioChannelSet::stereo());
            require(plugin->setBusesLayout(buses), "Enable external sidechain");
            parameter(*plugin, "Mix", 0);
            const int before = plugin->getLatencySamples();
            plugin->prepareToPlay(sampleRate, 256);
            require(plugin->getLatencySamples() == 16, "Host latency after prepare is 16 samples");
            juce::AudioBuffer<float> audio(4, 256);
            juce::MidiBuffer midi;
            audio.clear();
            audio.setSample(0, 0, .25f);
            audio.setSample(1, 0, .25f);
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
                    audio.setSample(2, i, voice);
                    audio.setSample(3, i, voice);
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
            std::cout << "PASS " << descriptions[0]->pluginFormatName << " " << descriptions[0]->name << " @ "
                      << sampleRate << " Hz: cached latency before prepare=" << before
                      << ", running latency=16, measured dry impulse=16, sidechain energy=" << energy << '\n';
        }
    }
}
