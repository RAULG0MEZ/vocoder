#pragma once
#include "HardwareSkin.h"
#include "Plugin/PluginProcessor.h"

class SoundCanvas final : public juce::Component, public juce::SettableTooltipClient
{
  public:
    explicit SoundCanvas(RVocoderProcessor &p) : processor(p)
    {
        setComponentID("sound-canvas");
        setTitle("Forma de la voz");
        setDescription("Control XY de Formant y Character, o bordes del espectro. Flechas para ajustar.");
        setWantsKeyboardFocus(true);
        setMode(false);
    }
    ~SoundCanvas() override
    {
        endGesture();
    }
    void setMode(bool spectrum)
    {
        endGesture();
        spectral = spectrum;
        setTooltip(spectral ? "Arrastra el borde inferior o superior del espectro. Flechas: graves/agudos."
                            : "Arrastra: horizontal = Formant, vertical = Character. Flechas para ajustar; "
                              "doble clic reinicia.");
        repaint();
    }
    void refresh()
    {
        const int count = std::clamp(processor.meters.bandCount.load(), 1, 64);
        for (int i = 0; i < count; ++i)
        {
            const auto v = processor.meters.bands[static_cast<std::size_t>(i)].load();
            levels[static_cast<std::size_t>(i)] =
                std::max(std::clamp(std::sqrt(std::max(0.f, v)) * 6.f, 0.f, 1.f),
                         levels[static_cast<std::size_t>(i)] * .83f);
        }
        repaint();
    }
    void paint(juce::Graphics &g) override
    {
        const juce::Colour lime(0xffd7ed8d), muted(0xff9da7a5), blue(0xff78c9d1);
        const auto p = processor.readParameters();
        const auto area = plot();
        g.fillAll(juce::Colour(0xff0d1214));
        hardware::texture(g, "glass_png", getLocalBounds().toFloat(), .16f);
        g.setColour(juce::Colour(0xff293336));
        for (int i = 0; i <= 4; ++i)
        {
            const auto f = static_cast<float>(i) / 4;
            g.drawVerticalLine(static_cast<int>(area.getX() + area.getWidth() * f), area.getY(),
                               area.getBottom());
            g.drawHorizontalLine(static_cast<int>(area.getY() + area.getHeight() * f), area.getX(),
                                 area.getRight());
        }
        const int count = std::clamp(processor.meters.bandCount.load(), 1, 64);
        juce::Path curve;
        curve.startNewSubPath(area.getX(), area.getBottom());
        for (int i = 0; i < count; ++i)
        {
            const auto fraction = (static_cast<float>(i) + .5f) / count;
            const auto x =
                spectral ? hzX(p[rv::P::freqMin] * std::pow(p[rv::P::freqMax] / p[rv::P::freqMin], fraction))
                         : area.getX() + area.getWidth() * fraction;
            curve.lineTo(x, area.getBottom() - levels[static_cast<std::size_t>(i)] * area.getHeight() * .8f);
        }
        curve.lineTo(area.getRight(), area.getBottom());
        g.setColour(blue.withAlpha(.65f));
        g.strokePath(curve, juce::PathStrokeType(1.8f));
        curve.closeSubPath();
        g.setGradientFill(juce::ColourGradient(blue.withAlpha(.24f), 0, area.getY(), blue.withAlpha(.015f), 0,
                                               area.getBottom(), false));
        g.fillPath(curve);
        g.setFont(juce::FontOptions(12));
        g.setColour(muted);
        if (spectral)
        {
            const float lo = hzX(p[rv::P::freqMin]), hi = hzX(p[rv::P::freqMax]);
            g.setColour(lime.withAlpha(.08f));
            g.fillRect(juce::Rectangle<float>(lo, area.getY(), hi - lo, area.getHeight()));
            for (const auto x : {lo, hi})
            {
                g.setColour(lime);
                g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom());
                g.fillRoundedRectangle(x - 5, area.getCentreY() - 14, 10, 28, 4);
            }
            g.drawText(juce::String(p[rv::P::freqMin], 0) + " Hz", 18, 9, 100, 20, juce::Justification::left);
            g.drawText(juce::String(p[rv::P::freqMax] / 1000, 1) + " kHz", getWidth() - 128, 9, 110, 20,
                       juce::Justification::right);
            g.setColour(muted);
            g.drawText("40 Hz", 18, getHeight() - 27, 80, 20, juce::Justification::left);
            g.drawText("18 kHz", getWidth() - 98, getHeight() - 27, 80, 20, juce::Justification::right);
        }
        else
        {
            const auto point =
                juce::Point<float>(area.getX() + (p[rv::P::formant] + 12) / 24 * area.getWidth(),
                                   area.getBottom() - p[rv::P::character] * area.getHeight());
            g.setColour(lime.withAlpha(.45f));
            g.drawLine(point.x, area.getY(), point.x, area.getBottom());
            g.drawLine(area.getX(), point.y, area.getRight(), point.y);
            g.setColour(lime.withAlpha(.12f));
            g.fillEllipse(point.x - 20, point.y - 20, 40, 40);
            g.setColour(lime);
            g.drawEllipse(point.x - 9, point.y - 9, 18, 18, 2);
            g.fillEllipse(point.x - 3, point.y - 3, 6, 6);
            const int half = getWidth() / 2;
            g.setFont(juce::FontOptions(10));
            g.drawText("FORMANT", 12, 4, half - 16, 15, juce::Justification::left);
            g.drawText("CHARACTER", half, 4, half - 12, 15, juce::Justification::right);
            g.setFont(juce::FontOptions(12));
            g.drawText(juce::String(p[rv::P::formant], 1) + " st", 12, 20, half - 16, 15,
                       juce::Justification::left);
            g.drawText(juce::String(p[rv::P::character] * 100, 0) + "%", half, 20, half - 12, 15,
                       juce::Justification::right);
            g.setColour(muted);
            g.drawText("-12 st", 18, getHeight() - 27, 80, 20, juce::Justification::left);
            g.drawText("+12 st", getWidth() - 98, getHeight() - 27, 80, 20, juce::Justification::right);
        }
        if (hasKeyboardFocus(true))
        {
            g.setColour(lime);
            g.drawRect(getLocalBounds(), 1);
        }
    }
    void mouseDown(const juce::MouseEvent &e) override
    {
        if (!plot().contains(e.position))
            return;
        grabKeyboardFocus();
        endGesture();
        if (spectral)
        {
            const auto p = processor.readParameters();
            ids[0] = std::abs(e.position.x - hzX(p[rv::P::freqMin])) <
                             std::abs(e.position.x - hzX(p[rv::P::freqMax]))
                         ? rv::P::freqMin
                         : rv::P::freqMax;
            gestureCount = 1;
        }
        else
        {
            ids = {rv::P::formant, rv::P::character};
            gestureCount = 2;
        }
        for (int i = 0; i < gestureCount; ++i)
            parameter(ids[static_cast<std::size_t>(i)])->beginChangeGesture();
        mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent &e) override
    {
        if (gestureCount == 0)
            return;
        const auto a = plot();
        const auto x = std::clamp((e.position.x - a.getX()) / a.getWidth(), 0.f, 1.f);
        const auto y = std::clamp((a.getBottom() - e.position.y) / a.getHeight(), 0.f, 1.f);
        if (spectral)
            set(ids[0], 40.f * std::pow(450.f, x));
        else
        {
            set(rv::P::formant, -12.f + x * 24.f);
            set(rv::P::character, y);
        }
        repaint();
    }
    void mouseUp(const juce::MouseEvent &) override
    {
        endGesture();
    }
    void mouseDoubleClick(const juce::MouseEvent &) override
    {
        endGesture();
        for (auto id : spectral ? std::array{rv::P::freqMin, rv::P::freqMax}
                                : std::array{rv::P::formant, rv::P::character})
            completeGesture(id, rv::definitions[rv::index(id)].initial);
    }
    bool keyPressed(const juce::KeyPress &key) override
    {
        const auto code = key.getKeyCode();
        const bool horizontal = code == juce::KeyPress::leftKey || code == juce::KeyPress::rightKey;
        if (!horizontal && code != juce::KeyPress::upKey && code != juce::KeyPress::downKey)
            return false;
        const auto id = spectral ? (horizontal ? rv::P::freqMin : rv::P::freqMax)
                                 : (horizontal ? rv::P::formant : rv::P::character);
        const auto direction =
            code == juce::KeyPress::leftKey || code == juce::KeyPress::downKey ? -1.f : 1.f;
        const auto p = processor.readParameters();
        completeGesture(
            id, p[id] + direction * (spectral ? (horizontal ? 10.f : 100.f) : (horizontal ? .25f : .02f)));
        return true;
    }

  private:
    juce::Rectangle<float> plot() const
    {
        return getLocalBounds().toFloat().reduced(26, 38);
    }
    float hzX(float hz) const
    {
        return plot().getX() + std::log(hz / 40.f) / std::log(450.f) * plot().getWidth();
    }
    juce::RangedAudioParameter *parameter(rv::P id)
    {
        return processor.apvts.getParameter(rv::definitions[rv::index(id)].id);
    }
    void set(rv::P id, float value)
    {
        const auto &d = rv::definitions[rv::index(id)];
        parameter(id)->setValueNotifyingHost(parameter(id)->convertTo0to1(std::clamp(value, d.min, d.max)));
    }
    void completeGesture(rv::P id, float value)
    {
        parameter(id)->beginChangeGesture();
        set(id, value);
        parameter(id)->endChangeGesture();
        repaint();
    }
    void endGesture()
    {
        for (int i = 0; i < gestureCount; ++i)
            parameter(ids[static_cast<std::size_t>(i)])->endChangeGesture();
        gestureCount = 0;
    }
    RVocoderProcessor &processor;
    std::array<float, 64> levels{};
    std::array<rv::P, 2> ids{rv::P::formant, rv::P::character};
    int gestureCount = 0;
    bool spectral = false;
};
