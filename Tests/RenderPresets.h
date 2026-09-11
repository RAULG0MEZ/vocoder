#pragma once
#include "Plugin/PluginProcessor.h"
#include <fstream>
// Offline diagnostic only. No file I/O from the realtime processor.
inline int renderPresets(const juce::File &source, const juce::File &destination, bool originalVoice = false)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(source));
    if (!reader || reader->lengthInSamples < 1000)
    {
        std::cerr << "Invalid speech fixture\n";
        return 1;
    }
    destination.createDirectory();
    const int length = static_cast<int>(reader->lengthInSamples);
    juce::AudioBuffer<float> speech(1, length);
    reader->read(&speech, 0, length, 0, true, false);
    std::ofstream report(destination.getChildFile("preset-measurements.csv").getFullPathName().toStdString());
    report << "preset,category,rms_db,peak_db,correlation,dc\n";
    for (const auto &preset : rv::factoryPresets())
    {
        rv::dsp::VocoderEngine engine;
        auto parameters = preset.parameters;
        if (originalVoice)
            parameters[rv::P::voiceMode] = 1;
        engine.prepare(reader->sampleRate, parameters);
        juce::AudioBuffer<float> output(2, length + static_cast<int>(reader->sampleRate));
        double sum = 0, l2 = 0, r2 = 0, lr = 0, dc = 0;
        float peak = 0;
        for (int i = 0; i < output.getNumSamples(); ++i)
        {
            const float x = i < length ? speech.getSample(0, i) : 0;
            const auto y = engine.process({x, x}, {0, 0}, false);
            if (!std::isfinite(y.l) || !std::isfinite(y.r))
                return 2;
            output.setSample(0, i, y.l);
            output.setSample(1, i, y.r);
            sum += (y.l * y.l + y.r * y.r) * .5;
            peak = std::max(peak, std::max(std::abs(y.l), std::abs(y.r)));
            l2 += y.l * y.l;
            r2 += y.r * y.r;
            lr += y.l * y.r;
            dc += (y.l + y.r) * .5;
        }
        const double rms = std::sqrt(sum / output.getNumSamples());
        report << preset.name << ',' << preset.category << ',' << 20 * std::log10(std::max(1e-12, rms)) << ','
               << 20 * std::log10(std::max(1e-12f, peak)) << ','
               << (l2 * r2 > 0 ? lr / std::sqrt(l2 * r2) : 1) << ',' << dc / output.getNumSamples() << '\n';
        const std::array<std::string, 10> demos{
            "Classic Robot", "French Chrome", "Hyper Clean",  "Demon Machine", "Dream Vocoder",
            "Alien",         "Robot Choir",   "Deep Machine", "Intercom",      "Wide Robot"};
        if (std::find(demos.begin(), demos.end(), preset.name) != demos.end())
        {
            auto stream = destination.getChildFile(juce::String(preset.name) + ".wav").createOutputStream();
            if (!stream)
                return 3;
            stream->setPosition(0);
            stream->truncate();
            std::unique_ptr<juce::AudioFormatWriter> writer(
                juce::WavAudioFormat{}.createWriterFor(stream.release(), reader->sampleRate, 2, 24, {}, 0));
            if (!writer || !writer->writeFromAudioSampleBuffer(output, 0, output.getNumSamples()))
                return 3;
        }
    }
    return 0;
}
