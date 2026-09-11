#include "DSP/VocoderEngine.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
using namespace rv;
using namespace rv::dsp;
int checks = 0;
void check(bool ok, const char *message)
{
    ++checks;
    if (!ok)
    {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}
float voice(int i, double sr)
{
    const float t = static_cast<float>(i / sr);
    return 0.22f * (std::sin(2 * pi * 140 * t) + 0.4f * std::sin(2 * pi * 700 * t) +
                    0.2f * std::sin(2 * pi * 2100 * t));
}
#include "OriginalVoiceChecks.h"
int main()
{
    originalVoiceChecks();
    Envelope env;
    env.set(48000, 10, 100);
    for (int i = 0; i < 480; ++i)
        env.process(1);
    check(std::abs(env.value - 0.6321f) < 0.003f, "Envelope attack time constant");
    for (int i = 0; i < 4800; ++i)
        env.process(0);
    check(std::abs(env.value - 0.2325f) < 0.003f, "Envelope release time constant");
    Gate gate;
    Params p;
    gate.prepare(48000);
    gate.configure(p);
    for (int i = 0; i < 4800; ++i)
        gate.process(0.2f);
    check(gate.getGain() > 0.99f, "Gate opens");
    for (int i = 0; i < 96000; ++i)
        gate.process(0.00001f);
    check(gate.getGain() < 0.0001f, "Gate closes on low noise");
    p[P::gateOn] = 0;
    gate.configure(p);
    for (int i = 0; i < 4800; ++i)
        gate.process(0);
    check(gate.getGain() > 0.99f, "Gate disabled");
    p = Params{};
    SVF center, off;
    center.set(48000, 1000, 5);
    off.set(48000, 1000, 5);
    double a = 0, b = 0;
    for (int i = 0; i < 24000; ++i)
    {
        const float c = center.band(std::sin(2 * pi * 1000 * static_cast<float>(i) / 48000)),
                    d = off.band(std::sin(2 * pi * 3000 * static_cast<float>(i) / 48000));
        if (i > 12000)
        {
            a += c * c;
            b += d * d;
        }
    }
    check(a > b * 40, "Band-pass selects center frequency");
    for (double sr : {44100., 48000., 88200., 96000.})
        for (int count = 0; count < 7; ++count)
        {
            VocoderEngine e;
            p = Params{};
            p[P::bands] = static_cast<float>(count);
            e.prepare(sr, p);
            e.beginBlock(p);
            for (int i = 0; i < 4096; ++i)
            {
                const auto y = e.process({0, 0}, {0, 0}, false);
                check(std::abs(y.l) < 1e-10f && std::abs(y.r) < 1e-10f, "Silent modulator stays silent");
            }
            double rms = 0;
            for (int i = 0; i < 8192; ++i)
            {
                const float x = voice(i, sr);
                const auto y = e.process({x, x}, {x, -x});
                check(std::isfinite(y.l) && std::isfinite(y.r) && std::abs(y.l) <= 1 && std::abs(y.r) <= 1,
                      "Audio finite and bounded");
                rms += y.l * y.l;
            }
            check(rms / 8192 > 1e-7, "Vocoder produces useful signal");
        }
    p = Params{};
    p[P::mix] = 0;
    VocoderEngine dry;
    dry.prepare(48000, p);
    for (int i = 0; i < 4096; ++i)
    {
        float x = voice(i, 48000);
        auto y = dry.process({x, -x}, {0, 0});
        const float delayed = i < VocoderEngine::latency ? 0 : voice(i - VocoderEngine::latency, 48000);
        check(std::abs(y.l - delayed) < 1e-6f && std::abs(y.r + delayed) < 1e-6f,
              "Dry mix latency alignment");
    }
    CharacterFX fx;
    p = Params{};
    p[P::drive] = p[P::character] = p[P::warmth] = 0;
    p[P::air] = p[P::presence] = 0;
    fx.prepare(48000);
    fx.configure(p);
    int peakIndex = 0;
    float peak = 0;
    for (int i = 0; i < 100; ++i)
    {
        const auto y = fx.process({i == 0 ? 0.0001f : 0, i == 0 ? 0.0001f : 0}, 0);
        if (std::abs(y.l) > peak)
        {
            peak = std::abs(y.l);
            peakIndex = i;
        }
    }
    check(peakIndex == VocoderEngine::latency, "Reported FIR latency matches impulse peak");
    p = Params{};
    p[P::synthMode] = 1;
    p[P::sibilance] = p[P::clarity] = p[P::breath] = p[P::unvoiced] = 0;
    VocoderEngine midi;
    midi.prepare(48000, p);
    double noNote = 0, withNote = 0;
    for (int i = 0; i < 4096; ++i)
    {
        const auto y = midi.process({voice(i, 48000), voice(i, 48000)}, {0, 0});
        noNote += y.l * y.l;
    }
    const std::uint8_t on[]{0x90, 60, 100}, offNote[]{0x80, 60, 0}, pedal[]{0xb0, 64, 127},
        pedalOff[]{0xb0, 64, 0};
    midi.midi(on, 3);
    for (int i = 0; i < 12000; ++i)
    {
        const auto y = midi.process({voice(i, 48000), voice(i, 48000)}, {0, 0});
        withNote += y.l * y.l;
    }
    check(noNote < 1e-10 && withNote > 0.01, "MIDI carrier note-on");
    midi.midi(pedal, 3);
    midi.midi(offNote, 3);
    for (int i = 0; i < 24000; ++i)
        midi.process({0, 0}, {0, 0});
    check(midi.activeVoices() == 1, "Sustain pedal holds note");
    midi.midi(pedalOff, 3);
    for (int i = 0; i < 96000; ++i)
        midi.process({0, 0}, {0, 0});
    check(midi.activeVoices() == 0, "Sustain pedal releases note");
    p = Params{};
    p[P::width] = 0;
    VocoderEngine mono;
    mono.prepare(48000, p);
    for (int i = 0; i < 5000; ++i)
    {
        const auto y = mono.process({voice(i, 48000), voice(i, 48000)}, {0, 0});
        check(std::abs(y.l - y.r) < 1e-6f, "Width zero is mono");
    }
    Random random;
    VocoderEngine stress;
    stress.prepare(48000);
    for (int block = 0; block < 400; ++block)
    {
        for (std::size_t i = 0; i < parameterCount; ++i)
        {
            auto &d = definitions[i];
            p.values[i] = d.min + (random.bipolar() + 1) * 0.5f * (d.max - d.min);
            if (d.choices[0])
                p.values[i] = std::round(p.values[i]);
        }
        stress.beginBlock(p, 80 + block % 140);
        for (int i = 0; i < 32 + block % 2048; ++i)
        {
            const auto y = stress.process({random.bipolar() * 2, random.bipolar() * 2},
                                          {random.bipolar(), random.bipolar()}, block % 2);
            check(std::isfinite(y.l) && std::isfinite(y.r), "Random automation remains finite");
        }
    }
    p = Params{};
    p[P::formant] = std::numeric_limits<float>::quiet_NaN();
    stress.beginBlock(p);
    for (int i = 0; i < 4096; ++i)
    {
        auto y = stress.process(
            {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}, {0, 0});
        check(std::isfinite(y.l) && std::isfinite(y.r), "Invalid input contained");
    }
    p = Params{};
    p[P::bands] = 6;
    VocoderEngine bench;
    bench.prepare(48000, p);
    auto start = std::chrono::steady_clock::now();
    double checksum = 0;
    for (int i = 0; i < 96000; ++i)
    {
        float x = voice(i, 48000);
        checksum += bench.process({x, x}, {0, 0}).l;
    }
    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "PASS " << checks << " checks. 64 bands, 48 kHz: " << elapsed / 2 * 100
              << "% of one core; checksum " << checksum << "\n";
}
