#include "PresetManager.h"
namespace rv
{
PresetManager::PresetManager(juce::File directory)
    : folder(directory == juce::File{}
                 ? juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("RSTK/R-Vocoder")
                 : directory),
      presets(factoryPresets())
{
    favorites.addTokens(folder.getChildFile("favorites.txt").loadFileAsString(), "\n", "");
    reloadUsers();
}
void PresetManager::reloadUsers()
{
    presets.erase(std::remove_if(presets.begin(), presets.end(), [](const auto &p) { return p.user; }),
                  presets.end());
    for (const auto &f : folder.findChildFiles(juce::File::findFiles, false, "*.rvpreset"))
    {
        const auto json = juce::JSON::parse(f);
        auto *obj = json.getDynamicObject();
        if (!obj || static_cast<int>(obj->getProperty("version")) != 1)
            continue;
        auto *vals = obj->getProperty("parameters").getDynamicObject();
        if (!vals)
            continue;
        Preset p;
        p.id = f.getFileNameWithoutExtension().toStdString();
        p.name = obj->getProperty("name").toString().toStdString();
        p.category = "USER";
        p.tags = "user";
        p.user = true;
        for (std::size_t i = 0; i < parameterCount; ++i)
            if (vals->hasProperty(definitions[i].id))
                p.parameters.values[i] = static_cast<float>(vals->getProperty(definitions[i].id));
        p.parameters.sanitize();
        if (!p.name.empty())
            presets.push_back(std::move(p));
    }
}
juce::Result PresetManager::save(const juce::String &name, const Params &parameters, std::string *savedId)
{
    if (name.trim().isEmpty())
        return juce::Result::fail("Escribe un nombre para el preset.");
    auto created = folder.createDirectory();
    if (created.failed())
        return created;
    auto root = new juce::DynamicObject;
    root->setProperty("version", 1);
    root->setProperty("name", name.trim().substring(0, 80));
    auto values = new juce::DynamicObject;
    for (std::size_t i = 0; i < parameterCount; ++i)
        values->setProperty(definitions[i].id, parameters.values[i]);
    root->setProperty("parameters", juce::var(values));
    const auto file = folder.getChildFile("user-" + juce::Uuid().toString() + ".rvpreset");
    if (!file.replaceWithText(juce::JSON::toString(juce::var(root))))
        return juce::Result::fail("No se pudo guardar el preset.");
    if (savedId)
        *savedId = file.getFileNameWithoutExtension().toStdString();
    reloadUsers();
    return juce::Result::ok();
}
juce::Result PresetManager::remove(const std::string &id)
{
    const auto p = std::find_if(presets.begin(), presets.end(),
                                [&](const auto &item) { return item.user && item.id == id; });
    if (p == presets.end())
        return juce::Result::fail("Los presets de fábrica no se pueden borrar.");
    if (!folder.getChildFile(juce::String(id) + ".rvpreset").deleteFile())
        return juce::Result::fail("No se pudo borrar el preset.");
    reloadUsers();
    return juce::Result::ok();
}
bool PresetManager::favorite(const std::string &id) const
{
    return favorites.contains(juce::String(id));
}
juce::Result PresetManager::toggleFavorite(const std::string &id)
{
    if (favorite(id))
        favorites.removeString(juce::String(id));
    else
        favorites.add(juce::String(id));
    auto result = folder.createDirectory();
    if (result.failed())
        return result;
    return folder.getChildFile("favorites.txt").replaceWithText(favorites.joinIntoString("\n"))
               ? juce::Result::ok()
               : juce::Result::fail("No se pudieron guardar favoritos.");
}
} // namespace rv
