#pragma once
#include "../Parameters.h"
#include "Primitives.h"
namespace rv::dsp
{
class Gate
{
  public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        detector.set(sr, 0.3f, 20);
        reset();
    }
    void configure(const Params &p)
    {
        on = p[P::gateOn] > 0.5f;
        threshold = dbGain(p[P::gateThreshold]);
        closeThreshold = dbGain(p[P::gateThreshold] - p[P::gateHysteresis]);
        attack = coefficient(p[P::gateAttack], sr);
        release = coefficient(p[P::gateRelease], sr);
        floor = p[P::gateRange] >= 90 ? 0 : dbGain(-p[P::gateRange]);
        holdSamples = static_cast<int>(p[P::gateHold] * 0.001 * sr);
    }
    float process(float x)
    {
        const float level = detector.process(x);
        if (level >= threshold)
        {
            open = true;
            remaining = holdSamples;
        }
        else if (level < closeThreshold)
        {
            if (remaining > 0)
                --remaining;
            else
                open = false;
        }
        const float target = (!on || open) ? 1.0f : floor;
        gain = target + (target > gain ? attack : release) * (gain - target);
        if (gain < 1e-20f)
            gain = 0;
        return gain;
    }
    void reset()
    {
        detector.reset();
        gain = 0;
        open = false;
        remaining = 0;
    }
    float getGain() const
    {
        return gain;
    }
    float getLevel() const
    {
        return detector.value;
    }

  private:
    double sr = 48000;
    Envelope detector;
    float threshold = 0.004f, closeThreshold = 0.002f, attack = 0, release = 0, floor = 0, gain = 0;
    int holdSamples = 0, remaining = 0;
    bool on = true, open = false;
};
} // namespace rv::dsp
