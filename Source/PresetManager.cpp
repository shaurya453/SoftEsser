// SoftEsser - Built by Shaurya 13-05-2026
// Save/load of user presets: each preset is an XML snapshot of the plugin's parameter state.

#include "PresetManager.h"

// ====================================================================================================== //

namespace
{
    const juce::String presetFileExtension = ".xml";
    const juce::String defaultPresetName = "Default";
}

// ====================================================================================================== //

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToManage) : apvts (stateToManage)
{
    auto directory = getPresetDirectory();

    if (! directory.isDirectory())
        directory.createDirectory();

    // On first run there are no presets on disk yet - save the plugin's current (default)
    // state as a starting point, so the preset list is never empty.
    if (getAllPresets().isEmpty())
        savePreset (defaultPresetName);
}

// ====================================================================================================== //

juce::File PresetManager::getPresetDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("SoftEsser")
        .getChildFile ("Presets");
}

// ====================================================================================================== //

bool PresetManager::savePreset (const juce::String& presetName)
{
    if (presetName.isEmpty())
        return false;

    auto file = getPresetDirectory().getChildFile (presetName + presetFileExtension);

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());

    if (xml == nullptr || ! xml->writeTo (file))
        return false;

    currentPresetName = presetName;
    return true;
}

// ====================================================================================================== //

void PresetManager::loadPreset (const juce::String& presetName)
{
    auto file = getPresetDirectory().getChildFile (presetName + presetFileExtension);

    if (! file.existsAsFile())
        return;

    if (auto xml = juce::XmlDocument::parse (file))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));

    currentPresetName = presetName;
}

// ====================================================================================================== //

void PresetManager::deletePreset (const juce::String& presetName)
{
    getPresetDirectory().getChildFile (presetName + presetFileExtension).deleteFile();

    if (currentPresetName == presetName)
        currentPresetName.clear();
}

// ====================================================================================================== //

juce::StringArray PresetManager::getAllPresets() const
{
    juce::StringArray presets;

    for (const auto& file : getPresetDirectory().findChildFiles (juce::File::findFiles, false, "*" + presetFileExtension))
        presets.add (file.getFileNameWithoutExtension());

    presets.sort (true);
    return presets;
}

// ====================================================================================================== //
