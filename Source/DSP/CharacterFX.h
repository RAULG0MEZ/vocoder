#pragma once
#include "../Parameters.h"
#include "Primitives.h"
namespace rv::dsp
{
// Two 33-tap linear-phase half-band FIRs at 2x. Group delay = 16 base-rate samples.
class CharacterFX
{
    struct FIR
    {
        std::array<float, 33> history{};
        int pos = 0;
        float tick(float x, const std::array<float, 33> &c)
        {
            history[static_cast<std::size_t>(pos)] = x;
            float y = 0;
            int j = pos;
            for (int i = 0; i < 33; ++i)
            {
                y += history[static_cast<std::size_t>(j)] * c[static_cast<std::size_t>(i)];
                if (--j < 0)
                    j = 32;
            }
            if (++pos == 33)
                pos = 0;
            return y;
        }
    };
    struct Channel
    {
        FIR up, down;
        std::array<SVF, 5> tone;
        SVF speakerHP, speakerLP, vintageLP;
        float held = 0;
        int hold = 0;
    };

  public:
    static constexpr int latency = 16;
    void prepare(double rate)
    {
        sr = rate;
        float sum = 0;
        for (int i = 0; i < 33; ++i)
        {
            const int n = i - 16;
            const float sinc =
                n == 0 ? 0.5f : std::sin(pi * 0.5f * static_cast<float>(n)) / (pi * static_cast<float>(n));
            const float w = 0.42f - 0.5f * std::cos(2 * pi * static_cast<float>(i) / 32) +
                            0.08f * std::cos(4 * pi * static_cast<float>(i) / 32);
            coeffs[static_cast<std::size_t>(i)] = sinc * w;
            sum += sinc * w;
        }
        for (auto &c : coeffs)
            c /= sum;
        reset();
    }
    void reset()
    {
        channels = {};
        random.state = 0x913722abu;
    }
    void configure(const Params &p)
    {
        params = p;
        const std::array<float, 5> freqs{130, 380, 1200, 3600, 8500};
        const std::array<P, 5> ids{P::low, P::body, P::mid, P::presence, P::air};
        for (int j = 0; j < 5; ++j)
            gains[static_cast<std::size_t>(j)] = dbGain(p[ids[static_cast<std::size_t>(j)]]) - 1;
        for (auto &c : channels)
        {
            for (int j = 0; j < 5; ++j)
                c.tone[static_cast<std::size_t>(j)].set(sr, freqs[static_cast<std::size_t>(j)], 0.7f);
            c.speakerHP.set(sr, 420, 0.65f);
            c.speakerLP.set(sr, 3300, 0.7f);
            c.vintageLP.set(sr, lerp(17000, 4500, p[P::vintage]), 0.7f);
        }
        saturation = 1 + p[P::drive] * 5 + p[P::warmth] * 1.8f + p[P::character] * 0.8f;
        norm = 1 / std::tanh(saturation);
        quant = std::exp2(std::round(p[P::bitDepth]) - 1);
        holdPeriod = std::max(1, static_cast<int>(std::round(p[P::reduction])));
    }
    Stereo process(Stereo in, float envelope)
    {
        return {channel(in.l, channels[0], envelope), channel(in.r, channels[1], envelope)};
    }

  private:
    float shape(float x) const
    {
        const float soft = std::tanh(x * saturation) * norm;
        const float hard = std::clamp(x * (1 + params[P::distortion] * 18), -1.0f, 1.0f) * 0.65f;
        return lerp(soft, hard, params[P::distortion]) * 0.8f;
    }
    float channel(float x, Channel &c, float envelope)
    {
        x += random.bipolar() * params[P::noise] * envelope * 0.06f;
        const float original = x;
        float eq = 0;
        for (int i = 0; i < 5; ++i)
        {
            auto o = c.tone[static_cast<std::size_t>(i)].tick(original);
            eq += gains[static_cast<std::size_t>(i)] * (i == 0 ? o.low : (i == 4 ? o.high : o.band));
        }
        x += eq;
        const float vintage = c.vintageLP.low(x);
        x = lerp(x, vintage, params[P::vintage] * 0.8f);
        x = lerp(x, c.speakerLP.low(c.speakerHP.high(x)) * 2.0f, params[P::speaker]);
        if (c.hold-- <= 0)
        {
            c.held = std::round(x * quant) / quant;
            c.hold = holdPeriod - 1;
        }
        x = lerp(x, c.held, params[P::crusher]);
        const float hi = x - vintage;
        x += params[P::exciter] * hi * std::abs(hi) * 2;
        const float up0 = c.up.tick(x * 2, coeffs);
        const float y = c.down.tick(shape(up0), coeffs);
        const float up1 = c.up.tick(0, coeffs);
        c.down.tick(shape(up1), coeffs);
        return y;
    }
    double sr = 48000;
    Params params;
    std::array<float, 33> coeffs{};
    std::array<float, 5> gains{};
    std::array<Channel, 2> channels{};
    Random random;
    float saturation = 1, norm = 1, quant = 2048;
    int holdPeriod = 1;
};
} // namespace rv::dsp
