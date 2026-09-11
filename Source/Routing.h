#pragma once
#include "Parameters.h"

namespace rv
{
enum class VoiceInput
{
    track,
    sidechain
};
enum class SoundSource
{
    voice,
    synth,
    external
};

struct Routing
{
    VoiceInput input;
    SoundSource sound;
    static Routing from(const Params &p)
    {
        const int route = static_cast<int>(p[P::route]);
        return {route >= 2 ? VoiceInput::sidechain : VoiceInput::track,
                p[P::voiceMode] > .5f
                    ? SoundSource::voice
                    : (route == 1 || route == 2 ? SoundSource::external : SoundSource::synth)};
    }
    void apply(Params &p) const
    {
        const bool side = input == VoiceInput::sidechain, external = sound == SoundSource::external;
        p[P::route] = side ? (external ? 2.f : 3.f) : (external ? 1.f : 0.f);
        p[P::voiceMode] = sound == SoundSource::voice ? 1.f : 0.f;
    }
};
} // namespace rv
