#pragma once
#include "../Parameters.h"
#include "Primitives.h"
namespace rv::dsp
{
class SynthCarrier
{
    struct Voice
    {
        int note = -1, channel = 1, stage = 0;
        bool keyDown = false;
        float velocity = 0, env = 0, hz = 220;
        std::array<float, 7> phases{};
    };

  public:
    void prepare(double rate)
    {
        sr = rate;
        switchCoefficient = coefficient(4, sr);
        reset();
    }
    void reset()
    {
        voices = {};
        drone = {};
        sustain.fill(false);
        bend.fill(0);
        left.reset();
        right.reset();
        random.state = 0x6d2b79f5u;
        waveFade = 1;
        oldWave = wave;
        midiGain = 0;
    }
    void midi(const std::uint8_t *data, int size)
    {
        if (size < 1)
            return;
        const int type = data[0] & 0xf0, ch = data[0] & 15;
        if (size >= 3 && type == 0x90 && data[2] > 0)
        {
            Voice *selected = nullptr;
            for (auto &v : voices)
                if (v.note == data[1] && v.channel == ch)
                {
                    selected = &v;
                    break;
                }
            if (!selected)
                for (auto &v : voices)
                    if (v.stage == 0)
                    {
                        selected = &v;
                        break;
                    }
            if (!selected)
                selected = &*std::min_element(voices.begin(), voices.end(),
                                              [](const auto &a, const auto &b) { return a.env < b.env; });
            selected->note = data[1];
            selected->channel = ch;
            selected->velocity = static_cast<float>(data[2]) / 127;
            selected->keyDown = true;
            selected->stage = 1;
            if (glideCoeff == 0)
                selected->hz = frequency(static_cast<float>(data[1]));
        }
        else if (size >= 3 && (type == 0x80 || (type == 0x90 && data[2] == 0)))
        {
            for (auto &v : voices)
                if (v.note == data[1] && v.channel == ch)
                {
                    v.keyDown = false;
                    if (!sustain[static_cast<std::size_t>(ch)])
                        v.stage = 4;
                }
        }
        else if (size >= 3 && type == 0xb0)
        {
            if (data[1] == 64 || data[1] == 121)
            {
                sustain[static_cast<std::size_t>(ch)] = data[1] == 64 && data[2] >= 64;
                if (data[1] == 121)
                    bend[static_cast<std::size_t>(ch)] = 0;
                for (auto &v : voices)
                    if (v.channel == ch && !v.keyDown && !sustain[static_cast<std::size_t>(ch)] &&
                        v.stage != 0)
                        v.stage = 4;
            }
            if (data[1] == 120 || data[1] == 123)
            {
                sustain[static_cast<std::size_t>(ch)] = false;
                for (auto &v : voices)
                    if (v.channel == ch)
                    {
                        v.keyDown = false;
                        v.stage = data[1] == 120 ? 0 : 4;
                        if (data[1] == 120)
                            v.env = 0;
                    }
            }
        }
        else if (size >= 3 && type == 0xe0)
            bend[static_cast<std::size_t>(ch)] =
                2.0f * (static_cast<float>(data[1] + 128 * data[2]) - 8192) / 8192;
    }
    void configure(const Params &p, float filterMod)
    {
        params = p;
        const int nextWave = static_cast<int>(p[P::wave]);
        if (nextWave != wave)
        {
            oldWave = wave;
            wave = nextWave;
            waveFade = 0;
        }
        numUnison = std::clamp(static_cast<int>(p[P::unison]), 1, 7);
        if (wave == 5)
            numUnison = std::max(numUnison, 3);
        attack = coefficient(p[P::synthAttack] * 0.22f, sr);
        decay = coefficient(p[P::synthDecay] * 0.22f, sr);
        release = coefficient(p[P::synthRelease] * 0.15f, sr);
        glideCoeff = coefficient(std::max(3.0f, p[P::glide]), sr);
        for (int i = 0; i < numUnison; ++i)
        {
            const float pos =
                numUnison > 1 ? 2.0f * static_cast<float>(i) / static_cast<float>(numUnison - 1) - 1 : 0;
            detunes[static_cast<std::size_t>(i)] = std::exp2(pos * p[P::detune] / 1200);
            const float pan = pos * p[P::unisonWidth];
            pans[static_cast<std::size_t>(i)] = {std::sqrt(1 - pan), std::sqrt(1 + pan)};
        }
        left.set(sr, p[P::synthFilter] * std::exp2(filterMod * 3), 0.7f + p[P::resonance] * 3);
        right = copyCoefficients(left, right);
        static constexpr int intervals[5][3] = {{0, 0, 0}, {0, 7, 0}, {0, 3, 7}, {0, 4, 7}, {0, 12, -12}};
        droneCount = p[P::chord] < 0.5f ? 1 : (p[P::chord] < 1.5f ? 2 : 3);
        for (int i = 0; i < 3; ++i)
        {
            auto &v = drone[static_cast<std::size_t>(i)];
            v.note = static_cast<int>(p[P::rootNote]) +
                     intervals[std::clamp(static_cast<int>(p[P::chord]), 0, 4)][i];
            v.velocity = 0.8f;
            v.keyDown = i < droneCount && p[P::synthMode] < 0.5f;
            if (v.keyDown && v.stage == 0)
                v.stage = 1;
            if (!v.keyDown && v.stage != 0)
                v.stage = 4;
        }
        for (std::size_t i = 0; i < voices.size(); ++i)
            targets[i] = frequency(static_cast<float>(std::max(0, voices[i].note)) + p[P::octave] * 12 +
                                   bend[static_cast<std::size_t>(voices[i].channel)]);
        for (std::size_t i = 0; i < drone.size(); ++i)
            droneTargets[i] = frequency(static_cast<float>(drone[i].note) + p[P::octave] * 12);
    }
    Stereo process()
    {
        Stereo out;
        int active = 0;
        waveFade = std::min(1.0f, waveFade + 1.0f / static_cast<float>(sr * 0.01));
        const float midiTarget = params[P::synthMode] > 0.5f ? 1.0f : 0.0f;
        midiGain = midiTarget + switchCoefficient * (midiGain - midiTarget);
        for (std::size_t i = 0; i < voices.size(); ++i)
            if (voices[i].stage != 0)
            {
                auto y = render(voices[i], targets[i]);
                out = out + y * midiGain;
                ++active;
            }
        for (std::size_t i = 0; i < drone.size(); ++i)
            if (drone[i].stage != 0)
            {
                out = out + render(drone[i], droneTargets[i]);
                ++active;
            }
        // Fixed normalization avoids gain pumping when a note is released.
        (void)active;
        return {left.low(out.l) * 0.32f, right.low(out.r) * 0.32f};
    }
    int activeVoices() const
    {
        int n = 0;
        for (const auto &v : voices)
            if (v.stage != 0)
                ++n;
        return n;
    }

  private:
    static SVF copyCoefficients(const SVF &from, SVF to)
    {
        to.a1 = from.a1;
        to.a2 = from.a2;
        to.a3 = from.a3;
        to.k = from.k;
        return to;
    }
    static float frequency(float note)
    {
        return 440 * std::exp2((note - 69) / 12);
    }
    static float blep(float t, float dt)
    {
        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1;
        }
        if (t > 1 - dt)
        {
            t = (t - 1) / dt;
            return t * t + t + t + 1;
        }
        return 0;
    }
    float oscillator(float phase, float step, int shape)
    {
        switch (shape)
        {
        case 0:
        case 5:
            return 2 * phase - 1 - blep(phase, step);
        case 1:
        case 4:
        {
            const float duty = shape == 4 ? 0.28f : 0.5f;
            float t = phase - duty;
            if (t < 0)
                t += 1;
            return (phase < duty ? 1.0f : -1.0f) + blep(phase, step) - blep(t, step) - (2 * duty - 1);
        }
        case 2:
            return (8 / (pi * pi)) *
                   (std::sin(2 * pi * phase) - (step < 0.15f ? std::sin(6 * pi * phase) / 9 : 0) +
                    (step < 0.09f ? std::sin(10 * pi * phase) / 25 : 0));
        case 3:
            return std::sin(2 * pi * phase);
        default:
            return random.bipolar();
        }
    }
    Stereo render(Voice &v, float target)
    {
        if (v.stage == 1)
        {
            v.env = 1 + attack * (v.env - 1);
            if (v.env >= 0.999f)
            {
                v.env = 1;
                v.stage = 2;
            }
        }
        else if (v.stage == 2)
        {
            v.env = params[P::synthSustain] + decay * (v.env - params[P::synthSustain]);
            if (std::abs(v.env - params[P::synthSustain]) < 0.001f)
                v.stage = 3;
        }
        else if (v.stage == 3)
            v.env = params[P::synthSustain];
        else if (v.stage == 4)
        {
            v.env *= release;
            if (v.env < 1e-5f)
            {
                v.env = 0;
                v.stage = 0;
            }
        }
        v.hz = target + glideCoeff * (v.hz - target);
        Stereo out;
        for (int i = 0; i < numUnison; ++i)
        {
            auto &phase = v.phases[static_cast<std::size_t>(i)];
            const float step = std::clamp(
                v.hz * detunes[static_cast<std::size_t>(i)] / static_cast<float>(sr), 0.00001f, 0.45f);
            float osc = oscillator(phase, step, wave);
            if (waveFade < 1)
                osc = lerp(oscillator(phase, step, oldWave), osc, waveFade);
            const float y = lerp(osc, std::sin(2 * pi * phase), params[P::oscMix]);
            out.l += y * pans[static_cast<std::size_t>(i)].l;
            out.r += y * pans[static_cast<std::size_t>(i)].r;
            phase += step;
            if (phase >= 1)
                phase -= 1;
        }
        return out * (v.env * v.velocity / static_cast<float>(numUnison));
    }
    double sr = 48000;
    Params params;
    std::array<Voice, 12> voices{};
    std::array<Voice, 3> drone{};
    std::array<bool, 16> sustain{};
    std::array<float, 16> bend{};
    std::array<float, 12> targets{};
    std::array<float, 3> droneTargets{};
    std::array<float, 7> detunes{};
    std::array<Stereo, 7> pans{};
    float attack = 0, decay = 0, release = 0, glideCoeff = 0, switchCoefficient = 0, waveFade = 1,
          midiGain = 0;
    int wave = 5, oldWave = 5, numUnison = 3, droneCount = 1;
    SVF left, right;
    Random random;
};
} // namespace rv::dsp
