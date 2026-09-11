#pragma once
#include "Plugin/MidiMonitor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class RVKeyboard final : public juce::MidiKeyboardComponent
{
  public:
    RVKeyboard(juce::MidiKeyboardState &mouseKeys, const rv::MidiMonitor &incoming)
        : MidiKeyboardComponent(mouseKeys, horizontalKeyboard), monitor(incoming)
    {
    }
    void drawWhiteNote(int note, juce::Graphics &g, juce::Rectangle<float> area, bool down, bool over,
                       juce::Colour line, juce::Colour text) override
    {
        MidiKeyboardComponent::drawWhiteNote(note, g, area, down || monitor.isDown(note), over, line, text);
    }
    void drawBlackNote(int note, juce::Graphics &g, juce::Rectangle<float> area, bool down, bool over,
                       juce::Colour fill) override
    {
        MidiKeyboardComponent::drawBlackNote(note, g, area, down || monitor.isDown(note), over, fill);
    }
    void refreshIncomingNotes()
    {
        if (const auto sequence = monitor.sequence(); sequence != lastSequence)
        {
            lastSequence = sequence;
            const auto note = monitor.latestNote();
            const auto bounds = getRectangleForKey(note);
            if (bounds.getX() < 16 || bounds.getRight() > getWidth() - 16)
                setLowestVisibleKey(juce::jlimit(0, 115, note - 12));
        }
        repaint();
    }

  private:
    const rv::MidiMonitor &monitor;
    unsigned lastSequence = 0;
};
