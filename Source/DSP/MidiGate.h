#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace rv::dsp
{
// Independent from oscillator voices: covers dry audio, consonants and FX too.
// Note state is channel-aware and survives voice stealing in the synthesiser.
class MidiGate
{
  public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        attackStep = 1.f / static_cast<float>(sr * .003);
        configure(70);
    }
    void reset(bool enabled) noexcept
    {
        held.fill(0);
        sounding.fill(0);
        sustain = 0;
        open = false;
        gain = enabled ? 0.f : 1.f;
    }
    void configure(float releaseMs) noexcept
    {
        releaseStep = 1.f / static_cast<float>(sr * std::clamp(releaseMs, 5.f, 600.f) * .001);
    }
    void midi(const std::uint8_t *data, int length) noexcept
    {
        if (length < 1)
            return;
        if (data[0] == 0xff)
        {
            held.fill(0);
            sounding.fill(0);
            sustain = 0;
        }
        else if (length >= 3 && data[1] < 128 && data[2] < 128)
        {
            const auto channel = static_cast<std::uint16_t>(1u << (data[0] & 15));
            const auto type = data[0] & 0xf0;
            const auto note = data[1];
            if (type == 0x90 && data[2] > 0)
            {
                held[note] |= channel;
                sounding[note] |= channel;
            }
            else if (type == 0x80 || (type == 0x90 && data[2] == 0))
            {
                held[note] &= static_cast<std::uint16_t>(~channel);
                if ((sustain & channel) == 0)
                    sounding[note] &= static_cast<std::uint16_t>(~channel);
            }
            else if (type == 0xb0)
            {
                if (note == 64 || note == 121)
                {
                    if (note == 64 && data[2] >= 64)
                        sustain |= channel;
                    else
                    {
                        sustain &= static_cast<std::uint16_t>(~channel);
                        for (std::size_t i = 0; i < held.size(); ++i)
                            sounding[i] =
                                static_cast<std::uint16_t>((sounding[i] & ~channel) | (held[i] & channel));
                    }
                }
                if (note == 120 || note == 123)
                {
                    sustain &= static_cast<std::uint16_t>(~channel);
                    for (std::size_t i = 0; i < held.size(); ++i)
                    {
                        held[i] &= static_cast<std::uint16_t>(~channel);
                        sounding[i] &= static_cast<std::uint16_t>(~channel);
                    }
                }
            }
        }
        open = std::any_of(sounding.begin(), sounding.end(), [](auto notes) { return notes != 0; });
    }
    float process(bool enabled) noexcept
    {
        // Finite ramps reach exact zero; release never leaks an exponential tail.
        gain = !enabled || open ? std::min(1.f, gain + attackStep) : std::max(0.f, gain - releaseStep);
        return gain;
    }
    float value() const noexcept
    {
        return gain;
    }
    bool isOpen() const noexcept
    {
        return open;
    }

  private:
    std::array<std::uint16_t, 128> held{}, sounding{};
    std::uint16_t sustain = 0;
    double sr = 48000;
    float gain = 0, attackStep = 0, releaseStep = 0;
    bool open = false;
};
} // namespace rv::dsp
