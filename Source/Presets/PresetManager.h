#pragma once
#include "FactoryPresets.h"
#include <juce_data_structures/juce_data_structures.h>
namespace rv
{
class PresetManager
{
  public:
    explicit PresetManager(juce::File directory = {});
    const std::vector<Preset> &all() const
    {
        return presets;
    }
    void reloadUsers();
    juce::Result save(const juce::String &name, const Params &parameters, std::string *savedId = nullptr);
    juce::Result remove(const std::string &id);
    bool favorite(const std::string &id) const;
    juce::Result toggleFavorite(const std::string &id);
    juce::File directory() const
    {
        return folder;
    }

  private:
    juce::File folder;
    std::vector<Preset> presets;
    juce::StringArray favorites;
};
} // namespace rv
