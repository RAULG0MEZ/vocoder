#pragma once
#include "SkinData.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace hardware
{
inline juce::Image artwork(const char *name)
{
    int size = 0;
    if (const auto *data = SkinData::getNamedResource(name, size))
        return juce::ImageCache::getFromMemory(data, size);
    return {};
}
inline void texture(juce::Graphics &g, const char *name, juce::Rectangle<float> bounds, float opacity)
{
    if (const auto image = artwork(name); image.isValid())
    {
        juce::Graphics::ScopedSaveState scope(g);
        juce::Path clip;
        clip.addRoundedRectangle(bounds.reduced(.5f), 4.f);
        g.reduceClipRegion(clip);
        g.setOpacity(opacity);
        g.drawImage(image, bounds, juce::RectanglePlacement::stretchToFit);
    }
}
class Chassis final : public juce::Component
{
  public:
    Chassis()
    {
        setInterceptsMouseClicks(false, false);
        setComponentID("hardware-chassis");
    }
    void paint(juce::Graphics &g) override
    {
        g.setGradientFill({juce::Colour(0xff282a2a), 0, 0, juce::Colour(0xff101214), 0,
                           static_cast<float>(getHeight()), false});
        g.fillAll();
        texture(g, "chassis_png", getLocalBounds().toFloat(), .58f);
        g.setColour(juce::Colours::white.withAlpha(.013f));
        for (int y = 2; y < getHeight(); y += 3)
            g.drawHorizontalLine(y, 1.f, static_cast<float>(getWidth() - 1));
        g.setColour(juce::Colour(0xff545653));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.8f), 8.f, 1.f);
        g.setColour(juce::Colours::black.withAlpha(.7f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.f), 7.f, 1.f);
    }
};
} // namespace hardware
