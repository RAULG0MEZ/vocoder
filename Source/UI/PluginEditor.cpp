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
    setColour(juce::ResizableWindow::backgroundColourId, bg);
    setColour(juce::Label::textColourId, ink);
    setColour(juce::Slider::textBoxTextColourId, ink);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId, sidebar);
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::outlineColourId, line);
    setColour(juce::ComboBox::arrowColourId, accent);
    setColour(juce::PopupMenu::backgroundColourId, sidebar);
    setColour(juce::PopupMenu::textColourId, ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, line);
    setColour(juce::TextEditor::backgroundColourId, bg);
    setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TextEditor::outlineColourId, line);
    setColour(juce::TextEditor::focusedOutlineColourId, accent);
    setColour(juce::ListBox::backgroundColourId, sidebar);
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::TextButton::textColourOnId, bg);
    setColour(juce::ToggleButton::textColourId, muted);
    setColour(juce::ToggleButton::tickColourId, accent);
}
void RVLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int w, int h, float pos, float start,
                                     float end, juce::Slider &)
{
    auto area = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w),
                                       static_cast<float>(h))
                    .reduced(7);
    const float size = std::min(area.getWidth(), area.getHeight()), r = size / 2, cx = area.getCentreX(),
                cy = area.getCentreY();
    juce::Path track;
    track.addCentredArc(cx, cy, r - 3, r - 3, 0, start, end, true);
    g.setColour(line);
    g.strokePath(track, juce::PathStrokeType(3));
    juce::Path value;
    value.addCentredArc(cx, cy, r - 3, r - 3, 0, start, start + pos * (end - start), true);
    g.setColour(accent);
    g.strokePath(value, juce::PathStrokeType(3, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343d40), cx, cy - r, juce::Colour(0xff1b2225), cx,
                                           cy + r, false));
    g.fillEllipse(cx - r + 11, cy - r + 11, size - 22, size - 22);
    const float angle = start + pos * (end - start);
    g.setColour(ink);
    g.drawLine(cx + std::sin(angle) * (r - 22), cy - std::cos(angle) * (r - 22),
               cx + std::sin(angle) * (r - 14), cy - std::cos(angle) * (r - 14), 2);
}
void RVLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &b, const juce::Colour &, bool hover,
                                         bool down)
{
    g.setColour(b.getToggleState()
                    ? accent
                    : (down ? line : (hover ? juce::Colour(0xff30393b) : juce::Colour(0xff232a2d))));
    g.fillRoundedRectangle(b.getLocalBounds().toFloat(), 5);
}
ParameterPanel::ParameterPanel(RVocoderProcessor &p) : processor(p)
{
    for (std::size_t i = 0; i < parameterCount; ++i)
    {
        const auto &d = definitions[i];
        if (i == index(P::route) || i == index(P::bypass))
            continue;
        auto c = std::make_unique<Control>();
        c->id = static_cast<P>(i);
        c->slider.setName(d.name);
        c->choice.setTitle(d.name);
        c->label.setText(d.name, juce::dontSendNotification);
        c->label.setFont(font(13));
        c->label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(c->label);
        c->combo = d.choices[0] != '\0';
        if (c->combo)
        {
            const auto choices = juce::StringArray::fromTokens(d.choices, "|", "");
            c->choice.addItemList(choices, 1);
            c->choice.setSelectedId(static_cast<int>(processor.readParameters().values[i] - d.min) + 1,
                                    juce::dontSendNotification);
            c->choice.onChange = [this, i, ptr = c.get()]
            {
                auto *param = processor.apvts.getParameter(definitions[i].id);
                param->beginChangeGesture();
                param->setValueNotifyingHost(
                    param->convertTo0to1(definitions[i].min + ptr->choice.getSelectedId() - 1));
                param->endChangeGesture();
            };
            addAndMakeVisible(c->choice);
        }
        else
        {
            c->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            c->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 22);
            c->slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                          juce::MathConstants<float>::pi * 2.8f, true);
            c->slider.setDoubleClickReturnValue(true, d.initial);
            const P id = c->id;
            c->slider.textFromValueFunction = [id](double v)
            { return formatValue(id, static_cast<float>(v)); };
            c->slider.setTooltip(juce::String(d.name) + " · Doble clic para restablecer");
            addAndMakeVisible(c->slider);
            c->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processor.apvts, d.id, c->slider);
            c->slider.valueFromTextFunction = [id](const juce::String &text) { return parseValue(id, text); };
            c->slider.textFromValueFunction = [id](double v)
            { return formatValue(id, static_cast<float>(v)); };
            c->slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            c->slider.updateText();
        }
        controls.push_back(std::move(c));
    }
    showPage(0);
}
void ParameterPanel::showPage(int v)
{
    page = v;
    layout(getWidth());
}
void ParameterPanel::layout(int width, int availableHeight)
{
    headings.clear();
    const int mainRowHeight = std::clamp((availableHeight - 24) / 2, 110, 160);
    const int cols = 4, cell = std::max(140, (width - 36) / cols);
    int y = 8, col = 0;
    juce::String last;
    const auto values = processor.readParameters();
    for (auto &c : controls)
    {
        const auto group = juce::String(definitions[index(c->id)].group);
        bool visible = (page == 0 && group == "Main") ||
                       (page == 1 && (group == "Analysis" || group == "Intelligibility" ||
                                      group == "Identity" || group == "Gate" || group == "Blend")) ||
                       (page == 2 && group == "Synth") ||
                       (page == 3 && (group == "Stereo" || group == "Character" || group == "Tone" ||
                                      group == "Routing")) ||
                       (page == 4 && group == "Motion");
        c->label.setVisible(visible);
        c->slider.setVisible(visible && !c->combo);
        c->choice.setVisible(visible && c->combo);
        if (!visible)
            continue;
        if (page != 0 && last != group)
        {
            if (col != 0)
            {
                y += 130;
                col = 0;
            }
            headings.push_back({group.toUpperCase(), y});
            y += 34;
            last = group;
        }
        const int height = page == 0 ? mainRowHeight : 130, x = 18 + col * cell;
        c->label.setBounds(x, y, cell, 23);
        if (c->combo)
        {
            c->choice.setBounds(x + 8, y + 48, cell - 16, 32);
            c->choice.setSelectedId(static_cast<int>(values[c->id] - definitions[index(c->id)].min) + 1,
                                    juce::dontSendNotification);
        }
        else
            c->slider.setBounds(x + 6, y + 24, cell - 12, height - 30);
        if (++col == cols)
        {
            col = 0;
            y += height;
        }
    }
    if (col)
        y += page == 0 ? mainRowHeight : 130;
    setSize(width, std::max(240, y + 16));
    repaint();
}
void ParameterPanel::refreshChoices()
{
    const auto values = processor.readParameters();
    for (auto &c : controls)
        if (c->combo)
            c->choice.setSelectedId(static_cast<int>(values[c->id] - definitions[index(c->id)].min) + 1,
                                    juce::dontSendNotification);
}
void ParameterPanel::paint(juce::Graphics &g)
{
    g.setFont(font(11, true));
    for (const auto &h : headings)
    {
        g.setColour(accent);
        g.drawText(h.text, 24, h.y, getWidth() - 48, 24, juce::Justification::centredLeft);
        g.setColour(line);
        g.drawHorizontalLine(h.y + 26, 24, static_cast<float>(getWidth() - 24));
    }
}
RVocoderEditor::RVocoderEditor(RVocoderProcessor &p)
    : AudioProcessorEditor(p), processor(p), panel(p),
      keyboard(p.keyboard, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&look);
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(960, 640, 1600, 1100);
    brand.setText("R / VOCODER", juce::dontSendNotification);
    brand.setFont(font(25, true));
    addAndMakeVisible(brand);
    presetTitle.setFont(font(29, true));
    addAndMakeVisible(presetTitle);
    subtitle.setFont(font(12));
    subtitle.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(subtitle);
    status.setFont(font(11));
    status.setColour(juce::Label::textColourId, accent);
    addAndMakeVisible(status);
    category.addItem("All sounds", 1);
    juce::StringArray categories;
    for (const auto &item : processor.presets.all())
        categories.addIfNotAlreadyThere(item.category);
    categories.addIfNotAlreadyThere("USER");
    category.addItemList(categories, 2);
    category.setSelectedId(1);
    category.onChange = [this] { filterPresets(); };
    addAndMakeVisible(category);
    search.setTextToShowWhenEmpty("Search sounds or tags...", muted);
    search.onTextChange = [this] { filterPresets(); };
    addAndMakeVisible(search);
    list.setModel(this);
    list.setRowHeight(43);
    list.setOutlineThickness(0);
    addAndMakeVisible(list);
    for (auto *b : {&prev, &next, &random, &favorite, &onlyFavorites, &save, &remove, &reset})
        addAndMakeVisible(*b);
    prev.onClick = [this] { step(-1); };
    next.onClick = [this] { step(1); };
    random.onClick = [this]
    {
        if (!filtered.empty())
            list.selectRow(juce::Random::getSystemRandom().nextInt(static_cast<int>(filtered.size())));
    };
    favorite.onClick = [this]
    {
        const auto result = processor.presets.toggleFavorite(processor.presetId().toStdString());
        if (result.failed())
            status.setText(result.getErrorMessage(), juce::dontSendNotification);
        filterPresets();
    };
    onlyFavorites.onClick = [this]
    {
        favoritesOnly = !favoritesOnly;
        onlyFavorites.setToggleState(favoritesOnly, juce::dontSendNotification);
        filterPresets();
    };
    save.onClick = [this] { savePreset(); };
    remove.onClick = [this] { deletePreset(); };
    reset.onClick = [this] { processor.resetSound(); };
    sources.addItemList(juce::StringArray::fromTokens(definitions[index(P::route)].choices, "|", ""), 1);
    sources.onChange = [this]
    {
        auto *p = processor.apvts.getParameter("route");
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(sources.getSelectedId() - 1)));
        p->endChangeGesture();
    };
    addAndMakeVisible(sources);
    const std::array<juce::String, 5> names{"MAIN", "VOCODER", "SYNTH", "FX", "MOTION"};
    for (int i = 0; i < 5; ++i)
    {
        tabs[static_cast<std::size_t>(i)].setButtonText(names[static_cast<std::size_t>(i)]);
        tabs[static_cast<std::size_t>(i)].onClick = [this, i] { setPage(i); };
        addAndMakeVisible(tabs[static_cast<std::size_t>(i)]);
    }
    addAndMakeVisible(bypass);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "bypass", bypass);
    viewport.setViewedComponent(&panel, false);
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(8);
    addAndMakeVisible(viewport);
    keyboard.setAvailableRange(36, 84);
    keyboard.setLowestVisibleKey(48);
    keyboard.setKeyWidth(24);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffaeb7b3));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, bg);
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, accent);
    keyboard.setWantsKeyboardFocus(false);
    addAndMakeVisible(keyboard);
    setSize(1100, 700);
    filterPresets();
    setPage(0);
    startTimerHz(25);
    timerCallback();
}
RVocoderEditor::~RVocoderEditor()
{
    stopTimer();
    list.setModel(nullptr);
    setLookAndFeel(nullptr);
}
void RVocoderEditor::setPage(int p)
{
    currentPage = p;
    for (int i = 0; i < 5; ++i)
        tabs[static_cast<std::size_t>(i)].setToggleState(i == p, juce::dontSendNotification);
    panel.showPage(p);
    viewport.setViewPosition(0, 0);
    resized();
}
void RVocoderEditor::resized()
{
    const int w = getWidth(), h = getHeight(), left = 246;
    brand.setBounds(22, 20, 224, 40);
    for (int i = 0; i < 5; ++i)
        tabs[static_cast<std::size_t>(i)].setBounds(left + 24 + i * 94, 25, 86, 30);
    bypass.setBounds(w - 120, 24, 96, 32);
    category.setBounds(18, 113, 208, 32);
    search.setBounds(18, 155, 208, 33);
    list.setBounds(10, 200, 226, h - 354);
    onlyFavorites.setBounds(18, h - 144, 100, 29);
    favorite.setBounds(126, h - 144, 100, 29);
    save.setBounds(18, h - 105, 100, 29);
    remove.setBounds(126, h - 105, 100, 29);
    sources.setBounds(left + 24, 94, std::min(430, w - left - 280), 32);
    reset.setBounds(w - 100, 94, 76, 30);
    presetTitle.setBounds(left + 20, 143, w - left - 275, 42);
    subtitle.setBounds(left + 24, 187, w - left - 100, 24);
    prev.setBounds(w - 216, 151, 36, 30);
    next.setBounds(w - 174, 151, 36, 30);
    random.setBounds(w - 130, 151, 106, 30);
    viewport.setBounds(left + 8, 210, w - left - 26, h - 390);
    panel.layout(viewport.getWidth() - 10, viewport.getHeight());
    keyboard.setBounds(left + 30, h - 174, w - left - 58, 50);
    keyboard.setVisible(currentPage == 2);
    status.setBounds(left + 24, h - 113, w - left - 48, 22);
    repaint();
}
void RVocoderEditor::paint(juce::Graphics &g)
{
    const int w = getWidth(), h = getHeight(), left = 246;
    g.fillAll(bg);
    g.setColour(sidebar);
    g.fillRect(0, 80, left, h - 80);
    g.setColour(line);
    g.drawHorizontalLine(79, 0, static_cast<float>(w));
    g.drawVerticalLine(left, 80, static_cast<float>(h));
    g.setFont(font(11, true));
    g.setColour(muted);
    g.drawText("SOUND LIBRARY  /  100 FACTORY", 20, 84, 220, 20, juce::Justification::centredLeft);
    g.drawText("RSTK   /   0.1.0", 22, h - 38, 180, 20, juce::Justification::centredLeft);
    if (currentPage != 2)
    {
        const int count = processor.meters.bandCount.load();
        const float plotX = static_cast<float>(left + 30), plotW = static_cast<float>(w - left - 58),
                    base = static_cast<float>(h - 129), barW = plotW / static_cast<float>(count);
        g.setColour(muted);
        g.setFont(font(10));
        g.drawText("SPECTRAL ENVELOPES", left + 30, h - 174, 220, 16, juce::Justification::centredLeft);
        for (int i = 0; i < count; ++i)
        {
            const float v = std::clamp(
                std::sqrt(processor.meters.bands[static_cast<std::size_t>(i)].load()) * 100, 2.0f, 30.0f);
            g.setColour(accent.withAlpha(0.35f + 0.5f * static_cast<float>(i) / static_cast<float>(count)));
            g.fillRoundedRectangle(plotX + static_cast<float>(i) * barW, base - v, std::max(2.0f, barW - 3),
                                   v, 2);
        }
    }
    g.setColour(line);
    g.drawHorizontalLine(h - 95, static_cast<float>(left + 24), static_cast<float>(w - 24));
    const std::array<juce::String, 4> labels{"INPUT", "MODULATOR", "CARRIER", "OUTPUT"};
    const int cell = (w - left - 58) / 4;
    for (int i = 0; i < 4; ++i)
    {
        int x = left + 30 + i * cell;
        const float level = displayMeters[static_cast<std::size_t>(i)],
                    db = juce::Decibels::gainToDecibels(level, -72.0f);
        g.setFont(font(10, true));
        g.setColour(muted);
        g.drawText(labels[static_cast<std::size_t>(i)], x, h - 80, cell - 20, 18,
                   juce::Justification::centredLeft);
        g.setColour(line);
        g.fillRect(x, h - 54, cell - 22, 4);
        g.setColour(db > -.5f ? juce::Colour(0xfff39d86) : accent);
        g.fillRect(x, h - 54,
                   static_cast<int>(std::clamp((db + 60) / 60, 0.0f, 1.0f) * static_cast<float>(cell - 22)),
                   4);
        g.setColour(muted);
        g.setFont(font(10));
        g.drawText(db <= -72 ? "-inf dB" : juce::String(db, 1) + " dB", x, h - 42, cell - 20, 18,
                   juce::Justification::centredLeft);
    }
}
void RVocoderEditor::timerCallback()
{
    panel.refreshChoices();
    const auto p = processor.readParameters();
    sources.setSelectedId(static_cast<int>(p[P::route]) + 1, juce::dontSendNotification);
    presetTitle.setText(processor.presetName(), juce::dontSendNotification);
    const int bands = bandCounts[static_cast<std::size_t>(static_cast<int>(p[P::bands]))];
    subtitle.setText(juce::String(bands) + " BANDS  /  " +
                         (p[P::synthMode] > .5f ? "MIDI CARRIER" : "DRONE CARRIER") + "  /  " +
                         juce::String(processor.getSampleRate() / 1000, 1) + " kHz",
                     juce::dontSendNotification);
    const auto id = processor.presetId();
    if (id != lastId)
    {
        lastId = id;
        updating = true;
        for (std::size_t i = 0; i < filtered.size(); ++i)
            if (processor.presets.all()[static_cast<std::size_t>(filtered[i])].id == id.toStdString())
            {
                list.selectRow(static_cast<int>(i));
                break;
            }
        updating = false;
        panel.layout(panel.getWidth(), viewport.getHeight());
    }
    favorite.setToggleState(processor.presets.favorite(id.toStdString()), juce::dontSendNotification);
    remove.setEnabled(id.startsWith("user-"));
    const std::array<float, 4> levels{processor.meters.input.load(), processor.meters.modulator.load(),
                                      processor.meters.carrier.load(), processor.meters.output.load()};
    for (std::size_t i = 0; i < 4; ++i)
        displayMeters[i] = std::max(levels[i], displayMeters[i] * .88f);
    juce::String message;
    if ((p[P::route] > 1.5f && processor.meters.modulator.load() < 0.0001f) ||
        (p[P::route] > 0.5f && p[P::route] < 1.5f && processor.meters.carrier.load() < 0.0001f))
        message = "Selecciona la pista de voz o carrier en el Side Chain de tu DAW.";
    else if (p[P::synthMode] > .5f && processor.meters.carrier.load() < 0.0001f)
        message = "Toca MIDI para activar el carrier. También puedes usar el teclado en SYNTH.";
    else
        message = "GATE " + juce::String(processor.meters.gate.load() > .5f ? "OPEN" : "CLOSED") +
                  "     /     CORRELATION " + juce::String(processor.meters.correlation.load(), 2) +
                  "     /     LATENCY " + juce::String(rv::dsp::VocoderEngine::latency) + " samples";
    status.setText(message, juce::dontSendNotification);
    repaint();
}
void RVocoderEditor::paintListBoxItem(int row, juce::Graphics &g, int w, int h, bool selected)
{
    if (row < 0 || row >= static_cast<int>(filtered.size()))
        return;
    const auto &p =
        processor.presets.all()[static_cast<std::size_t>(filtered[static_cast<std::size_t>(row)])];
    if (selected)
    {
        g.setColour(accent.withAlpha(.12f));
        g.fillRect(0, 0, w, h);
        g.setColour(accent);
        g.fillRect(0, 8, 3, h - 16);
    }
    g.setFont(font(14, selected));
    g.setColour(selected ? accent : ink);
    g.drawText(juce::String(p.name), 12, 4, w - 30, 21, juce::Justification::centredLeft);
    g.setFont(font(9));
    g.setColour(muted);
    g.drawText(juce::String(p.category), 12, 25, w - 30, 14, juce::Justification::centredLeft);
    if (processor.presets.favorite(p.id))
    {
        g.setColour(accent);
        g.fillEllipse(static_cast<float>(w - 14), 10, 4, 4);
    }
}
void RVocoderEditor::selectedRowsChanged(int row)
{
    if (!updating && row >= 0 && row < static_cast<int>(filtered.size()))
        processor.loadPreset(
            processor.presets.all()[static_cast<std::size_t>(filtered[static_cast<std::size_t>(row)])]);
}
void RVocoderEditor::filterPresets()
{
    filtered.clear();
    const auto query = search.getText().trim().toLowerCase();
    const auto cat = category.getText();
    const auto &presets = processor.presets.all();
    for (std::size_t i = 0; i < presets.size(); ++i)
    {
        const auto &p = presets[i];
        if (category.getSelectedId() > 1 && cat != juce::String(p.category))
            continue;
        if (favoritesOnly && !processor.presets.favorite(p.id))
            continue;
        if (query.isNotEmpty() &&
            !(juce::String(p.name + " " + p.tags + " " + p.category).toLowerCase().contains(query)))
            continue;
        filtered.push_back(static_cast<int>(i));
    }
    updating = true;
    list.deselectAllRows();
    list.updateContent();
    list.repaint();
    updating = false;
    lastId.clear();
}
void RVocoderEditor::step(int delta)
{
    if (filtered.empty())
        return;
    const int n = static_cast<int>(filtered.size());
    list.selectRow((list.getSelectedRow() + delta + n) % n);
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
                        safe->filterPresets();
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
    juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon, "Borrar preset",
                                       "Se borrará el preset personal «" + processor.presetName() +
                                           "». Los proyectos guardados conservan su sonido.",
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
                                                       safe->filterPresets();
                                                   }
                                               }
                                           }));
}
