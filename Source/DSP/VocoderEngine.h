#pragma once
#include "CharacterFX.h"
#include "FilterBank.h"
#include "Gate.h"
#include "SynthCarrier.h"
#include <atomic>
namespace rv::dsp
{
struct Meters
{
    std::atomic<float> input{0}, modulator{0}, carrier{0}, output{0}, gate{0}, correlation{1}, unvoiced{0};
    std::array<std::atomic<float>, 64> bands{};
    std::atomic<int> bandCount{24};
};
class VocoderEngine
{
  public:
    static constexpr int latency = CharacterFX::latency;
    void prepare(double sampleRate, const Params &initial = Params{})
    {
        sr = std::clamp(sampleRate, 8000.0, 384000.0);
        target = initial;
        target.sanitize();
        current = target;
        smoothing = coefficient(12, sr);
        gate.prepare(sr);
        synth.prepare(sr);
        fx.prepare(sr);
        dcCoefficient = std::exp(-2 * pi * 15 / static_cast<float>(sr));
        modEnvelope.set(sr, 2, 90);
        highEnvelope.set(sr, 2, 45);
        zcEnvelope.set(sr, 4, 30);
        voiceHP.set(sr, 3000);
        breathHP.set(sr, 7000);
        reset();
        topology = target;
        for (auto &b : banks)
        {
            b.configure(sr, bandCounts[static_cast<std::size_t>(static_cast<int>(target[P::bands]))],
                        target[P::freqMin], target[P::freqMax], target[P::formantQ], target[P::envAttack],
                        target[P::envRelease]);
            b.reset();
        }
        control();
    }
    void reset()
    {
        for (auto &b : banks)
            b.reset();
        gate.reset();
        synth.reset();
        fx.reset();
        modEnvelope.reset();
        highEnvelope.reset();
        zcEnvelope.reset();
        voiceHP.reset();
        breathHP.reset();
        dryL.reset();
        dryR.reset();
        voiceDryL.reset();
        voiceDryR.reset();
        active = 0;
        fade = 0;
        transition = false;
        clock = 0;
        phase = 0;
        prevMod = 0;
        cross = energyL = energyR = 0;
        dcIn = dcOut = {};
        bypassBlend = target[P::bypass];
        voiceBlend = target[P::voiceMode];
        routeWeights.fill(0);
        routeWeights[static_cast<std::size_t>(std::clamp(static_cast<int>(target[P::route]), 0, 3))] = 1;
    }
    void beginBlock(const Params &p, double bpm = 120, double ppq = -1, bool playing = false)
    {
        target = p;
        target.sanitize();
        tempo = (std::isfinite(bpm) && bpm > 0) ? std::clamp(bpm, 10.0, 1000.0) : 120;
        if (playing && target[P::tempoSync] > 0.5f && std::isfinite(ppq) && ppq >= 0)
        {
            const double period = beatPeriod();
            const double hostPhase = ppq / period;
            phase = hostPhase - std::floor(hostPhase);
        }
    }
    void midi(const std::uint8_t *data, int length)
    {
        synth.midi(data, length);
        synth.configure(current, filterMod);
    }
    Stereo process(Stereo main, Stereo sidechain, bool sidechainAvailable = true)
    {
        for (std::size_t i = 0; i < parameterCount; ++i)
            current.values[i] = definitions[i].choices[0] != '\0'
                                    ? target.values[i]
                                    : target.values[i] + smoothing * (current.values[i] - target.values[i]);
        if ((clock++ & 63) == 0)
            control();
        main = {sane(main.l), sane(main.r)};
        sidechain = sidechainAvailable ? Stereo{sane(sidechain.l), sane(sidechain.r)} : Stereo{};
        const Stereo dry{dryL.process(main.l), dryR.process(main.r)};
        main = main * inGain;
        sidechain = sidechain * inGain;
        const auto synthOut = synth.process();
        float mod = 0;
        Stereo carrier, voice;
        const int route = static_cast<int>(target[P::route]);
        for (int i = 0; i < 4; ++i)
        {
            auto &w = routeWeights[static_cast<std::size_t>(i)];
            w = (i == route ? 1.0f : 0.0f) + smoothing * (w - (i == route ? 1.0f : 0.0f));
            const auto m = i < 2 ? main : sidechain;
            const auto c = (i == 0 || i == 3) ? synthOut : (i == 1 ? sidechain : main);
            mod += 0.5f * (m.l + m.r) * w;
            voice = voice + m * w;
            carrier = carrier + c * w;
        }
        const float gateGain = gate.process(mod), env = modEnvelope.process(mod);
        const Stereo voiceDry{voiceDryL.process(inGain > 0 ? voice.l / inGain : 0),
                              voiceDryR.process(inGain > 0 ? voice.r / inGain : 0)};
        voiceBlend = target[P::voiceMode] + smoothing * (voiceBlend - target[P::voiceMode]);
        if (voiceBlend < .00001f)
            voiceBlend = 0;
        if (voiceBlend > .99999f)
            voiceBlend = 1;
        const float hp = voiceHP.high(mod), breathy = breathHP.high(mod), high = highEnvelope.process(hp);
        const float zeroCross = zcEnvelope.process(mod * prevMod < 0 ? 1.0f : 0.0f);
        prevMod = mod;
        unvoicedAmount = std::clamp((high / (env + 0.00001f) - 0.2f) * 1.4f + zeroCross * 1.6f, 0.0f, 1.0f);
        const auto gatedVoice = voice * gateGain;
        Stereo shapedVoice;
        auto wet = banks[static_cast<std::size_t>(active)].process(mod * gateGain, carrier, gatedVoice,
                                                                   voiceBlend > 0 ? &shapedVoice : nullptr);
        if (transition)
        {
            Stereo otherVoice;
            const auto b = banks[static_cast<std::size_t>(1 - active)].process(
                mod * gateGain, carrier, gatedVoice, voiceBlend > 0 ? &otherVoice : nullptr);
            fade = std::min(1.0f, fade + 1.0f / static_cast<float>(sr * 0.035));
            wet = wet * (1 - fade) + b * fade;
            shapedVoice = shapedVoice * (1 - fade) + otherVoice * fade;
            if (fade >= 1)
            {
                active = 1 - active;
                transition = false;
            }
        }
        const float consonant = gateGain * (hp * (current[P::sibilance] * 0.6f + current[P::clarity] * 0.23f +
                                                  current[P::unvoiced] * unvoicedAmount * 0.6f) +
                                            breathy * current[P::breath] * 0.4f);
        const float amount = current[P::amount];
        wet = wet * amount + carrier * ((1 - amount) * env * gateGain * 2);
        wet = wet + Stereo{consonant, consonant};
        wet = wet + Stereo{mod, mod} * (current[P::modMix] * gateGain) +
              carrier * (current[P::carMix] * gateGain);
        const auto processedVoice =
            gatedVoice * (1 - amount) + shapedVoice * amount + Stereo{consonant, consonant} * .35f;
        wet = wet * (1 - voiceBlend) + processedVoice * voiceBlend;
        const float mid = (wet.l + wet.r) * 0.5f, side = (wet.l - wet.r) * 0.5f;
        const float width =
            std::clamp(current[P::width] + motionValue * current[P::widthMotion] * 0.5f, 0.0f, 2.0f);
        wet = {mid + side * width, mid - side * width};
        const float pan =
            std::clamp(lfoValue * (current[P::autoPan] + current[P::stereoMod] * 0.4f) * current[P::motion],
                       -0.8f, 0.8f);
        wet.l *= std::sqrt(1 - pan);
        wet.r *= std::sqrt(1 + pan);
        wet = fx.process(wet, env * gateGain);
        const Stereo dcRemoved{wet.l - dcIn.l + dcCoefficient * dcOut.l,
                               wet.r - dcIn.r + dcCoefficient * dcOut.r};
        dcIn = wet;
        dcOut = dcRemoved;
        // Factory trim was calibrated for vocoding. It must not over-amplify a
        // full-level original voice, nor make a voice-only preset depend on it.
        wet = dcRemoved * lerp(presetGain, 1.f, voiceBlend);
        const float mix = current[P::mix];
        Stereo out = voiceDry * std::cos(mix * pi * 0.5f) + wet * std::sin(mix * pi * 0.5f);
        out = out * outGain;
        // Unity below -1.4 dBFS, monotonic soft knee above it, finite bounded output.
        auto protect = [](float x)
        {
            x = sane(x);
            const float a = std::abs(x);
            return a <= 0.85f ? x : std::copysign(0.85f + 0.15f * std::tanh((a - 0.85f) / 0.15f), x);
        };
        out = {protect(out.l), protect(out.r)};
        const float bypassTarget = target[P::bypass];
        bypassBlend = bypassTarget + smoothing * (bypassBlend - bypassTarget);
        out = out * (1 - bypassBlend) + dry * bypassBlend;
        phase += lfoHz / sr;
        if (phase >= 1)
            phase -= std::floor(phase);
        inputPeak = std::max(inputPeak, std::max(std::abs(main.l), std::abs(main.r)));
        modPeak = std::max(modPeak, std::abs(mod));
        const auto sound = carrier * (1 - voiceBlend) + voice * voiceBlend;
        carPeak = std::max(carPeak, std::max(std::abs(sound.l), std::abs(sound.r)));
        outPeak = std::max(outPeak, std::max(std::abs(out.l), std::abs(out.r)));
        cross = 0.999f * cross + 0.001f * out.l * out.r;
        energyL = 0.999f * energyL + 0.001f * out.l * out.l;
        energyR = 0.999f * energyR + 0.001f * out.r * out.r;
        return out;
    }
    void publish(Meters &m)
    {
        m.input.store(inputPeak, std::memory_order_relaxed);
        m.modulator.store(modPeak, std::memory_order_relaxed);
        m.carrier.store(carPeak, std::memory_order_relaxed);
        m.output.store(outPeak, std::memory_order_relaxed);
        m.gate.store(gate.getGain(), std::memory_order_relaxed);
        m.unvoiced.store(unvoicedAmount, std::memory_order_relaxed);
        m.correlation.store(
            energyL * energyR > 1e-15f ? std::clamp(cross / std::sqrt(energyL * energyR), -1.0f, 1.0f) : 1,
            std::memory_order_relaxed);
        const auto &b = banks[static_cast<std::size_t>(active)];
        m.bandCount.store(b.count(), std::memory_order_relaxed);
        for (int i = 0; i < 64; ++i)
            m.bands[static_cast<std::size_t>(i)].store(b.activity()[static_cast<std::size_t>(i)],
                                                       std::memory_order_relaxed);
        inputPeak = modPeak = carPeak = outPeak = 0;
    }
    const FilterBank &bank() const
    {
        return banks[static_cast<std::size_t>(active)];
    }
    float getGateGain() const
    {
        return gate.getGain();
    }
    int activeVoices() const
    {
        return synth.activeVoices();
    }

  private:
    double beatPeriod() const
    {
        double beats = 4.0 / std::pow(2.0, std::round(target[P::division]));
        if (target[P::rhythm] > 1.5f)
            beats *= 2.0 / 3.0;
        else if (target[P::rhythm] > 0.5f)
            beats *= 1.5;
        return beats;
    }
    void control()
    {
        lfoHz = current[P::tempoSync] > 0.5f ? tempo / (60 * beatPeriod()) : current[P::lfoRate];
        const float ph = static_cast<float>(phase);
        lfoValue = current[P::lfoShape] < 0.5f
                       ? std::sin(2 * pi * ph)
                       : (current[P::lfoShape] < 1.5f ? 1 - 4 * std::abs(ph - 0.5f)
                                                      : std::tanh(3 * std::sin(2 * pi * ph)));
        motionValue = current[P::motion] *
                      (lfoValue + current[P::follower] * std::clamp(modEnvelope.value * 4, 0.0f, 1.0f));
        filterMod = motionValue * current[P::filterMotion];
        inGain = dbGain(current[P::inputGain]);
        outGain = dbGain(current[P::outputGain]);
        presetGain = dbGain(current[P::presetLevel]);
        gate.configure(current);
        synth.configure(current, filterMod);
        fx.configure(current);
        const int n = bandCounts[static_cast<std::size_t>(static_cast<int>(target[P::bands]))];
        if (!transition && (n != banks[static_cast<std::size_t>(active)].count() ||
                            std::abs(current[P::freqMin] - topology[P::freqMin]) > 2 ||
                            std::abs(current[P::freqMax] - topology[P::freqMax]) > 80 ||
                            std::abs(current[P::formantQ] - topology[P::formantQ]) > 0.03f))
        {
            auto &b = banks[static_cast<std::size_t>(1 - active)];
            b.configure(sr, n, current[P::freqMin], current[P::freqMax], current[P::formantQ],
                        current[P::envAttack], current[P::envRelease]);
            b.reset();
            topology = current;
            transition = true;
            fade = 0;
        }
        const float shift = current[P::formant] - current[P::size] * 5 + current[P::identity] * 3 +
                            motionValue * current[P::formantMotion] * 5;
        for (auto &b : banks)
        {
            b.setEnvelopes(sr, current[P::envAttack], current[P::envRelease]);
            b.shape(shift, current[P::bandShift] + motionValue * current[P::bandMotion] * 3,
                    current[P::tilt] + current[P::modern] * 2, current[P::spread],
                    current[P::definition] + current[P::clarity] * 0.3f, current[P::nasal],
                    current[P::throat]);
        }
    }
    double sr = 48000, tempo = 120, phase = 0, lfoHz = 0.5;
    Params target, current, topology;
    std::array<FilterBank, 2> banks;
    Gate gate;
    SynthCarrier synth;
    CharacterFX fx;
    Envelope modEnvelope, highEnvelope, zcEnvelope;
    SVF voiceHP, breathHP;
    Delay<latency> dryL, dryR;
    Delay<latency> voiceDryL, voiceDryR;
    std::array<float, 4> routeWeights{};
    Stereo dcIn, dcOut;
    float dcCoefficient = 0, presetGain = 1;
    int active = 0;
    std::uint64_t clock = 0;
    bool transition = false;
    float fade = 0, smoothing = 0, inGain = 1, outGain = 1, motionValue = 0, filterMod = 0, lfoValue = 0,
          prevMod = 0, unvoicedAmount = 0, bypassBlend = 0, voiceBlend = 0;
    float inputPeak = 0, modPeak = 0, carPeak = 0, outPeak = 0, cross = 0, energyL = 0, energyR = 0;
};
} // namespace rv::dsp
