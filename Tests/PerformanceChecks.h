#pragma once
inline void performanceChecks()
{
    const std::uint8_t on[]{0x90, 60, 100}, off[]{0x80, 60, 0};
    for (double sr : {44100., 48000., 88200., 96000.})
        for (int mode = 0; mode < 3; ++mode)
        {
            Params p;
            p[P::synthMode] = 1;
            p[P::voiceMode] = mode == 1 ? 1.f : 0.f;
            p[P::route] = mode == 2 ? 1.f : 3.f;
            p[P::wetOnly] = 0;
            p[P::mix] = .35f;
            p[P::modMix] = .8f;
            p[P::carMix] = .3f;
            p[P::noise] = .8f;
            p[P::sibilance] = p[P::unvoiced] = p[P::breath] = 1;
            VocoderEngine engine;
            engine.prepare(sr, p);
            int sample = 0;
            auto tick = [&]
            {
                const float x = voice(sample++, sr);
                return engine.process({x, x}, {x, x});
            };
            for (int i = 0; i < 4096; ++i)
            {
                const auto y = tick();
                check(y.l == 0 && y.r == 0,
                      "MIDI gate blocks dry voice, consonants, external audio and FX without notes");
            }
            engine.midi(on, 3);
            double power = 0;
            for (int i = 0; i < 8192; ++i)
            {
                const auto y = tick();
                power += y.l * y.l + y.r * y.r;
            }
            check(power > .1, "MIDI note opens useful output in every sound mode");
            engine.midi(off, 3);
            for (int i = 0; i <= static_cast<int>(std::ceil(sr * .07)); ++i)
                tick();
            for (int i = 0; i < 2048; ++i)
            {
                const auto y = tick();
                check(y.l == 0 && y.r == 0,
                      "Note release closes all output exactly at the configured release time");
            }
        }
    MidiGate gate;
    gate.prepare(48000);
    gate.reset(true);
    auto send = [&](int status, int a, int b)
    {
        const std::uint8_t message[]{static_cast<std::uint8_t>(status), static_cast<std::uint8_t>(a),
                                     static_cast<std::uint8_t>(b)};
        gate.midi(message, 3);
    };
    send(0x90, 60, 100);
    send(0x9f, 60, 100);
    send(0x80, 60, 0);
    check(gate.isOpen(), "Another MIDI channel keeps the same note open");
    send(0x9f, 60, 0);
    check(!gate.isOpen(), "Velocity zero releases the remaining channel");
    send(0x92, 64, 100);
    send(0xb2, 64, 127);
    send(0x82, 64, 0);
    check(gate.isOpen(), "Sustain holds the entire output gate");
    send(0xb1, 64, 0);
    check(gate.isOpen(), "Sustain-off on another channel cannot release this note");
    send(0xb2, 121, 0);
    check(!gate.isOpen(), "Reset controllers releases the sustained note");
    for (int n = 30; n < 100; ++n)
        send(0x90, n, 100);
    for (int n = 30; n < 99; ++n)
        send(0x80, n, 0);
    check(gate.isOpen(), "Gate note tracking is independent of synth voice stealing");
    send(0xb0, 123, 0);
    check(!gate.isOpen(), "All notes off clears all held gate notes");
    send(0x90, 60, 100);
    send(0xb0, 64, 127);
    send(0x80, 60, 0);
    send(0xb0, 120, 0);
    check(!gate.isOpen(), "Panic clears the sustained gate");
    send(0x90, 61, 100);
    const std::uint8_t reset[]{0xff};
    gate.midi(reset, 1);
    check(!gate.isOpen(), "MIDI reset clears all channels");
    for (int i = 0; i < 160; ++i)
        gate.process(false);
    check(gate.value() == 1, "Continuous mode opens without notes");
    gate.reset(true);
    send(0x90, 60, 100);
    float previous = 0;
    for (int i = 0; i < 200; ++i)
    {
        const float next = gate.process(true);
        check(next >= previous && next - previous <= .007f, "Opening uses a short continuous fade");
        previous = next;
    }
    send(0x80, 60, 0);
    for (int i = 0; i < 4000; ++i)
    {
        const float next = gate.process(true);
        check(next <= previous && previous - next <= .00031f, "Closing has no sudden amplitude step");
        previous = next;
    }
    check(gate.value() == 0, "Release ends in exact silence");
    for (int mode = 0; mode < 3; ++mode)
    {
        Params a;
        a[P::voiceMode] = mode == 1 ? 1.f : 0.f;
        a[P::route] = mode == 2 ? 1.f : 0.f;
        a[P::mix] = 0;
        a[P::modMix] = 1;
        Params b = a;
        b[P::mix] = 1;
        b[P::modMix] = 0;
        VocoderEngine first, second;
        first.prepare(48000, a);
        second.prepare(48000, b);
        for (int i = 0; i < 8000; ++i)
        {
            const float x = voice(i, 48000);
            const auto one = first.process({x, x}, {x, x}), two = second.process({x, x}, {x, x});
            check(std::abs(one.l - two.l) < 1e-7f && std::abs(one.r - two.r) < 1e-7f,
                  "Wet only ignores both direct voice mix paths in all sound modes");
        }
    }
}
