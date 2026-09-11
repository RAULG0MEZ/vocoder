#pragma once
#include "Primitives.h"
namespace rv::dsp
{
class FilterBank
{
  public:
    struct Band
    {
        SVF analysis1, analysis2, left1, left2, right1, right2;
        SVF voiceLeft1, voiceLeft2, voiceRight1, voiceRight2;
        Envelope envelope;
        float frequency = 0;
    };
    void configure(double sr, int count, float low, float high, float focus, float attack, float release)
    {
        n = std::clamp(count, 8, 64);
        hi = std::clamp(high, low * 2, static_cast<float>(sr) * 0.43f);
        lo = low;
        logStep = std::log(hi / lo) / static_cast<float>(n - 1);
        const float ratio = std::exp(logStep),
                    q = std::clamp((0.65f + focus * 0.75f) / (ratio - 1.0f), 0.7f, 25.0f);
        for (int i = 0; i < n; ++i)
        {
            auto &b = bank[static_cast<std::size_t>(i)];
            b.frequency = lo * std::exp(logStep * static_cast<float>(i));
            b.analysis1.set(sr, b.frequency, q);
            b.analysis2.set(sr, b.frequency, q);
            b.left1.set(sr, b.frequency, q);
            b.left2.set(sr, b.frequency, q);
            b.right1.set(sr, b.frequency, q);
            b.right2.set(sr, b.frequency, q);
            b.voiceLeft1.set(sr, b.frequency, q);
            b.voiceLeft2.set(sr, b.frequency, q);
            b.voiceRight1.set(sr, b.frequency, q);
            b.voiceRight2.set(sr, b.frequency, q);
            b.envelope.set(sr, attack, release);
        }
    }
    void setEnvelopes(double sr, float attack, float release)
    {
        for (int i = 0; i < n; ++i)
            bank[static_cast<std::size_t>(i)].envelope.set(sr, attack, release);
    }
    void shape(float shiftSemitones, float bandShift, float tilt, float spread, float definition, float nasal,
               float throat)
    {
        offset = shiftSemitones * (0.057762265f / logStep) + bandShift;
        compression = 0.18f + definition * 0.3f;
        for (int i = 0; i < n; ++i)
        {
            const float pos = static_cast<float>(i) / static_cast<float>(n - 1),
                        hz = bank[static_cast<std::size_t>(i)].frequency;
            const float ln = std::log2(hz / 1100.0f), lt = std::log2(hz / 350.0f);
            const float gain =
                std::exp2(tilt * (pos - 0.5f) / 6) *
                std::max(0.05f, 1 + nasal * 1.5f * std::exp(-ln * ln * 3) + throat * std::exp(-lt * lt * 2));
            const float pan = spread * ((i % 2 == 0 ? -1.0f : 1.0f) * (0.35f + 0.65f * pos));
            weights[static_cast<std::size_t>(i)] = {gain * (1 - pan), gain * (1 + pan)};
        }
    }
    Stereo process(float mod, Stereo carrier, Stereo originalVoice = {}, Stereo *reshapedVoice = nullptr)
    {
        for (int i = 0; i < n; ++i)
        {
            auto &b = bank[static_cast<std::size_t>(i)];
            const float x = b.analysis2.band(b.analysis1.band(mod));
            envelopes[static_cast<std::size_t>(i)] = b.envelope.process(x);
        }
        Stereo out;
        if (reshapedVoice != nullptr)
            *reshapedVoice = originalVoice;
        for (int i = 0; i < n; ++i)
        {
            auto &b = bank[static_cast<std::size_t>(i)];
            const float source = static_cast<float>(i) - offset;
            float env = 0;
            if (source >= 0 && source <= static_cast<float>(n - 1))
            {
                const int a = static_cast<int>(source);
                env = lerp(envelopes[static_cast<std::size_t>(a)],
                           envelopes[static_cast<std::size_t>(std::min(a + 1, n - 1))],
                           source - static_cast<float>(a));
            }
            // 4-pole analysis + synthesis; compression improves weak phoneme audibility.
            const float gain = lerp(env, std::sqrt(std::max(0.0f, env)) * 0.5f, compression);
            out.l += b.left2.band(b.left1.band(carrier.l)) * gain * weights[static_cast<std::size_t>(i)].l;
            out.r += b.right2.band(b.right1.band(carrier.r)) * gain * weights[static_cast<std::size_t>(i)].r;
            if (reshapedVoice != nullptr)
            {
                // Retain the original waveform and change only the spectral balance.
                // At neutral formant/tilt/spread the correction is exactly zero.
                // A floor and a bounded ratio avoid boosting near-empty voice bands.
                const float reference = envelopes[static_cast<std::size_t>(i)];
                const float ratio = std::clamp((env + .0005f) / (reference + .0005f), .2f, 4.f);
                const auto weight = weights[static_cast<std::size_t>(i)];
                const float definition = .7f + compression * .5f;
                reshapedVoice->l += b.voiceLeft2.band(b.voiceLeft1.band(originalVoice.l)) *
                                    (std::clamp(ratio * weight.l, .1f, 5.f) - 1.f) * definition;
                reshapedVoice->r += b.voiceRight2.band(b.voiceRight1.band(originalVoice.r)) *
                                    (std::clamp(ratio * weight.r, .1f, 5.f) - 1.f) * definition;
            }
        }
        return out * 9.0f;
    }
    void reset()
    {
        for (auto &b : bank)
        {
            b.analysis1.reset();
            b.analysis2.reset();
            b.left1.reset();
            b.left2.reset();
            b.right1.reset();
            b.right2.reset();
            b.voiceLeft1.reset();
            b.voiceLeft2.reset();
            b.voiceRight1.reset();
            b.voiceRight2.reset();
            b.envelope.reset();
        }
        envelopes.fill(0);
    }
    int count() const
    {
        return n;
    }
    float frequency(int i) const
    {
        return bank[static_cast<std::size_t>(i)].frequency;
    }
    const std::array<float, 64> &activity() const
    {
        return envelopes;
    }

  private:
    std::array<Band, 64> bank{};
    std::array<float, 64> envelopes{};
    std::array<Stereo, 64> weights{};
    int n = 24;
    float lo = 90, hi = 12000, logStep = 0.2f, offset = 0, compression = 0.3f;
};
} // namespace rv::dsp
