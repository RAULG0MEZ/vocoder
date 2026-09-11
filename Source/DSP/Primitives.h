#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>

namespace rv::dsp
{
inline constexpr float pi = std::numbers::pi_v<float>;
inline float dbGain(float db)
{
    return std::pow(10.0f, db * 0.05f);
}
inline float sane(float x)
{
    return std::isfinite(x) ? std::clamp(x, -32.0f, 32.0f) : 0.0f;
}
inline float coefficient(float ms, double sr)
{
    return std::exp(-1.0f / (std::max(0.05f, ms) * 0.001f * static_cast<float>(sr)));
}
inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}
struct Stereo
{
    float l = 0, r = 0;
    Stereo operator+(Stereo b) const
    {
        return {l + b.l, r + b.r};
    }
    Stereo operator*(float g) const
    {
        return {l * g, r * g};
    }
};
struct Envelope
{
    float value = 0, attack = 0, release = 0;
    void set(double sr, float a, float r)
    {
        attack = coefficient(a, sr);
        release = coefficient(r, sr);
    }
    float process(float x)
    {
        x = std::abs(x);
        value = x + (x > value ? attack : release) * (value - x);
        if (value < 1e-20f)
            value = 0;
        return value;
    }
    void reset()
    {
        value = 0;
    }
};
// Topology-preserving state-variable filter. Band output has unity peak gain.
struct SVF
{
    float a1 = 1, a2 = 0, a3 = 0, k = 1, ic1 = 0, ic2 = 0;
    void set(double sr, float hz, float q = 0.7071f)
    {
        const float g =
            std::tan(pi * std::clamp(hz, 10.0f, static_cast<float>(sr) * 0.45f) / static_cast<float>(sr));
        k = 1.0f / std::clamp(q, 0.35f, 30.0f);
        a1 = 1 / (1 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    struct Outputs
    {
        float low, band, high;
    };
    Outputs tick(float x)
    {
        const float v3 = x - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2 * v1 - ic1;
        ic2 = 2 * v2 - ic2;
        return {v2, k * v1, x - k * v1 - v2};
    }
    float band(float x)
    {
        return tick(x).band;
    }
    float low(float x)
    {
        return tick(x).low;
    }
    float high(float x)
    {
        return tick(x).high;
    }
    void reset()
    {
        ic1 = ic2 = 0;
    }
};
struct Random
{
    std::uint32_t state = 0x6d2b79f5u;
    float bipolar()
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return static_cast<float>(state >> 8) * (1.0f / 8388608.0f) - 1;
    }
};
template <int N> struct Delay
{
    std::array<float, N> memory{};
    int pos = 0;
    float process(float x)
    {
        const float y = memory[static_cast<std::size_t>(pos)];
        memory[static_cast<std::size_t>(pos)] = x;
        pos = (pos + 1) % N;
        return y;
    }
    void reset()
    {
        memory.fill(0);
        pos = 0;
    }
};
} // namespace rv::dsp
