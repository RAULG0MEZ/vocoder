#pragma once
#include "MidiKeyboard.h"
#include "Plugin/PluginProcessor.h"
#include "SoundCanvas.h"
class RVLookAndFeel final : public juce::LookAndFeel_V4
{
  public:
    RVLookAndFeel();
    void drawRotarySlider(juce::Graphics &, int, int, int, int, float, float, float, juce::Slider &) override;
    void drawButtonBackground(juce::Graphics &, juce::Button &, const juce::Colour &, bool, bool) override;
};
class ParameterPanel final : public juce::Component
{
  public:
    explicit ParameterPanel(RVocoderProcessor &, bool macros = false);
    void showPage(int);
    void layout(int width, int availableHeight = 344);
    void refreshChoices();
    void paint(juce::Graphics &) override;

  private:
    struct Control
    {
        rv::P id;
        juce::Label label;
        juce::Slider slider;
        juce::ComboBox choice;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        bool combo = false;
    };
    RVocoderProcessor &processor;
    std::vector<std::unique_ptr<Control>> controls;
    struct Heading
    {
        juce::String text;
        int y;
    };
    std::vector<Heading> headings;
    int page = 0;
    bool macrosOnly = false;
};
class RVocoderEditor final : public juce::AudioProcessorEditor,
                             private juce::Timer,
                             private juce::ListBoxModel
{
  public:
    explicit RVocoderEditor(RVocoderProcessor &);
    ~RVocoderEditor() override;
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    void timerCallback() override;
    int getNumRows() override
    {
        return static_cast<int>(filtered.size());
    }
    void paintListBoxItem(int, juce::Graphics &, int, int, bool) override;
    void selectedRowsChanged(int) override;
    void filterPresets();
    void step(int);
    void savePreset();
    void deletePreset();
    void setPage(int);
    RVocoderProcessor &processor;
    RVLookAndFeel look;
    juce::TooltipWindow tips{this, 650};
    juce::Label brand, presetTitle, subtitle, status, inputLabel, soundLabel, midiStatus, playLabel,
        keyboardLabel, canvasTitle, inspectorTitle;
    juce::TextEditor search;
    juce::ComboBox category;
    std::array<juce::TextButton, 2> inputButtons;
    std::array<juce::TextButton, 3> soundButtons;
    std::array<juce::TextButton, 2> playButtons;
    juce::TextButton xyButton{"XY"}, spectrumButton{"Espectro"}, panicButton{"Silenciar"};
    juce::ToggleButton keyGateToggle{"Cortar al soltar"}, wetOnlyToggle{"Solo efecto"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> keyGateAttachment,
        wetOnlyAttachment;
    juce::ListBox list;
    juce::TextButton prev{"<"}, next{">"}, random{"Shuffle"}, favorite{"Favorite"},
        onlyFavorites{"Favorites"}, save{"Save"}, remove{"Delete"}, reset{"Reset"};
    std::array<juce::TextButton, 5> tabs;
    juce::ToggleButton bypass{"Bypass"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    juce::Viewport viewport;
    ParameterPanel panel;
    ParameterPanel macros;
    SoundCanvas canvas;
    RVKeyboard keyboard;
    std::vector<int> filtered;
    int currentPage = 0;
    bool favoritesOnly = false, updating = false;
    juce::String lastId;
    std::array<float, 4> displayMeters{};
    unsigned lastMidiSequence = 0;
    int midiFlashTicks = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVocoderEditor)
};
