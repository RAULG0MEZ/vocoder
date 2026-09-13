#include "PluginEditor.h"
using namespace rv;
namespace
{
const juce::Colour bg(0xff111416), sidebar(0xff171b1d), ink(0xffecefe8), muted(0xff929a9b),
    accent(0xffd7ed8d), line(0xff303739);
juce::Font font(float size, bool bold = false)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}
juce::String formatValue(P id, float v)
{
    const auto &d = definitions[index(id)];
    if (juce::String(d.id).containsIgnoreCase("attack") || juce::String(d.id).containsIgnoreCase("release") ||
        id == P::gateHold || id == P::synthDecay || id == P::glide)
        return juce::String(v, v < 10 ? 1 : 0) + " ms";
    if (id == P::freqMin || id == P::freqMax || id == P::synthFilter)
        return v >= 1000 ? juce::String(v / 1000, 1) + " kHz" : juce::String(v, 0) + " Hz";
    if (id == P::formant)
        return juce::String(v, 1) + " st";
    if (id == P::rootNote)
        return juce::MidiMessage::getMidiNoteName(static_cast<int>(v), true, true, 4);
    if (id == P::lfoRate)
        return juce::String(v, 2) + " Hz";
    if (d.min == 0 && d.max == 1)
        return juce::String(v * 100, 0) + "%";
    if (id == P::width)
        return juce::String(v * 100, 0) + "%";
    if (id == P::bitDepth)
        return juce::String(v, 0) + " bit";
    if (id == P::detune)
        return juce::String(v, 0) + " ct";
    if (id == P::inputGain || id == P::outputGain || id == P::presetLevel || id == P::low || id == P::body ||
        id == P::mid || id == P::presence || id == P::air || id == P::gateThreshold || id == P::gateRange ||
        id == P::gateHysteresis || id == P::tilt)
        return juce::String(v, 1) + " dB";
    return juce::String(v, std::abs(v) < 10 ? 1 : 0);
}
double parseValue(P id, juce::String text)
{
    const auto &d = definitions[index(id)];
    if (id == P::rootNote)
        for (int note = 24; note <= 84; ++note)
            if (text.trim().equalsIgnoreCase(juce::MidiMessage::getMidiNoteName(note, true, true, 4)))
                return note;
    double value = text.getDoubleValue();
    if ((d.min == 0 && d.max == 1) || id == P::width)
        value /= 100.0;
    if ((id == P::freqMin || id == P::freqMax || id == P::synthFilter) && text.containsIgnoreCase("k"))
        value *= 1000.0;
    return value;
}
} // namespace
RVLookAndFeel::RVLookAndFeel()
{
    setColour(juce::Label::textColourId, ink);
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffd9d3bf));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff101214));
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::arrowColourId, accent);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff171b1d));
    setColour(juce::PopupMenu::textColourId, ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff384038));
    setColour(juce::TextEditor::backgroundColourId, bg);
    setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TextEditor::outlineColourId, line);
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::TextButton::textColourOnId, accent);
    setColour(juce::ToggleButton::textColourId, ink);
}
void RVLookAndFeel::surface(juce::Graphics &g, juce::Rectangle<float> r, bool recessed)
{
    g.setGradientFill({juce::Colour(recessed ? 0xff0b0e10 : 0xff282c2d), r.getX(), r.getY(),
                       juce::Colour(recessed ? 0xff171b1b : 0xff151819), r.getX(), r.getBottom(), false});
    g.fillRoundedRectangle(r, 5);
    g.setColour(juce::Colours::black.withAlpha(.75f));
    g.drawRoundedRectangle(r.reduced(.5f), 5, 1);
    g.setColour(juce::Colours::white.withAlpha(recessed ? .05f : .13f));
    g.drawLine(r.getX() + 6, r.getY() + 1.5f, r.getRight() - 6, r.getY() + 1.5f);
}
juce::Font RVLookAndFeel::getComboBoxFont(juce::ComboBox &box)
{
    return font(box.getComponentID() == "preset-selector" ? 15.f : box.getWidth() < 110 ? 11.5f : 13.f);
}
juce::Image RVLookAndFeel::knobFace(int size)
{
    size = std::clamp(size, 16, 220);
    if (auto found = faces.find(size); found != faces.end())
        return found->second;
    if (faces.size() > 20)
        faces.clear();
    juce::Image image(juce::Image::ARGB, size * 2, size * 2, true);
    juce::Graphics g(image);
    const float d = static_cast<float>(size * 2), r = d / 2, c = r;
    g.setColour(juce::Colours::black.withAlpha(.65f));
    g.fillEllipse(2, 4, d - 4, d - 4);
    g.setGradientFill({juce::Colour(0xffb3ae9c), 0, 0, juce::Colour(0xff353634), d, d, false});
    g.fillEllipse(3, 2, d - 6, d - 6);
    g.setColour(juce::Colour(0xff101415));
    g.fillEllipse(5, 4, d - 10, d - 10);
    for (int i = 0; i < 64; ++i)
    {
        const float a = static_cast<float>(i) * juce::MathConstants<float>::twoPi / 64;
        g.setColour(juce::Colour(0xffaba997).withAlpha(.12f + .10f * (1 - std::sin(a))));
        g.drawLine(c + std::sin(a) * (r - 6), c - std::cos(a) * (r - 6), c + std::sin(a) * (r - 11),
                   c - std::cos(a) * (r - 11), 1);
    }
    g.setGradientFill(
        {juce::Colour(0xff686c68), r * .45f, r * .25f, juce::Colour(0xff171c1e), r * 1.6f, r * 1.7f, false});
    g.fillEllipse(12, 11, d - 24, d - 24);
    if (const auto face = hardware::artwork("knob_png"); face.isValid())
    {
        juce::Path clip;
        clip.addEllipse(7, 6, d - 14, d - 14);
        juce::Graphics::ScopedSaveState scope(g);
        g.reduceClipRegion(clip);
        g.drawImage(face, juce::Rectangle<float>(7, 6, d - 14, d - 14),
                    juce::RectanglePlacement::stretchToFit);
    }
    g.setColour(juce::Colours::white.withAlpha(.11f));
    g.drawEllipse(12, 11, d - 24, d - 24, 1);
    faces.emplace(size, image);
    return image;
}
void RVLookAndFeel::meter(juce::Graphics &g, juce::Point<float> c, float radius, float level, float peak,
                          float start, float end)
{
    const auto norm = [](float v)
    { return std::clamp((juce::Decibels::gainToDecibels(v, -60.f) + 60) / 60, 0.f, 1.f); };
    const float amount = norm(level), held = norm(peak);
    constexpr int segments = 40;
    for (int i = 0; i < segments; ++i)
    {
        const float part = (static_cast<float>(i) + .5f) / segments;
        const auto a = start + part * (end - start);
        const auto colour = part > .95f  ? juce::Colour(0xffed8664)
                            : part > .8f ? juce::Colour(0xffdcb56c)
                                         : accent;
        const bool lit = part <= amount || (peak > .0001f && std::abs(part - held) < .5f / segments);
        g.setColour(lit ? colour : juce::Colour(0xff363b37));
        g.drawLine(c.x + std::sin(a) * (radius - 3), c.y - std::cos(a) * (radius - 3),
                   c.x + std::sin(a) * radius, c.y - std::cos(a) * radius, radius > 20 ? 2.f : 1.2f);
    }
}
void RVLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int w, int h, float pos, float start,
                                     float end, juce::Slider &slider)
{
    const auto area = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                             static_cast<float>(w), static_cast<float>(h))
                          .reduced(3);
    const float diameter = std::min(area.getWidth(), area.getHeight());
    if (diameter < 12)
        return;
    const auto c = area.getCentre();
    const float r = diameter / 2;
    const auto colour = juce::Colour(static_cast<juce::uint32>(
        static_cast<int>(slider.getProperties().getWithDefault("tint", static_cast<int>(accent.getARGB())))));
    juce::Graphics::ScopedSaveState scope(g);
    const float alpha = slider.isEnabled() ? 1.f : .38f;
    if (slider.getProperties().contains("meter"))
        meter(g, c, r - 1, slider.getProperties()["meter"], slider.getProperties()["peak"], start, end);
    else
    {
        for (int i = 0; i <= 20; ++i)
        {
            const float fraction = static_cast<float>(i) / 20, a = start + fraction * (end - start);
            g.setColour((fraction <= pos ? colour : juce::Colour(0xff51554e)).withMultipliedAlpha(alpha));
            g.drawLine(c.x + std::sin(a) * (r - 2), c.y - std::cos(a) * (r - 2), c.x + std::sin(a) * (r - 4),
                       c.y - std::cos(a) * (r - 4), 1.2f);
        }
    }
    const float faceD = diameter - (diameter < 55 ? 10 : 15);
    g.setOpacity(alpha);
    g.drawImage(knobFace(static_cast<int>(std::ceil(faceD))),
                juce::Rectangle<float>(c.x - faceD / 2, c.y - faceD / 2, faceD, faceD));
    const float a = start + pos * (end - start);
    g.setColour(juce::Colour(0xfff0e4c8).withMultipliedAlpha(alpha));
    g.drawLine(c.x + std::sin(a) * (faceD * .14f), c.y - std::cos(a) * (faceD * .14f),
               c.x + std::sin(a) * (faceD * .41f), c.y - std::cos(a) * (faceD * .41f), 2);
}
void RVLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &b, const juce::Colour &, bool hover,
                                         bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(1);
    surface(g, r, down);
    hardware::texture(g, "button_png", r, .14f);
    if (b.getToggleState() || hover)
    {
        g.setColour(accent.withAlpha(b.getToggleState() ? .12f : .05f));
        g.fillRoundedRectangle(r.reduced(2), 3);
    }
    if (b.getToggleState())
    {
        g.setColour(accent);
        g.drawLine(r.getX() + 8, r.getBottom() - 3, r.getRight() - 8, r.getBottom() - 3, 1.5f);
    }
}
void RVLookAndFeel::drawComboBox(juce::Graphics &g, int w, int h, bool down, int, int, int, int,
                                 juce::ComboBox &box)
{
    surface(g, box.getLocalBounds().toFloat().reduced(1), true);
    g.setColour(box.isEnabled() ? accent : muted.withAlpha(.4f));
    juce::Path arrow;
    const float x = static_cast<float>(w - 15), y = static_cast<float>(h) / 2;
    arrow.addTriangle(x - 4, y - 2, x + 4, y - 2, x, y + 2);
    g.fillPath(arrow);
    if (down || box.hasKeyboardFocus(true))
    {
        g.setColour(accent.withAlpha(.5f));
        g.drawRoundedRectangle(box.getLocalBounds().toFloat().reduced(1), 4, 1);
    }
}
void RVLookAndFeel::drawToggleButton(juce::Graphics &g, juce::ToggleButton &b, bool hover, bool)
{
    const float y = static_cast<float>(b.getHeight()) / 2;
    g.setColour(juce::Colours::black);
    g.fillEllipse(4, y - 4, 8, 8);
    g.setColour(b.getToggleState() ? accent.withAlpha(b.isEnabled() ? 1.f : .25f) : line);
    g.fillEllipse(5, y - 3, 6, 6);
    g.setColour((hover ? ink : juce::Colour(0xffccc8b9)).withAlpha(b.isEnabled() ? 1.f : .35f));
    g.setFont(font(11));
    g.drawFittedText(b.getButtonText(), 18, 0, b.getWidth() - 18, b.getHeight(),
                     juce::Justification::centredLeft, 1);
}
ParameterControl::ParameterControl(RVocoderProcessor &p, P parameter, juce::Colour colour)
    : processor(p), id(parameter), combo(definitions[index(id)].choices[0] != '\0')
{
    const auto &d = definitions[index(id)];
    setComponentID("control-" + juce::String(d.id));
    label.setText(id == P::inputGain    ? "Entrada"
                  : id == P::outputGain ? "Salida"
                                        : d.name,
                  juce::dontSendNotification);
    label.setFont(font(11, true));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffd5cdbb));
    label.setJustificationType(juce::Justification::centred);
    label.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(label);
    if (combo)
    {
        choice.setComponentID(d.id);
        choice.setTitle(d.name);
        choice.addItemList(juce::StringArray::fromTokens(d.choices, "|", ""), 1);
        choice.onChange = [this]
        {
            if (choice.getSelectedId() == 0)
                return;
            auto *parameter = processor.apvts.getParameter(definitions[index(id)].id);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(
                parameter->convertTo0to1(definitions[index(id)].min + choice.getSelectedId() - 1));
            parameter->endChangeGesture();
        };
        addAndMakeVisible(choice);
    }
    else
    {
        slider.setComponentID(d.id);
        slider.setName(d.name);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffd9d3bf));
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 18);
        slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                   juce::MathConstants<float>::pi * 2.8f, true);
        slider.setDoubleClickReturnValue(true, d.initial);
        slider.getProperties().set("tint", static_cast<int>(colour.getARGB()));
        slider.setTooltip(
            juce::String(d.name) +
            juce::String::fromUTF8(" · Doble clic reinicia. Escribe el valor para un ajuste exacto."));
        addAndMakeVisible(slider);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                            d.id, slider);
        slider.valueFromTextFunction = [parameter](const juce::String &s)
        { return parseValue(parameter, s); };
        slider.textFromValueFunction = [parameter](double v)
        { return formatValue(parameter, static_cast<float>(v)); };
        slider.updateText();
    }
    refresh();
}
void ParameterControl::resized()
{
    label.setBounds(0, 0, getWidth(), 18);
    slider.setBounds(1, 18, getWidth() - 2, getHeight() - 18);
    choice.setBounds(3, 18 + (getHeight() - 18 - 28) / 2, getWidth() - 6, 28);
}
void ParameterControl::refresh()
{
    const auto values = processor.readParameters();
    const bool original = values[P::voiceMode] > .5f, wet = values[P::wetOnly] > .5f;
    const bool internal = Routing::from(values).sound == SoundSource::synth;
    const auto group = juce::String(definitions[index(id)].group);
    const bool enabled = !(group == "Synth" && !internal) &&
                         !(original && (id == P::modMix || id == P::carMix || id == P::presetLevel)) &&
                         !(wet && (id == P::mix || id == P::modMix)) &&
                         !((id == P::rootNote || id == P::chord) && values[P::synthMode] > .5f);
    label.setAlpha(enabled ? 1.f : .38f);
    slider.setEnabled(enabled);
    choice.setEnabled(enabled);
    if (combo)
        choice.setSelectedId(static_cast<int>(values[id] - definitions[index(id)].min) + 1,
                             juce::dontSendNotification);
    if (id == P::mix)
    {
        slider.setValue(wet ? 1.f : values[id], juce::dontSendNotification);
        slider.setTooltip("Original / efecto. Desactiva Solo efecto para mezclar voz limpia.");
    }
}
void ParameterControl::setMeter(float level, float peak)
{
    slider.getProperties().set("meter", level);
    slider.getProperties().set("peak", peak);
    slider.repaint();
}
ParameterRack::ParameterRack(RVocoderProcessor &processor, juce::String name, juce::Colour tint,
                             std::vector<Page> sections)
    : title(std::move(name)), colour(tint), pages(std::move(sections))
{
    setComponentID("rack-" + title.toLowerCase());
    selector.setComponentID("rack-page");
    selector.setTitle(title + " — grupo de ajustes");
    for (std::size_t i = 0; i < pages.size(); ++i)
    {
        selector.addItem(pages[i].name, static_cast<int>(i + 1));
        for (auto id : pages[i].parameters)
        {
            auto control = std::make_unique<ParameterControl>(processor, id, colour);
            addChildComponent(*control);
            controls.push_back(std::move(control));
        }
    }
    selector.onChange = [this] { selectPage(selector.getSelectedId() - 1); };
    addAndMakeVisible(selector);
    selectPage(0);
}
void ParameterRack::selectPage(int page)
{
    selected = std::clamp(page, 0, static_cast<int>(pages.size()) - 1);
    selector.setSelectedId(selected + 1, juce::dontSendNotification);
    resized();
}
std::vector<P> ParameterRack::parameterIds() const
{
    std::vector<P> result;
    for (const auto &page : pages)
        result.insert(result.end(), page.parameters.begin(), page.parameters.end());
    return result;
}
void ParameterRack::resized()
{
    selector.setBounds(10, 38, getWidth() - 20, 28);
    int position = 0;
    const auto &visible = pages[static_cast<std::size_t>(selected)].parameters;
    const int cell = (getWidth() - 16) / 2, row = std::max(1, (getHeight() - 80) / 3);
    for (auto &c : controls)
    {
        const bool show = std::find(visible.begin(), visible.end(), c->parameterId()) != visible.end();
        c->setVisible(show);
        if (show)
        {
            c->setBounds(8 + (position % 2) * cell, 76 + (position / 2) * row, cell, row);
            ++position;
        }
    }
}
void ParameterRack::paint(juce::Graphics &g)
{
    RVLookAndFeel::surface(g, getLocalBounds().toFloat().reduced(.5f));
    hardware::texture(g, "rack_png", getLocalBounds().toFloat().reduced(2), .12f);
    g.setColour(colour);
    g.setFont(font(13, true));
    g.drawText(title, 14, 7, getWidth() - 28, 24, juce::Justification::centredLeft);
    g.setColour(colour.withAlpha(.45f));
    g.drawHorizontalLine(32, 14.f, static_cast<float>(getWidth() - 14));
}
void ParameterRack::refresh()
{
    for (auto &c : controls)
        c->refresh();
}
GainStage::GainStage(RVocoderProcessor &p)
    : processor(p), input(p, P::inputGain, accent), output(p, P::outputGain, accent)
{
    setComponentID("gain-stage");
    addAndMakeVisible(input);
    addAndMakeVisible(output);
}
void GainStage::resized()
{
    const int cell = getWidth() / 2;
    input.setBounds(0, 0, cell, getHeight() - 35);
    output.setBounds(cell, 0, getWidth() - cell, getHeight() - 35);
}
void GainStage::refresh()
{
    const std::array<float, 4> fresh{processor.meters.input.load(), processor.meters.modulator.load(),
                                     processor.meters.carrier.load(), processor.meters.output.load()};
    for (std::size_t i = 0; i < fresh.size(); ++i)
    {
        level[i] = std::max(fresh[i], level[i] * .83f);
        if (fresh[i] >= peak[i])
        {
            peak[i] = fresh[i];
            hold[i] = 25;
        }
        else if (hold[i] > 0)
            --hold[i];
        else
            peak[i] = std::max(level[i], peak[i] * .91f);
    }
    input.refresh();
    output.refresh();
    input.setMeter(level[0], peak[0]);
    output.setMeter(level[3], peak[3]);
    repaint();
}
void GainStage::paint(juce::Graphics &g)
{
    const int cell = getWidth() / 2;
    for (int i = 0; i < 2; ++i)
    {
        const int x = i * cell;
        const float y = static_cast<float>(getHeight() - 17);
        RVLookAndFeel::meter(g, {static_cast<float>(x + 18), y}, 12.f, level[static_cast<std::size_t>(i + 1)],
                             peak[static_cast<std::size_t>(i + 1)], juce::MathConstants<float>::pi * 1.2f,
                             juce::MathConstants<float>::pi * 2.8f);
        g.setColour(muted);
        g.setFont(font(9, true));
        g.drawText(i == 0 ? "VOZ" : "CARRIER", x + 36, getHeight() - 32, cell - 36, 14,
                   juce::Justification::left);
        const auto db = juce::Decibels::gainToDecibels(level[static_cast<std::size_t>(i + 1)], -72.f);
        g.setColour(juce::Colour(0xffd9d3bf));
        g.setFont(font(10));
        g.drawText(db <= -72 ? "-inf dBFS" : juce::String(db, 1) + " dBFS", x + 36, getHeight() - 19,
                   cell - 36, 16, juce::Justification::left);
    }
}
RVocoderEditor::RVocoderEditor(RVocoderProcessor &p)
    : AudioProcessorEditor(p), processor(p), gains(p), canvas(p), keyboard(p.keyboard, p.midiMonitor)
{
    setLookAndFeel(&look);
    setOpaque(true);
    setResizable(true, true);
    brand.setText("R / VOCODER", juce::dontSendNotification);
    brand.setFont(font(22, true));
    addAndMakeVisible(brand);
    subtitle.setFont(font(9));
    subtitle.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(subtitle);
    status.setFont(font(11));
    status.setColour(juce::Label::textColourId, accent);
    addAndMakeVisible(status);
    presetSelector.setComponentID("preset-selector");
    presetSelector.setTitle("Preset");
    presetSelector.setTextWhenNothingSelected("Seleccionar preset");
    presetSelector.setTooltip(
        "Presets por categoria. Las flechas recorren todos los sonidos o tus favoritos.");
    presetSelector.onChange = [this]
    {
        const int i = presetSelector.getSelectedId() - 1;
        if (i >= 0 && i < static_cast<int>(processor.presets.all().size()))
        {
            processor.loadPreset(processor.presets.all()[static_cast<std::size_t>(i)]);
            timerCallback();
        }
    };
    addAndMakeVisible(presetSelector);
    prev.setComponentID("preset-prev");
    next.setComponentID("preset-next");
    prev.onClick = [this] { step(-1); };
    next.onClick = [this] { step(1); };
    favorite.setTooltip("Marcar o quitar este preset de favoritos");
    favorite.onClick = [this]
    {
        const auto result = processor.presets.toggleFavorite(processor.presetId().toStdString());
        if (result.wasOk())
            rebuildPresets();
        else
            status.setText(result.getErrorMessage(), juce::dontSendNotification);
    };
    menu.setTooltip("Guardar, favoritos, sonido al azar y reiniciar");
    menu.onClick = [this] { presetMenu(); };
    for (auto *button : {&prev, &next, &favorite, &menu})
        addAndMakeVisible(*button);
    inputLabel.setText("VOZ DESDE", juce::dontSendNotification);
    soundLabel.setText("SONIDO", juce::dontSendNotification);
    for (auto *label : {&inputLabel, &soundLabel, &midiStatus})
    {
        label->setFont(font(10, true));
        label->setColour(juce::Label::textColourId, muted);
        addAndMakeVisible(label);
    }
    midiStatus.setJustificationType(juce::Justification::centredRight);
    const std::array<juce::String, 2> inputs{"Pista", "Sidechain"};
    for (int i = 0; i < 2; ++i)
    {
        auto &button = inputButtons[static_cast<std::size_t>(i)];
        button.setButtonText(inputs[static_cast<std::size_t>(i)]);
        button.setConnectedEdges(i == 0 ? juce::Button::ConnectedOnRight : juce::Button::ConnectedOnLeft);
        button.setTooltip(i == 0 ? "Usar la voz de la pista donde insertaste R-Vocoder."
                                 : "Usar la voz elegida en el selector Side Chain del DAW.");
        button.onClick = [this, i]
        {
            auto routing = Routing::from(processor.readParameters());
            routing.input = i == 0 ? VoiceInput::track : VoiceInput::sidechain;
            processor.setRouting(routing);
            timerCallback();
        };
        addAndMakeVisible(button);
    }
    const std::array<juce::String, 3> sounds{"Voz", "Synth", "Externo"};
    const std::array<juce::String, 3> soundTips{
        juce::String::fromUTF8(
            "Procesar el timbre y formantes de la voz conservando su melodía, sin sintetizador."),
        juce::String::fromUTF8(
            "Vocoder clásico: la voz da las palabras y el sintetizador interno da las notas."),
        "Vocoder con carrier externo: la otra entrada da las notas y la textura."};
    for (int i = 0; i < 3; ++i)
    {
        auto &button = soundButtons[static_cast<std::size_t>(i)];
        button.setButtonText(sounds[static_cast<std::size_t>(i)]);
        button.setTooltip(soundTips[static_cast<std::size_t>(i)]);
        button.setConnectedEdges(
            i == 0 ? juce::Button::ConnectedOnRight
                   : (i == 2 ? juce::Button::ConnectedOnLeft
                             : juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight));
        button.onClick = [this, i]
        {
            auto routing = Routing::from(processor.readParameters());
            routing.sound = static_cast<SoundSource>(i);
            processor.setRouting(routing);
            timerCallback();
        };
        addAndMakeVisible(button);
    }
    if (processor.midiEdition)
    {
        inputButtons[0].setEnabled(false);
        inputButtons[0].setTooltip(juce::String::fromUTF8(
            "En la edición MIDI la voz llega por Side Chain. Para audio directo usa R-Vocoder en Audio FX."));
        soundButtons[2].setEnabled(false);
        soundButtons[2].setTooltip(juce::String::fromUTF8(
            "Para combinar voz y carrier externos usa la edición R-Vocoder de Audio FX."));
    }
    playLabel.setText(juce::String::fromUTF8("ACTIVACIÓN"), juce::dontSendNotification);
    keyboardLabel.setText("TECLADO MIDI", juce::dontSendNotification);
    canvasTitle.setText("FORMA DE LA VOZ", juce::dontSendNotification);
    correlation.setFont(font(10));
    correlation.setJustificationType(juce::Justification::centredRight);
    correlation.setTooltip(
        "Correlacion mono: si es negativa, reduce Width o Spread para conservar cuerpo en mono.");
    addAndMakeVisible(correlation);
    for (auto *label : {&playLabel, &keyboardLabel, &canvasTitle})
    {
        label->setFont(font(11, true));
        label->setColour(juce::Label::textColourId, muted);
        addAndMakeVisible(label);
    }
    auto setPlay = [this](int mode)
    {
        for (auto id : {P::synthMode, P::midiGate})
        {
            if (id == P::midiGate && mode == 0)
                continue;
            auto *parameter = processor.apvts.getParameter(definitions[index(id)].id);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(static_cast<float>(mode));
            parameter->endChangeGesture();
        }
        timerCallback();
    };
    for (int i = 0; i < 2; ++i)
    {
        auto &button = playButtons[static_cast<std::size_t>(i)];
        button.setButtonText(i == 0 ? "Continuo" : "Con teclas");
        button.onClick = [setPlay, i] { setPlay(i); };
        button.setTooltip(
            i == 0 ? "Sonar mientras llega voz. El synth usa su nota o acorde Drone."
                   : "Usar MIDI para las notas del synth y para abrir la salida de cualquier modo.");
        addAndMakeVisible(button);
    }
    keyboard.onUserNote = [this, setPlay]
    {
        if (processor.readParameters()[P::synthMode] < .5f)
            setPlay(1);
    };
    keyboard.setComponentID("performance-keyboard");
    keyGateToggle.setTooltip("En Con teclas, cierra toda la salida al soltar la ultima nota o el pedal. La "
                             "cola se ajusta en VOZ > Gate de voz.");
    wetOnlyToggle.setTooltip("Evita la mezcla directa de voz original. Se conserva al cambiar de preset. Mix "
                             "queda al 100% de efecto.");
    addAndMakeVisible(keyGateToggle);
    addAndMakeVisible(wetOnlyToggle);
    keyGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "midiGate", keyGateToggle);
    wetOnlyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "wetOnly", wetOnlyToggle);
    xyButton.setToggleState(true, juce::dontSendNotification);
    xyButton.onClick = [this]
    {
        canvas.setMode(false);
        xyButton.setToggleState(true, juce::dontSendNotification);
        spectrumButton.setToggleState(false, juce::dontSendNotification);
    };
    spectrumButton.onClick = [this]
    {
        canvas.setMode(true);
        xyButton.setToggleState(false, juce::dontSendNotification);
        spectrumButton.setToggleState(true, juce::dontSendNotification);
    };
    panicButton.setTooltip("Soltar todas las notas MIDI, incluido sustain. No silencia una pista externa que "
                           "suene por separado.");
    panicButton.onClick = [this]
    {
        processor.keyboard.allNotesOff(0);
        processor.panic();
    };
    for (auto *button : {&xyButton, &spectrumButton, &panicButton})
        addAndMakeVisible(*button);
    for (auto id : {P::character, P::clarity, P::formant, P::width, P::drive, P::motion, P::air, P::mix})
    {
        auto control = std::make_unique<ParameterControl>(p, id, accent);
        addAndMakeVisible(*control);
        macros.push_back(std::move(control));
    }
    using Page = ParameterRack::Page;
    racks[0] = std::make_unique<ParameterRack>(
        p, "VOZ", juce::Colour(0xffdcb878),
        std::vector<Page>{
            {"Vocoder", {P::bands, P::envAttack, P::envRelease, P::amount, P::definition, P::sibilance}},
            {"Espectro", {P::freqMin, P::freqMax, P::tilt, P::bandShift, P::formantQ}},
            {"Identidad", {P::unvoiced, P::breath, P::nasal, P::size, P::identity, P::throat}},
            {"Gate de voz",
             {P::gateOn, P::gateThreshold, P::gateAttack, P::gateHold, P::gateRelease, P::midiGateRelease}},
            {"Gate avanzado", {P::gateRange, P::gateHysteresis}}});
    racks[1] = std::make_unique<ParameterRack>(
        p, "SYNTH", accent,
        std::vector<Page>{
            {"Osciladores", {P::wave, P::oscMix, P::detune, P::unison, P::octave, P::synthFilter}},
            {"Envolvente",
             {P::resonance, P::synthAttack, P::synthDecay, P::synthSustain, P::synthRelease, P::glide}},
            {"Drone", {P::rootNote, P::chord}}});
    racks[2] = std::make_unique<ParameterRack>(
        p, "FX", juce::Colour(0xff9bb8cc),
        std::vector<Page>{
            {"Color", {P::distortion, P::speaker, P::warmth, P::vintage, P::modern, P::exciter}},
            {"Digital", {P::crusher, P::bitDepth, P::reduction, P::noise}},
            {"Ecualizador", {P::low, P::body, P::mid, P::presence}},
            {"Espacio", {P::unisonWidth, P::spread, P::stereoMod}},
            {"Mezcla", {P::modMix, P::carMix, P::presetLevel}}});
    racks[3] = std::make_unique<ParameterRack>(
        p, "MOTION", juce::Colour(0xffbaa5d0),
        std::vector<Page>{
            {"Reloj y forma", {P::lfoRate, P::tempoSync, P::division, P::rhythm, P::lfoShape, P::follower}},
            {"Destinos", {P::autoPan, P::bandMotion, P::filterMotion, P::formantMotion, P::widthMotion}}});
    for (auto &rack : racks)
        addAndMakeVisible(*rack);
    addAndMakeVisible(gains);
    addAndMakeVisible(canvas);
    addAndMakeVisible(bypass);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "bypass", bypass);
    keyboard.setAvailableRange(0, 127);
    keyboard.setLowestVisibleKey(33);
    keyboard.setOctaveForMiddleC(4);
    keyboard.setKeyWidth(22);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffe2ddcd));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff171b1d));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, accent);
    keyboard.setWantsKeyboardFocus(false);
    addAndMakeVisible(keyboard);
    setResizeLimits(1080, 760, 1800, 1100);
    setSize(1320, 840);
    rebuildPresets();
    startTimerHz(25);
    timerCallback();
}
RVocoderEditor::~RVocoderEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}
void RVocoderEditor::resized()
{
    const int w = getWidth(), h = getHeight(), x = 20, width = w - 40;
    chassis.setBounds(getLocalBounds());
    brand.setBounds(x, 10, 214, 29);
    subtitle.setBounds(x, 38, 220, 17);
    const int presetX = 278, presetEnd = w - 224;
    prev.setBounds(238, 15, 32, 30);
    presetSelector.setBounds(presetX, 15, presetEnd - presetX, 30);
    next.setBounds(presetEnd + 8, 15, 32, 30);
    favorite.setBounds(w - 178, 15, 36, 30);
    menu.setBounds(w - 136, 15, 36, 30);
    bypass.setBounds(w - 90, 16, 74, 28);
    const int playingWidth = width - 252, groupGap = 16;
    const int inputW = playingWidth * 25 / 100, soundW = playingWidth * 36 / 100;
    const int soundX = x + inputW + groupGap, playX = soundX + soundW + groupGap;
    inputLabel.setBounds(x, 67, inputW, 15);
    soundLabel.setBounds(soundX, 67, soundW, 15);
    playLabel.setBounds(playX, 67, x + playingWidth - playX, 15);
    for (int i = 0; i < 2; ++i)
        inputButtons[static_cast<std::size_t>(i)].setBounds(x + i * inputW / 2, 87, inputW / 2 - 3, 28);
    for (int i = 0; i < 3; ++i)
        soundButtons[static_cast<std::size_t>(i)].setBounds(soundX + i * soundW / 3, 87, soundW / 3 - 3, 28);
    const int playW = (x + playingWidth - playX) / 2;
    for (int i = 0; i < 2; ++i)
        playButtons[static_cast<std::size_t>(i)].setBounds(playX + i * playW, 87, playW - 3, 28);
    keyboardLabel.setBounds(x, 126, 112, 20);
    keyGateToggle.setBounds(x + 118, 123, 132, 26);
    wetOnlyToggle.setBounds(x + 254, 123, 106, 26);
    midiStatus.setBounds(x + playingWidth - 132, 126, 132, 20);
    keyboard.setBounds(x, 154, playingWidth, 62);
    keyboard.setKeyWidth(static_cast<float>(std::clamp((playingWidth - 32) / 35, 22, 36)));
    gains.setBounds(w - 246, 62, 226, 155);
    for (std::size_t i = 0; i < macros.size(); ++i)
        macros[i]->setBounds(x + static_cast<int>(i) * width / 8, 238, width / 8, 109);
    const int graphW = std::clamp(width * 23 / 100, 225, 360), top = 366;
    canvasTitle.setBounds(x + 4, top + 7, graphW - 8, 24);
    xyButton.setBounds(x + 8, top + 40, 52, 26);
    spectrumButton.setBounds(x + 66, top + 40, 98, 26);
    correlation.setBounds(x + 168, top + 40, graphW - 176, 26);
    canvas.setBounds(x + 4, top + 76, graphW - 8, h - top - 118);
    const int rackX = x + graphW + 10, rackW = (width - graphW - 40) / 4;
    for (std::size_t i = 0; i < racks.size(); ++i)
        racks[i]->setBounds(rackX + static_cast<int>(i) * (rackW + 10), top, rackW, h - top - 42);
    status.setBounds(x, h - 32, width - 112, 24);
    panicButton.setBounds(w - 108, h - 34, 88, 26);
    repaint();
}
void RVocoderEditor::paint(juce::Graphics &g)
{
    chassis.paint(g);
    const auto width = static_cast<float>(getWidth());
    g.setColour(juce::Colours::black.withAlpha(.55f));
    g.drawHorizontalLine(58, 16, width - 16);
    RVLookAndFeel::surface(g, juce::Rectangle<float>(20, 230, width - 40, 123));
    const int graphW = std::clamp((getWidth() - 40) * 23 / 100, 225, 360);
    RVLookAndFeel::surface(
        g, juce::Rectangle<float>(20, 366, static_cast<float>(graphW), static_cast<float>(getHeight() - 408)),
        true);
}
void RVocoderEditor::rebuildPresets()
{
    filtered.clear();
    presetSelector.clear(juce::dontSendNotification);
    juce::StringArray categories;
    const auto &all = processor.presets.all();
    for (std::size_t i = 0; i < all.size(); ++i)
    {
        if (favoritesOnly && !processor.presets.favorite(all[i].id))
            continue;
        filtered.push_back(static_cast<int>(i));
        categories.addIfNotAlreadyThere(all[i].category);
    }
    auto *root = presetSelector.getRootMenu();
    for (const auto &category : categories)
    {
        juce::PopupMenu entries;
        for (auto i : filtered)
        {
            const auto &preset = all[static_cast<std::size_t>(i)];
            if (juce::String(preset.category) == category)
                entries.addItem(i + 1, juce::String(preset.name), true,
                                processor.presetId() == juce::String(preset.id));
        }
        root->addSubMenu(category, entries);
    }
    if (filtered.empty())
        root->addItem(10001, "No hay favoritos guardados", false, false);
    lastId.clear();
    timerCallback();
}
void RVocoderEditor::step(int delta)
{
    if (filtered.empty())
        return;
    int selected = -1;
    for (std::size_t i = 0; i < filtered.size(); ++i)
        if (processor.presets.all()[static_cast<std::size_t>(filtered[i])].id ==
            processor.presetId().toStdString())
            selected = static_cast<int>(i);
    const int count = static_cast<int>(filtered.size());
    const int nextIndex = selected < 0 ? (delta > 0 ? 0 : count - 1) : (selected + delta + count) % count;
    processor.loadPreset(
        processor.presets.all()[static_cast<std::size_t>(filtered[static_cast<std::size_t>(nextIndex)])]);
    timerCallback();
}
void RVocoderEditor::presetMenu()
{
    juce::PopupMenu choices;
    choices.addItem(1, "Solo favoritos", true, favoritesOnly);
    choices.addItem(2, "Sonido al azar", !filtered.empty());
    choices.addSeparator();
    choices.addItem(3, "Guardar preset...");
    choices.addItem(4, "Borrar preset personal...", processor.presetId().startsWith("user-"));
    choices.addItem(5, "Reiniciar sonido");
    juce::Component::SafePointer<RVocoderEditor> safe(this);
    choices.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(menu),
        [safe](int result)
        {
            if (!safe)
                return;
            if (result == 1)
            {
                safe->favoritesOnly = !safe->favoritesOnly;
                safe->rebuildPresets();
            }
            if (result == 2 && !safe->filtered.empty())
            {
                const int n =
                    juce::Random::getSystemRandom().nextInt(static_cast<int>(safe->filtered.size()));
                safe->processor.loadPreset(
                    safe->processor.presets
                        .all()[static_cast<std::size_t>(safe->filtered[static_cast<std::size_t>(n)])]);
                safe->timerCallback();
            }
            if (result == 3)
                safe->savePreset();
            if (result == 4)
                safe->deletePreset();
            if (result == 5)
            {
                safe->processor.resetSound();
                safe->timerCallback();
            }
        });
}
void RVocoderEditor::timerCallback()
{
    keyboard.refreshIncomingNotes();
    for (auto &rack : racks)
        if (rack)
            rack->refresh();
    for (auto &control : macros)
        control->refresh();
    gains.refresh();
    canvas.refresh();
    const float mono = processor.meters.correlation.load();
    correlation.setText("Mono " + juce::String(mono >= 0 ? "+" : "") + juce::String(mono, 2),
                        juce::dontSendNotification);
    correlation.setColour(juce::Label::textColourId, mono < 0 ? juce::Colour(0xffed8664) : muted);
    const auto currentId = processor.presetId();
    if (lastId != currentId)
    {
        lastId = currentId;
        int selected = 0;
        for (auto i : filtered)
            if (processor.presets.all()[static_cast<std::size_t>(i)].id == currentId.toStdString())
                selected = i + 1;
        presetSelector.setSelectedId(selected, juce::dontSendNotification);
        presetSelector.setText(processor.presetName(), juce::dontSendNotification);
    }
    favorite.setToggleState(processor.presets.favorite(currentId.toStdString()), juce::dontSendNotification);
    const auto p = processor.readParameters();
    const auto routing = Routing::from(p);
    keyGateToggle.setEnabled(p[P::synthMode] > .5f);
    for (int i = 0; i < 2; ++i)
        playButtons[static_cast<std::size_t>(i)].setToggleState((p[P::synthMode] > .5f ? 1 : 0) == i,
                                                                juce::dontSendNotification);
    for (int i = 0; i < 2; ++i)
        inputButtons[static_cast<std::size_t>(i)].setToggleState(static_cast<int>(routing.input) == i,
                                                                 juce::dontSendNotification);
    for (int i = 0; i < 3; ++i)
        soundButtons[static_cast<std::size_t>(i)].setToggleState(static_cast<int>(routing.sound) == i,
                                                                 juce::dontSendNotification);
    const int bands = bandCounts[static_cast<std::size_t>(static_cast<int>(p[P::bands]))];
    const auto soundName = routing.sound == SoundSource::voice
                               ? "VOZ ORIGINAL"
                               : (routing.sound == SoundSource::external
                                      ? "CARRIER EXTERNO"
                                      : (p[P::synthMode] > .5f ? "SYNTH MIDI" : "SYNTH DRONE"));
    subtitle.setText(juce::String(bands) + " BANDS  /  " + soundName + "  /  " +
                         juce::String(processor.getSampleRate() / 1000, 1) + " kHz",
                     juce::dontSendNotification);
    if (const auto seq = processor.midiMonitor.sequence(); seq != lastMidiSequence)
    {
        lastMidiSequence = seq;
        midiFlashTicks = 25;
    }
    else if (midiFlashTicks > 0)
        --midiFlashTicks;
    midiStatus.setText(midiFlashTicks > 0 ? juce::String::fromUTF8("MIDI  ·  ") +
                                                juce::MidiMessage::getMidiNoteName(
                                                    processor.midiMonitor.latestNote(), true, true, 4)
                                          : "MIDI",
                       juce::dontSendNotification);
    midiStatus.setColour(juce::Label::textColourId, midiFlashTicks > 0 ? accent : muted);
    juce::String message;
    if (p[P::bypass] > .5f)
        message = "Bypass activo: pasa el audio original de la pista.";
    else if (processor.meters.modulator.load() < 0.0001f)
        message = routing.input == VoiceInput::sidechain
                      ? "No entra voz: elige su pista en Side Chain del DAW y reproduce audio."
                      : juce::String::fromUTF8(
                            "No entra voz: reproduce o activa la escucha de la pista donde está R-Vocoder.");
    else if (p[P::synthMode] > .5f && p[P::midiGate] > .5f && !processor.meters.midiHeld.load())
        message = processor.meters.keyGate.load() > .001f
                      ? "Soltando notas... cerrando la salida."
                      : "La voz llega. Toca una tecla para abrir la salida.";
    else if (p[P::synthMode] > .5f && p[P::midiGate] < .5f)
        message = "Cortar al soltar esta desactivado: la voz y los efectos pueden pasar sin notas.";
    else if (routing.sound == SoundSource::voice)
        message = juce::String::fromUTF8(
            "Voz original: conserva su melodía. Formant, Character y FX modifican su timbre.");
    else if (routing.sound == SoundSource::external && processor.meters.carrier.load() < 0.0001f)
        message =
            routing.input == VoiceInput::track
                ? "Falta el sonido externo: elige un sintetizador o carrier en Side Chain del DAW."
                : juce::String::fromUTF8(
                      "Falta el sonido externo: reproduce el carrier en la pista donde está R-Vocoder.");
    else if (routing.sound == SoundSource::synth && p[P::synthMode] > .5f &&
             processor.meters.carrier.load() < 0.0001f)
        message = "La voz llega. Toca MIDI para darle notas, o selecciona Continuo.";
    else
        message = (p[P::wetOnly] > .5f ? "SOLO EFECTO" : "VOZ + EFECTO") + juce::String("   /   ") +
                  (p[P::synthMode] > .5f ? "MIDI ABIERTO" : "CONTINUO") +
                  "   /   Arrastra el grafico para dar forma al sonido.";
    status.setText(message, juce::dontSendNotification);
    repaint();
}
void RVocoderEditor::savePreset()
{
    auto *alert = new juce::AlertWindow("Guardar preset", "Guarda este sonido en tu biblioteca personal.",
                                        juce::MessageBoxIconType::NoIcon);
    alert->addTextEditor("name", processor.presetName(), "Nombre");
    alert->addButton("Guardar", 1);
    alert->addButton("Cancelar", 0);
    juce::Component::SafePointer<RVocoderEditor> safe(this);
    alert->enterModalState(
        true,
        juce::ModalCallbackFunction::create(
            [safe, alert](int result)
            {
                if (safe && result == 1)
                {
                    std::string savedId;
                    auto r = safe->processor.presets.save(alert->getTextEditorContents("name"),
                                                          safe->processor.readParameters(), &savedId);
                    if (r.failed())
                        safe->status.setText(r.getErrorMessage(), juce::dontSendNotification);
                    else
                    {
                        for (const auto &saved : safe->processor.presets.all())
                            if (saved.id == savedId)
                            {
                                safe->processor.loadPreset(saved);
                                break;
                            }
                        safe->rebuildPresets();
                    }
                }
            }),
        true);
}
void RVocoderEditor::deletePreset()
{
    const auto id = processor.presetId().toStdString();
    if (id.rfind("user-", 0) != 0)
        return;
    juce::Component::SafePointer<RVocoderEditor> safe(this);
    juce::AlertWindow::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon, "Borrar preset",
        juce::String::fromUTF8("Se borrará el preset personal «") + processor.presetName() +
            juce::String::fromUTF8("». Los proyectos guardados conservan su sonido."),
        "Borrar", "Cancelar", this,
        juce::ModalCallbackFunction::create(
            [safe, id](int result)
            {
                if (safe && result)
                {
                    auto r = safe->processor.presets.remove(id);
                    if (r.wasOk())
                    {
                        safe->processor.resetSound();
                        safe->rebuildPresets();
                    }
                }
            }));
}
