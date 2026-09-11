#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace rv
{
// Append only. IDs, ranges and choice order are part of the session format.
// symbol, permanent ID, display name, min, max, default, skew, section, choices
#define RV_PARAMETERS(X)                                                                                     \
    X(route, "route", "Sources", 0, 3, 0, 1, "Routing",                                                      \
      "Voice + internal|Voice + sidechain carrier|Carrier + sidechain voice|Sidechain voice + internal")     \
    X(inputGain, "inputGain", "Input", -24, 24, 0, 1, "Routing", "")                                         \
    X(outputGain, "outputGain", "Output", -24, 12, 0, 1, "Routing", "")                                      \
    X(bypass, "bypass", "Bypass", 0, 1, 0, 1, "Routing", "Off|On")                                           \
    X(character, "character", "Character", 0, 1, 0.3, 1, "Main", "")                                         \
    X(clarity, "clarity", "Clarity", 0, 1, 0.55, 1, "Main", "")                                              \
    X(formant, "formant", "Formant", -12, 12, 0, 1, "Main", "")                                              \
    X(width, "width", "Width", 0, 2, 1, 1, "Main", "")                                                       \
    X(drive, "drive", "Drive", 0, 1, 0.12, 1, "Main", "")                                                    \
    X(motion, "motion", "Motion", 0, 1, 0, 1, "Main", "")                                                    \
    X(air, "air", "Air", -12, 12, 1, 1, "Main", "")                                                          \
    X(mix, "mix", "Mix", 0, 1, 1, 1, "Main", "")                                                             \
    X(bands, "bands", "Bands", 0, 6, 3, 1, "Analysis", "8|12|16|24|32|48|64")                                \
    X(envAttack, "envAttack", "Envelope attack", 0.5, 80, 3, 0.4, "Analysis", "")                            \
    X(envRelease, "envRelease", "Envelope release", 5, 500, 75, 0.45, "Analysis", "")                        \
    X(freqMin, "freqMin", "Low frequency", 40, 500, 90, 0.5, "Analysis", "")                                 \
    X(freqMax, "freqMax", "High frequency", 2000, 18000, 12000, 0.6, "Analysis", "")                         \
    X(tilt, "tilt", "Spectral tilt", -12, 12, 1.5, 1, "Analysis", "")                                        \
    X(bandShift, "bandShift", "Band shift", -12, 12, 0, 1, "Analysis", "")                                   \
    X(formantQ, "formantQ", "Formant focus", 0, 1, 0.45, 1, "Analysis", "")                                  \
    X(amount, "amount", "Vocoder amount", 0, 1, 1, 1, "Analysis", "")                                        \
    X(definition, "definition", "Definition", 0, 1, 0.4, 1, "Intelligibility", "")                           \
    X(sibilance, "sibilance", "Sibilance", 0, 1, 0.3, 1, "Intelligibility", "")                              \
    X(unvoiced, "unvoiced", "Unvoiced", 0, 1, 0.4, 1, "Intelligibility", "")                                 \
    X(breath, "breath", "Breath", 0, 1, 0.12, 1, "Intelligibility", "")                                      \
    X(nasal, "nasal", "Nasal", -1, 1, 0, 1, "Intelligibility", "")                                           \
    X(size, "size", "Size", -1, 1, 0, 1, "Identity", "")                                                     \
    X(identity, "identity", "Identity", -1, 1, 0, 1, "Identity", "")                                         \
    X(throat, "throat", "Throat", -1, 1, 0, 1, "Identity", "")                                               \
    X(gateOn, "gateOn", "Gate", 0, 1, 1, 1, "Gate", "Off|On")                                                \
    X(gateThreshold, "gateThreshold", "Threshold", -80, 0, -48, 1, "Gate", "")                               \
    X(gateAttack, "gateAttack", "Gate attack", 0.1, 50, 1, 0.4, "Gate", "")                                  \
    X(gateHold, "gateHold", "Gate hold", 0, 300, 45, 1, "Gate", "")                                          \
    X(gateRelease, "gateRelease", "Gate release", 5, 600, 95, 0.4, "Gate", "")                               \
    X(gateRange, "gateRange", "Gate range", 0, 100, 90, 1, "Gate", "")                                       \
    X(gateHysteresis, "gateHysteresis", "Hysteresis", 0, 18, 5, 1, "Gate", "")                               \
    X(modMix, "modMix", "Voice mix", 0, 1, 0, 1, "Blend", "")                                                \
    X(carMix, "carMix", "Carrier mix", 0, 1, 0, 1, "Blend", "")                                              \
    X(synthMode, "synthMode", "Play mode", 0, 1, 0, 1, "Synth", "Drone|MIDI")                                \
    X(wave, "wave", "Waveform", 0, 6, 5, 1, "Synth", "Saw|Square|Triangle|Sine|Pulse|Supersaw|Noise")        \
    X(oscMix, "oscMix", "Oscillator mix", 0, 1, 0.12, 1, "Synth", "")                                        \
    X(detune, "detune", "Detune", 0, 50, 9, 1, "Synth", "")                                                  \
    X(unison, "unison", "Unison", 1, 7, 3, 1, "Synth", "1|2|3|4|5|6|7")                                      \
    X(octave, "octave", "Octave", -2, 2, 0, 1, "Synth", "-2|-1|0|+1|+2")                                     \
    X(rootNote, "rootNote", "Drone note", 24, 84, 48, 1, "Synth", "")                                        \
    X(chord, "chord", "Drone chord", 0, 4, 0, 1, "Synth", "Single|Fifth|Minor|Major|Octaves")                \
    X(glide, "glide", "Glide", 0, 1000, 0, 0.4, "Synth", "")                                                 \
    X(synthFilter, "synthFilter", "Filter", 100, 20000, 14000, 0.35, "Synth", "")                            \
    X(resonance, "resonance", "Resonance", 0, 1, 0.1, 1, "Synth", "")                                        \
    X(synthAttack, "synthAttack", "Synth attack", 1, 2000, 8, 0.3, "Synth", "")                              \
    X(synthDecay, "synthDecay", "Synth decay", 1, 2000, 200, 0.4, "Synth", "")                               \
    X(synthSustain, "synthSustain", "Synth sustain", 0, 1, 0.8, 1, "Synth", "")                              \
    X(synthRelease, "synthRelease", "Synth release", 2, 3000, 180, 0.4, "Synth", "")                         \
    X(unisonWidth, "unisonWidth", "Unison width", 0, 1, 0.6, 1, "Stereo", "")                                \
    X(spread, "spread", "Band spread", 0, 1, 0.2, 1, "Stereo", "")                                           \
    X(stereoMod, "stereoMod", "Stereo movement", 0, 1, 0, 1, "Stereo", "")                                   \
    X(distortion, "distortion", "Distortion", 0, 1, 0, 1, "Character", "")                                   \
    X(crusher, "crusher", "Crusher mix", 0, 1, 0, 1, "Character", "")                                        \
    X(bitDepth, "bitDepth", "Bit depth", 3, 16, 12, 1, "Character", "")                                      \
    X(reduction, "reduction", "Sample reduction", 1, 32, 1, 0.5, "Character", "")                            \
    X(speaker, "speaker", "Speaker", 0, 1, 0, 1, "Character", "")                                            \
    X(vintage, "vintage", "Vintage", 0, 1, 0, 1, "Character", "")                                            \
    X(modern, "modern", "Modern", 0, 1, 0.3, 1, "Character", "")                                             \
    X(warmth, "warmth", "Warmth", 0, 1, 0.15, 1, "Character", "")                                            \
    X(exciter, "exciter", "Exciter", 0, 1, 0, 1, "Character", "")                                            \
    X(noise, "noise", "Texture noise", 0, 1, 0, 1, "Character", "")                                          \
    X(low, "low", "Low", -12, 12, 0, 1, "Tone", "")                                                          \
    X(body, "body", "Body", -12, 12, 0, 1, "Tone", "")                                                       \
    X(mid, "mid", "Mid", -12, 12, 0, 1, "Tone", "")                                                          \
    X(presence, "presence", "Presence", -12, 12, 1, 1, "Tone", "")                                           \
    X(lfoRate, "lfoRate", "LFO rate", 0.03, 15, 0.5, 0.4, "Motion", "")                                      \
    X(tempoSync, "tempoSync", "Tempo sync", 0, 1, 0, 1, "Motion", "Free|Sync")                               \
    X(division, "division", "Division", 0, 4, 2, 1, "Motion", "1/1|1/2|1/4|1/8|1/16")                        \
    X(rhythm, "rhythm", "Rhythm", 0, 2, 0, 1, "Motion", "Straight|Dotted|Triplet")                           \
    X(lfoShape, "lfoShape", "LFO shape", 0, 2, 0, 1, "Motion", "Sine|Triangle|Smooth steps")                 \
    X(follower, "follower", "Envelope motion", -1, 1, 0, 1, "Motion", "")                                    \
    X(autoPan, "autoPan", "Auto pan", 0, 1, 0, 1, "Motion", "")                                              \
    X(bandMotion, "bandMotion", "Band movement", 0, 1, 0.3, 1, "Motion", "")                                 \
    X(filterMotion, "filterMotion", "Filter movement", 0, 1, 0.3, 1, "Motion", "")                           \
    X(formantMotion, "formantMotion", "Formant movement", 0, 1, 0.2, 1, "Motion", "")                        \
    X(widthMotion, "widthMotion", "Width movement", 0, 1, 0.2, 1, "Motion", "")                              \
    X(presetLevel, "presetLevel", "Preset level", -18, 24, 0, 1, "Blend", "")

enum class P : std::size_t
{
#define RV_ENUM(s, ...) s,
    RV_PARAMETERS(RV_ENUM)
#undef RV_ENUM
    count
};
inline constexpr std::size_t parameterCount = static_cast<std::size_t>(P::count);
struct ParameterDef
{
    const char *id;
    const char *name;
    float min, max, initial, skew;
    const char *group;
    const char *choices;
};
inline constexpr std::array<ParameterDef, parameterCount> definitions{{
#define RV_DEF(s, id, name, lo, hi, d, sk, g, c) {id, name, lo, hi, d, sk, g, c},
    RV_PARAMETERS(RV_DEF)
#undef RV_DEF
}};
constexpr std::size_t index(P p)
{
    return static_cast<std::size_t>(p);
}
struct Params
{
    std::array<float, parameterCount> values{};
    Params()
    {
        for (std::size_t i = 0; i < parameterCount; ++i)
            values[i] = definitions[i].initial;
    }
    float &operator[](P p)
    {
        return values[index(p)];
    }
    float operator[](P p) const
    {
        return values[index(p)];
    }
    void sanitize()
    {
        for (std::size_t i = 0; i < parameterCount; ++i)
        {
            values[i] = std::isfinite(values[i])
                            ? std::clamp(values[i], definitions[i].min, definitions[i].max)
                            : definitions[i].initial;
            if (definitions[i].choices[0])
                values[i] = std::round(values[i]);
        }
    }
};
inline constexpr std::array<int, 7> bandCounts{8, 12, 16, 24, 32, 48, 64};
} // namespace rv
