#pragma once
inline void originalVoiceChecks()
{
    // The neutral spectral correction must preserve the waveform, not replace
    // it with a carrier or multiply it by its own envelope.
    FilterBank neutral;
    neutral.configure(48000, 32, 90, 12000, .45f, 3, 75);
    neutral.shape(0, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 12000; ++i)
    {
        const Stereo in{voice(i, 48000), voice(i + 13, 48000)};
        Stereo shaped;
        neutral.process((in.l + in.r) * .5f, {}, in, &shaped);
        check(std::abs(shaped.l - in.l) < 1e-7f && std::abs(shaped.r - in.r) < 1e-7f,
              "Neutral original-voice shaping preserves the waveform");
    }
    Params p;
    p[P::voiceMode] = 1;
    p[P::motion] = 0;
    p[P::gateOn] = 0;
    p[P::route] = 0;
    auto alternate = p;
    alternate[P::route] = 3;
    alternate[P::wave] = 6;
    alternate[P::unison] = 7;
    alternate[P::rootNote] = 84;
    alternate[P::carMix] = 1;
    alternate[P::synthMode] = 1;
    alternate[P::presetLevel] = 24;
    VocoderEngine track, side;
    track.prepare(48000, p);
    side.prepare(48000, alternate);
    const std::uint8_t note[]{0x90, 84, 127};
    side.midi(note, 3);
    double energy = 0;
    for (int i = 0; i < 24000; ++i)
    {
        const float x = voice(i, 48000);
        const auto a = track.process({x, x}, {}), b = side.process({}, {x, x});
        check(std::abs(a.l - b.l) < 1e-6f && std::abs(a.r - b.r) < 1e-6f,
              "Original voice is independent of synth, MIDI, carrier mix and factory vocoder trim");
        energy += a.l * a.l;
    }
    check(energy > 1, "Original voice produces useful audio without MIDI");
    p[P::route] = 3;
    p[P::mix] = 0;
    p[P::inputGain] = 12;
    VocoderEngine drySidechain;
    drySidechain.prepare(48000, p);
    for (int i = 0; i < 4096; ++i)
    {
        const float x = voice(i, 48000);
        const auto y = drySidechain.process({}, {x, x});
        const float expected = i < VocoderEngine::latency ? 0 : voice(i - VocoderEngine::latency, 48000);
        check(std::abs(y.l - expected) < 1e-6f,
              "Dry mix follows selected sidechain voice at the reported latency");
    }
    p[P::mix] = 1;
    p[P::inputGain] = 0;
    p[P::formant] = 0;
    alternate = p;
    alternate[P::formant] = 7;
    track.prepare(48000, p);
    side.prepare(48000, alternate);
    double formantDifference = 0;
    for (int i = 0; i < 24000; ++i)
    {
        const float x = voice(i, 48000);
        formantDifference += std::abs(track.process({}, {x, x}).l - side.process({}, {x, x}).l);
    }
    check(formantDifference > 1, "Formant changes original-voice spectral character");
}
