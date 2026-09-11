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
ParameterPanel::ParameterPanel(RVocoderProcessor &p, bool macros) : processor(p), macrosOnly(macros)
{
    for (std::size_t i = 0; i < parameterCount; ++i)
    {
        const auto &d = definitions[i];
        if ((juce::String(d.group) == "Main") != macrosOnly || i == index(P::route) ||
            i == index(P::bypass) || i == index(P::voiceMode) || i == index(P::synthMode) ||
            i == index(P::midiGate) || i == index(P::wetOnly))
            continue;
        auto c = std::make_unique<Control>();
        c->id = static_cast<P>(i);
        c->slider.setName(d.name);
        c->slider.setComponentID(d.id);
        c->choice.setComponentID(d.id);
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
            c->slider.setTooltip(juce::String(d.name) +
                                 juce::String::fromUTF8(" · Doble clic para restablecer"));
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
    const int cols = macrosOnly ? 8 : (width >= 390 ? 3 : 2);
    const int cell = std::max(1, (width - 16) / cols);
    const int rowHeight = macrosOnly ? availableHeight - 6 : 112;
    int y = 6, col = 0;
    juce::String last;
    for (auto &c : controls)
    {
        const auto group = juce::String(definitions[index(c->id)].group);
        const bool visible =
            macrosOnly ||
            (page == 0 && (group == "Analysis" || group == "Intelligibility" || group == "Identity")) ||
            (page == 1 && group == "Synth") ||
            (page == 2 && (group == "Stereo" || group == "Character" || group == "Tone")) ||
            (page == 3 && group == "Motion") ||
            (page == 4 &&
             (group == "Gate" || group == "Blend" || group == "Routing" || group == "Performance"));
        c->label.setVisible(visible);
        c->slider.setVisible(visible && !c->combo);
        c->choice.setVisible(visible && c->combo);
        if (!visible)
            continue;
        if (!macrosOnly && last != group)
        {
            if (col)
            {
                y += rowHeight;
                col = 0;
            }
            juce::String title = group.toUpperCase();
            if (group == "Gate")
                title = "PUERTA POR NIVEL DE VOZ";
            if (group == "Performance")
                title = "CIERRE AL SOLTAR LAS TECLAS";
            if (group == "Blend")
                title = "MEZCLA DE FUENTES";
            if (group == "Analysis")
                title = juce::String::fromUTF8("ANÁLISIS Y ENVOLVENTE");
            headings.push_back({title, y});
            y += 30;
            last = group;
        }
        const int x = 8 + col * cell;
        c->label.setBounds(x, y, cell, 22);
        if (c->combo)
            c->choice.setBounds(x + 4, y + 44, cell - 8, 30);
        else
            c->slider.setBounds(x + 2, y + 22, cell - 4, rowHeight - 24);
        if (++col == cols)
        {
            col = 0;
            y += rowHeight;
        }
    }
    if (col)
        y += rowHeight;
    setSize(width, macrosOnly ? availableHeight : y + 12);
    refreshChoices();
    repaint();
}
void ParameterPanel::refreshChoices()
{
    const auto values = processor.readParameters();
    for (auto &c : controls)
    {
        if (c->combo)
            c->choice.setSelectedId(static_cast<int>(values[c->id] - definitions[index(c->id)].min) + 1,
                                    juce::dontSendNotification);
        const auto group = juce::String(definitions[index(c->id)].group);
        const bool originalVoice = values[P::voiceMode] > .5f;
        const bool wetOnly = values[P::wetOnly] > .5f;
        const bool internal = Routing::from(values).sound == SoundSource::synth;
        const bool enabled =
            !(group == "Synth" && !internal) &&
            !(originalVoice && (c->id == P::modMix || c->id == P::carMix || c->id == P::presetLevel)) &&
            !(wetOnly && (c->id == P::mix || c->id == P::modMix)) &&
            !((c->id == P::rootNote || c->id == P::chord) && values[P::synthMode] > .5f);
        if (c->id == P::mix)
        {
            c->slider.setValue(wetOnly ? 1.f : values[P::mix], juce::dontSendNotification);
            c->slider.setTooltip("Original / efecto. Desactiva Solo efecto para mezclar la voz limpia.");
        }
        c->slider.setEnabled(enabled);
        c->choice.setEnabled(enabled);
        c->label.setAlpha(enabled ? 1.f : .4f);
    }
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
    : AudioProcessorEditor(p), processor(p), panel(p), macros(p, true), canvas(p),
      keyboard(p.keyboard, p.midiMonitor)
{
    setLookAndFeel(&look);
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(1080, 760, 1800, 1200);
    brand.setText("R / VOCODER", juce::dontSendNotification);
    brand.setFont(font(22, true));
    addAndMakeVisible(brand);
    presetTitle.setFont(font(28, true));
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
    list.setRowHeight(46);
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
    const std::array<juce::String, 5> names{"VOZ", "SYNTH", "FX", "MOTION", "SALIDA"};
    for (int i = 0; i < 5; ++i)
    {
        tabs[static_cast<std::size_t>(i)].setButtonText(names[static_cast<std::size_t>(i)]);
        tabs[static_cast<std::size_t>(i)].setComponentID("detail-" + juce::String(i));
        tabs[static_cast<std::size_t>(i)].onClick = [this, i] { setPage(i); };
        addAndMakeVisible(tabs[static_cast<std::size_t>(i)]);
    }
    playLabel.setText(juce::String::fromUTF8("ACTIVACIÓN"), juce::dontSendNotification);
    keyboardLabel.setText("TECLADO MIDI", juce::dontSendNotification);
    canvasTitle.setText("FORMA DE LA VOZ", juce::dontSendNotification);
    inspectorTitle.setText("AJUSTES", juce::dontSendNotification);
    for (auto *label : {&playLabel, &keyboardLabel, &canvasTitle, &inspectorTitle})
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
                             "cola se ajusta en SALIDA.");
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
    addAndMakeVisible(macros);
    addAndMakeVisible(canvas);
    addAndMakeVisible(bypass);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "bypass", bypass);
    viewport.setViewedComponent(&panel, false);
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(8);
    addAndMakeVisible(viewport);
    keyboard.setAvailableRange(0, 127);
    keyboard.setLowestVisibleKey(33);
    keyboard.setOctaveForMiddleC(4);
    keyboard.setKeyWidth(22);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffaeb7b3));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, bg);
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, accent);
    keyboard.setWantsKeyboardFocus(false);
    addAndMakeVisible(keyboard);
    setSize(1320, 840);
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
    const int w = getWidth(), h = getHeight(), left = 204, x = left + 22, width = w - x - 22;
    brand.setBounds(18, 18, 190, 35);
    presetTitle.setBounds(x, 15, width - 310, 35);
    subtitle.setBounds(x, 50, width - 280, 22);
    prev.setBounds(w - 310, 21, 32, 30);
    next.setBounds(w - 272, 21, 32, 30);
    random.setBounds(w - 234, 21, 82, 30);
    reset.setBounds(w - 145, 21, 60, 30);
    bypass.setBounds(w - 90, 52, 84, 28);
    category.setBounds(16, 102, 172, 30);
    search.setBounds(16, 141, 172, 32);
    list.setBounds(10, 185, 184, h - 335);
    onlyFavorites.setBounds(16, h - 136, 82, 30);
    favorite.setBounds(104, h - 136, 84, 30);
    save.setBounds(16, h - 97, 82, 30);
    remove.setBounds(104, h - 97, 84, 30);
    const int inputW = width * 24 / 100, soundW = width * 31 / 100;
    const int soundX = x + inputW + 16, playX = soundX + soundW + 16;
    inputLabel.setBounds(x, 91, inputW, 18);
    soundLabel.setBounds(soundX, 91, soundW, 18);
    playLabel.setBounds(playX, 91, w - playX - 22, 18);
    for (int i = 0; i < 2; ++i)
        inputButtons[static_cast<std::size_t>(i)].setBounds(x + i * (inputW / 2), 114, inputW / 2 - 3, 32);
    for (int i = 0; i < 3; ++i)
        soundButtons[static_cast<std::size_t>(i)].setBounds(soundX + i * (soundW / 3), 114, soundW / 3 - 3,
                                                            32);
    const int playW = std::min(120, (w - playX - 22) / 2);
    for (int i = 0; i < 2; ++i)
        playButtons[static_cast<std::size_t>(i)].setBounds(playX + i * playW, 114, playW - 3, 32);
    keyboardLabel.setBounds(x, 164, 116, 24);
    keyGateToggle.setBounds(x + 118, 162, 146, 28);
    wetOnlyToggle.setBounds(x + 272, 162, 122, 28);
    midiStatus.setBounds(w - 266, 164, 136, 24);
    panicButton.setBounds(w - 117, 162, 95, 28);
    keyboard.setBounds(x, 198, width, 62);
    keyboard.setVisible(true);
    macros.setBounds(x - 8, 276, width + 16, 136);
    macros.layout(width + 16, 136);
    const int detailW = std::clamp(width * 41 / 100, 338, 480), detailX = w - 22 - detailW;
    canvasTitle.setBounds(x, 431, detailX - x - 200, 26);
    xyButton.setBounds(detailX - 187, 429, 52, 28);
    spectrumButton.setBounds(detailX - 128, 429, 103, 28);
    canvas.setBounds(x, 469, detailX - x - 24, h - 589);
    inspectorTitle.setBounds(detailX, 431, 80, 26);
    for (int i = 0; i < 5; ++i)
        tabs[static_cast<std::size_t>(i)].setBounds(detailX + i * (detailW / 5), 464, detailW / 5 - 3, 28);
    viewport.setBounds(detailX, 500, detailW, h - 620);
    panel.layout(viewport.getWidth() - 10, viewport.getHeight());
    status.setBounds(x, h - 108, width, 26);
    repaint();
}
void RVocoderEditor::paint(juce::Graphics &g)
{
    const int w = getWidth(), h = getHeight(), left = 204, x = left + 22, width = w - x - 22;
    g.fillAll(bg);
    g.setColour(sidebar);
    g.fillRect(0, 0, left, h);
    g.setColour(line);
    g.drawVerticalLine(left, 0, static_cast<float>(h));
    g.drawHorizontalLine(79, 0, static_cast<float>(w));
    g.drawHorizontalLine(421, static_cast<float>(x), static_cast<float>(w - 22));
    g.drawHorizontalLine(h - 77, static_cast<float>(x), static_cast<float>(w - 22));
    g.setColour(muted);
    g.setFont(font(11, true));
    g.drawText("BIBLIOTECA / 100 PRESETS", 18, 78, 180, 20, juce::Justification::centredLeft);
    g.drawText("RSTK   /   " JucePlugin_VersionString, 18, h - 38, 180, 20, juce::Justification::centredLeft);
    const std::array<juce::String, 4> labels{processor.midiEdition ? "SIDECHAIN" : "PISTA", "VOZ", "SONIDO",
                                             "SALIDA"};
    const int cell = width / 4;
    for (int i = 0; i < 4; ++i)
    {
        const int mx = x + i * cell;
        const auto db = juce::Decibels::gainToDecibels(displayMeters[static_cast<std::size_t>(i)], -72.f);
        g.setFont(font(10, true));
        g.setColour(muted);
        g.drawText(labels[static_cast<std::size_t>(i)], mx, h - 69, cell - 14, 20, juce::Justification::left);
        g.setColour(line);
        g.fillRect(mx, h - 42, cell - 14, 5);
        g.setColour(db > -.5f ? juce::Colour(0xfff39d86) : accent);
        g.fillRect(mx, h - 42, static_cast<int>(std::clamp((db + 60) / 60, 0.f, 1.f) * (cell - 14)), 5);
        g.setFont(font(10));
        g.setColour(muted);
        g.drawText(db <= -72 ? "-inf dB" : juce::String(db, 1) + " dB", mx, h - 31, cell - 14, 20,
                   juce::Justification::left);
    }
}
void RVocoderEditor::timerCallback()
{
    keyboard.refreshIncomingNotes();
    panel.refreshChoices();
    macros.refreshChoices();
    canvas.refresh();
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
    presetTitle.setText(processor.presetName(), juce::dontSendNotification);
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
                        safe->filterPresets();
                    }
                }
            }));
}
