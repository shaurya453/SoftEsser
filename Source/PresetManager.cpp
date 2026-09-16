// SoftEsser - Built by Shaurya 13-05-2026
// Save/load of user presets: each preset is an XML snapshot of the plugin's parameter state.

#include "PresetManager.h"
#include "PluginProcessor.h"

// ====================================================================================================== //

namespace
{
    const juce::String presetFileExtension = ".xml";
    const juce::String defaultPresetName = "Default";

    // Values for each of the plugin's user parameters, used to seed a set of starting-point
    // presets on first run (in addition to "Default", which is just the plugin's own defaults).
    struct FactoryPreset
    {
        const char* name;
        float threshold, amount, frequency, q, mix, outputGain;
    };

    const FactoryPreset factoryPresets[] = {
        { "Vocal - Light",      -18.0f, 35.0f, 6500.0f, 2.0f, 100.0f, 0.0f },
        { "Vocal - Aggressive", -24.0f, 70.0f, 7000.0f, 3.0f, 100.0f, 0.0f },
        { "Broadcast",          -20.0f, 50.0f, 6000.0f, 1.5f, 100.0f, 0.0f },
    };

    // Writes a factory preset straight to XML, bypassing apvts entirely - the presets are seeded
    // before the editor exists, and going through apvts's own parameters would mean audibly
    // changing them on the live processor just to read them straight back out again.
    bool writeFactoryPresetFile (const juce::AudioProcessorValueTreeState& apvts,
                                  const FactoryPreset& preset, const juce::File& file)
    {
        juce::XmlElement root (apvts.state.getType());

        auto addParam = [&root] (const char* id, float value)
        {
            auto* param = new juce::XmlElement ("PARAM");
            param->setAttribute ("id", id);
            param->setAttribute ("value", value);
            root.addChildElement (param);
        };

        addParam (SoftEsserAudioProcessor::thresholdParamID,  preset.threshold);
        addParam (SoftEsserAudioProcessor::amountParamID,     preset.amount);
        addParam (SoftEsserAudioProcessor::frequencyParamID,  preset.frequency);
        addParam (SoftEsserAudioProcessor::qParamID,          preset.q);
        addParam (SoftEsserAudioProcessor::mixParamID,        preset.mix);
        addParam (SoftEsserAudioProcessor::outputGainParamID, preset.outputGain);
        addParam (SoftEsserAudioProcessor::listenParamID,     0.0f);

        return root.writeTo (file);
    }
}

// ====================================================================================================== //

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToManage) : apvts (stateToManage)
{
    auto directory = getPresetDirectory();

    if (! directory.isDirectory())
        directory.createDirectory();

    // Each factory preset (Default included) is seeded individually if its own file is missing,
    // rather than only when the whole folder is empty - an install that already had "Default"
    // from before this feature existed would otherwise never get the newer factory presets.
    if (! directory.getChildFile (defaultPresetName + presetFileExtension).existsAsFile())
        savePreset (defaultPresetName);

    for (const auto& preset : factoryPresets)
    {
        auto file = directory.getChildFile (juce::String (preset.name) + presetFileExtension);

        if (! file.existsAsFile())
            writeFactoryPresetFile (apvts, preset, file);
    }
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

juce::MemoryBlock PresetManager::getStateSnapshot() const
{
    juce::MemoryBlock block;

    if (auto xml = apvts.copyState().createXml())
        juce::AudioProcessor::copyXmlToBinary (*xml, block);

    return block;
}

// ====================================================================================================== //

void PresetManager::restoreStateSnapshot (const juce::MemoryBlock& snapshot)
{
    if (snapshot.getSize() == 0)
        return;

    if (auto xml = juce::AudioProcessor::getXmlFromBinary (snapshot.getData(), (int) snapshot.getSize()))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ====================================================================================================== //
