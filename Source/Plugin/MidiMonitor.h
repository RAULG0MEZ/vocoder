#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace rv
{
// Written by the audio callback, read by the editor. No keyboard listeners,
// locks, event queues or feedback into the synthesiser on the display path.
class MidiMonitor
{
  public:
    void reset() noexcept
    {
        for (auto &note : held)
            note.store(0, std::memory_order_relaxed);
    }
    void observe(const std::uint8_t *data, int length) noexcept
    {
        if (length < 3)
            return;
        const auto type = data[0] & 0xf0;
        const auto channel = static_cast<std::uint16_t>(1u << (data[0] & 15));
        if ((type == 0x90 || type == 0x80) && data[1] < 128)
        {
            auto &note = held[data[1]];
            const auto old = note.load(std::memory_order_relaxed);
            const bool on = type == 0x90 && data[2] != 0;
            note.store(static_cast<std::uint16_t>(on ? old | channel : old & ~channel),
                       std::memory_order_relaxed);
            if (on)
            {
                lastNote.store(data[1], std::memory_order_relaxed);
                noteOns.fetch_add(1, std::memory_order_relaxed);
            }
        }
        else if (type == 0xb0 && (data[1] == 120 || data[1] == 123))
        {
            for (auto &note : held)
                note.store(static_cast<std::uint16_t>(note.load(std::memory_order_relaxed) & ~channel),
                           std::memory_order_relaxed);
        }
    }
    bool isDown(int note) const noexcept
    {
        return note >= 0 && note < 128 &&
               held[static_cast<std::size_t>(note)].load(std::memory_order_relaxed) != 0;
    }
    int latestNote() const noexcept
    {
        return lastNote.load(std::memory_order_relaxed);
    }
    unsigned sequence() const noexcept
    {
        return noteOns.load(std::memory_order_relaxed);
    }

  private:
    static_assert(std::atomic<std::uint16_t>::is_always_lock_free);
    std::array<std::atomic<std::uint16_t>, 128> held{};
    std::atomic<int> lastNote{60};
    std::atomic<unsigned> noteOns{0};
};
} // namespace rv
