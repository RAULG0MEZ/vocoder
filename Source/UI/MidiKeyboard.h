#pragma once
#include "Plugin/MidiMonitor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class RVKeyboard final : public juce::MidiKeyboardComponent
{
  public:
    std::function<void()> onUserNote;
    bool mouseDownOnKey(int, const juce::MouseEvent &) override
    {
        if (onUserNote)
            onUserNote();
        return true;
    }
    RVKeyboard(juce::MidiKeyboardState &mouseKeys, const rv::MidiMonitor &incoming)
        : MidiKeyboardComponent(mouseKeys, horizontalKeyboard), monitor(incoming)
    {
    }
    void drawWhiteNote(int note, juce::Graphics &g, juce::Rectangle<float> area, bool down, bool over,
                       juce::Colour line, juce::Colour text) override
    {
        const bool held = down || monitor.isDown(note);
        g.setGradientFill({juce::Colour(held ? 0xffe3f6aa : 0xffc0bdae), area.getX(), area.getY(),
                           juce::Colour(held ? 0xffa6c560 : 0xffeee9d9), area.getX(), area.getBottom(),
                           false});
        g.fillRect(area);
        g.setColour(juce::Colour(0xff252a29));
        g.drawVerticalLine(static_cast<int>(area.getRight() - 1), area.getY(), area.getBottom());
        g.setColour(juce::Colours::black.withAlpha(.18f));
        g.fillRect(area.withTop(area.getBottom() - (held ? 2 : 4)));
        if (over && !held)
        {
            g.setColour(juce::Colours::white.withAlpha(.16f));
            g.fillRect(area);
        }
        if (note % 12 == 0)
        {
            g.setColour(juce::Colour(0xff373d32));
            g.setFont(juce::FontOptions(9));
            g.drawText(juce::MidiMessage::getMidiNoteName(note, true, true, 4),
                       area.reduced(1).withTop(area.getBottom() - 16), juce::Justification::centred);
        }
        juce::ignoreUnused(line, text);
    }
    void drawBlackNote(int note, juce::Graphics &g, juce::Rectangle<float> area, bool down, bool over,
                       juce::Colour fill) override
    {
        const bool held = down || monitor.isDown(note);
        g.setColour(juce::Colour(0xff080b0b));
        g.fillRect(area);
        const auto face = area.reduced(1.2f).withTrimmedBottom(held ? 2 : 5);
        g.setGradientFill({juce::Colour(held   ? 0xffabbf7c
                                        : over ? 0xff555c58
                                               : 0xff444b48),
                           face.getX(), face.getY(), juce::Colour(held ? 0xff6c8740 : 0xff1c2221),
                           face.getX(), face.getBottom(), false});
        g.fillRoundedRectangle(face, 1.5f);
        g.setColour(juce::Colours::white.withAlpha(.18f));
        g.drawHorizontalLine(static_cast<int>(face.getBottom() - 1), face.getX() + 1, face.getRight() - 1);
        juce::ignoreUnused(fill);
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
