#pragma once
#include "Plugin/PluginProcessor.h"
#include <new>
#include <thread>
extern int checks;
void check(bool, const char *);
inline thread_local bool watchAllocations = false;
inline thread_local std::size_t allocationCount = 0;
// Intercepts C++ heap allocations on the test thread, including linked JUCE calls.
void *operator new(std::size_t bytes)
{
    if (watchAllocations)
        ++allocationCount;
    if (void *p = std::malloc(std::max<std::size_t>(1, bytes)))
        return p;
    throw std::bad_alloc();
}
void operator delete(void *p) noexcept
{
    std::free(p);
}
void *operator new[](std::size_t bytes)
{
    return ::operator new(bytes);
}
void operator delete[](void *p) noexcept
{
    std::free(p);
}
inline void processorChecks()
{
    using namespace rv;
    using namespace rv::dsp;
    auto set = [](RVocoderProcessor &p, P id, float value)
    {
        auto *param = p.apvts.getParameter(definitions[index(id)].id);
        param->setValueNotifyingHost(param->convertTo0to1(value));
    };
    auto setup = [](RVocoderProcessor &p)
    {
        auto layout = p.getBusesLayout();
        layout.inputBuses.set(0, juce::AudioChannelSet::stereo());
        layout.inputBuses.set(1, juce::AudioChannelSet::mono());
        layout.outputBuses.set(0, juce::AudioChannelSet::stereo());
        check(p.setBusesLayout(layout), "Sidechain test layout");
        p.setRateAndBufferSizeDetails(48000, 256);
    };
    RVocoderProcessor a, b;
    setup(a);
    setup(b);
    set(a, P::route, 1);
    set(b, P::route, 2);
    a.prepareToPlay(48000, 256);
    b.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> ba(3, 256), bb(3, 256);
    juce::MidiBuffer ma, mb;
    double difference = 0, energy = 0;
    for (int block = 0; block < 100; ++block)
    {
        for (int i = 0; i < 256; ++i)
        {
            const float t = static_cast<float>(block * 256 + i) / 48000;
            const float mod = .2f * std::sin(2 * pi * 630 * t),
                        carrier = .3f * (2 * std::fmod(t * 110, 1.0f) - 1);
            ba.setSample(0, i, mod);
            ba.setSample(1, i, mod);
            ba.setSample(2, i, carrier);
            bb.setSample(0, i, carrier);
            bb.setSample(1, i, carrier);
            bb.setSample(2, i, mod);
        }
        watchAllocations = true;
        a.processBlock(ba, ma);
        b.processBlock(bb, mb);
        watchAllocations = false;
        for (int i = 0; i < 256; ++i)
        {
            difference += std::abs(ba.getSample(0, i) - bb.getSample(0, i));
            energy += ba.getSample(0, i) * ba.getSample(0, i);
            const float t = static_cast<float>(block * 256 + i) / 48000;
            check(std::abs(ba.getSample(2, i) - .3f * (2 * std::fmod(t * 110, 1.0f) - 1)) < 1e-7f,
                  "Sidechain memory remains unchanged");
        }
    }
    check(difference < .01 && energy > .01, "Main and sidechain source swap produces equivalent audio");
    check(allocationCount == 0, "No C++ heap allocations in audio callbacks");
    // Full state render must be repeatable after restoring into another instance.
    a.setCurrentProgram(17);
    juce::MemoryBlock saved;
    a.getStateInformation(saved);
    b.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    a.prepareToPlay(48000, 256);
    b.prepareToPlay(48000, 256);
    for (int block = 0; block < 20; ++block)
    {
        for (int i = 0; i < 256; ++i)
        {
            float x = .2f * std::sin(static_cast<float>(block * 256 + i) * .05f);
            for (int c = 0; c < 3; ++c)
            {
                ba.setSample(c, i, x);
                bb.setSample(c, i, x);
            }
        }
        a.processBlock(ba, ma);
        b.processBlock(bb, mb);
        for (int i = 0; i < 256; ++i)
            check(std::abs(ba.getSample(0, i) - bb.getSample(0, i)) < 1e-6,
                  "Restored state renders the same sound");
    }
    // Block partitioning must not change the timeline of envelopes or modulation.
    VocoderEngine whole, split;
    Params params;
    params[P::motion] = .5f;
    whole.prepare(48000, params);
    split.prepare(48000, params);
    for (int i = 0; i < 20000; ++i)
    {
        float x = .2f * std::sin(static_cast<float>(i) * .04f);
        if (i % 37 == 0)
            split.beginBlock(params);
        if (i % 1024 == 0)
            whole.beginBlock(params);
        auto one = whole.process({x, x}, {0, 0});
        auto two = split.process({x, x}, {0, 0});
        check(std::abs(one.l - two.l) < 1e-6, "Variable host buffer sizes preserve DSP output");
    }
    // Concurrent state installation and rendering must not block the audio callback.
    std::atomic<bool> done{false};
    std::thread writer(
        [&]
        {
            for (int i = 0; i < 100; ++i)
                b.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
            done.store(true);
        });
    while (!done.load())
    {
        ba.clear();
        watchAllocations = true;
        b.processBlock(ba, ma);
        watchAllocations = false;
        for (int i = 0; i < 256; ++i)
            check(std::isfinite(ba.getSample(0, i)), "Concurrent state restore stays finite");
    }
    writer.join();
    check(allocationCount == 0, "State changes introduce no audio-thread allocations");
}
