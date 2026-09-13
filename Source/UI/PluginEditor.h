#pragma once
#include "HardwareSkin.h"
#include "MidiKeyboard.h"
#include "Plugin/PluginProcessor.h"
#include "SoundCanvas.h"
#include <map>

class RVLookAndFeel final : public juce::LookAndFeel_V4
{
  public:
    RVLookAndFeel();
    juce::Font getComboBoxFont(juce::ComboBox &) override;
    void drawRotarySlider(juce::Graphics &, int, int, int, int, float, float, float, juce::Slider &) override;
    void drawButtonBackground(juce::Graphics &, juce::Button &, const juce::Colour &, bool, bool) override;
    void drawComboBox(juce::Graphics &, int, int, bool, int, int, int, int, juce::ComboBox &) override;
    void drawToggleButton(juce::Graphics &, juce::ToggleButton &, bool, bool) override;
    static void surface(juce::Graphics &, juce::Rectangle<float>, bool recessed = false);
    static void meter(juce::Graphics &, juce::Point<float>, float, float, float, float, float);

  private:
    juce::Image knobFace(int size);
    std::map<int, juce::Image> faces;
};

class ParameterControl final : public juce::Component
{
  public:
    ParameterControl(RVocoderProcessor &, rv::P, juce::Colour);
    void resized() override;
    void refresh();
    void setMeter(float level, float peak);
    rv::P parameterId() const
    {
        return id;
    }

  private:
    RVocoderProcessor &processor;
    rv::P id;
    juce::Label label;
    juce::Slider slider;
    juce::ComboBox choice;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    bool combo;
};

class ParameterRack final : public juce::Component
{
  public:
    struct Page
    {
        juce::String name;
        std::vector<rv::P> parameters;
    };
    ParameterRack(RVocoderProcessor &, juce::String name, juce::Colour, std::vector<Page>);
    void resized() override;
    void paint(juce::Graphics &) override;
    void refresh();
    void selectPage(int);
    int pageCount() const
    {
        return static_cast<int>(pages.size());
    }
    std::vector<rv::P> parameterIds() const;

  private:
    juce::String title;
    juce::Colour colour;
    juce::ComboBox selector;
    std::vector<Page> pages;
    std::vector<std::unique_ptr<ParameterControl>> controls;
    int selected = 0;
};

class GainStage final : public juce::Component
{
  public:
    explicit GainStage(RVocoderProcessor &);
    void resized() override;
    void paint(juce::Graphics &) override;
    void refresh();

  private:
    RVocoderProcessor &processor;
    ParameterControl input, output;
    std::array<float, 4> level{}, peak{};
    std::array<int, 4> hold{};
};

class RVocoderEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
  public:
    explicit RVocoderEditor(RVocoderProcessor &);
    ~RVocoderEditor() override;
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    void timerCallback() override;
    void rebuildPresets();
    void presetMenu();
    void step(int);
    void savePreset();
    void deletePreset();
    RVocoderProcessor &processor;
    RVLookAndFeel look;
    hardware::Chassis chassis;
    juce::TooltipWindow tips{this, 650};
    juce::Label brand, subtitle, status, inputLabel, soundLabel, midiStatus, playLabel, keyboardLabel,
        canvasTitle, correlation;
    juce::ComboBox presetSelector;
    juce::TextButton prev{"<"}, next{">"}, favorite{juce::String::fromUTF8("☆")}, menu{"..."};
    std::array<juce::TextButton, 2> inputButtons, playButtons;
    std::array<juce::TextButton, 3> soundButtons;
    juce::TextButton xyButton{"XY"}, spectrumButton{"Espectro"}, panicButton{"Silenciar"};
    juce::ToggleButton keyGateToggle{"Cortar al soltar"}, wetOnlyToggle{"Solo efecto"}, bypass{"Bypass"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> keyGateAttachment,
        wetOnlyAttachment, bypassAttachment;
    std::vector<std::unique_ptr<ParameterControl>> macros;
    std::array<std::unique_ptr<ParameterRack>, 4> racks;
    GainStage gains;
    SoundCanvas canvas;
    RVKeyboard keyboard;
    std::vector<int> filtered;
    bool favoritesOnly = false;
    juce::String lastId;
    unsigned lastMidiSequence = 0;
    int midiFlashTicks = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVocoderEditor)
};
