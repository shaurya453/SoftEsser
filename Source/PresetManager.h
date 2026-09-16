// SoftEsser - Built by Shaurya 13-05-2026
// Save/load of user presets: each preset is an XML snapshot of the plugin's parameter state,
// stored as its own file under the user's application data folder.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// ====================================================================================================== //

class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& stateToManage);

    // Saves the current parameter state as a preset with the given name (overwrites if it
    // already exists). Returns false if presetName is empty or the file couldn't be written.
    bool savePreset (const juce::String& presetName);

    // Loads a preset previously returned by getAllPresets(). Does nothing if it no longer exists.
    void loadPreset (const juce::String& presetName);

    // Deletes a preset file. Does nothing if it doesn't exist.
    void deletePreset (const juce::String& presetName);

    // All preset names currently on disk, alphabetically sorted.
    juce::StringArray getAllPresets() const;

    // In-memory equivalents of savePreset()/loadPreset(), used by the editor's A/B compare
    // buttons - no file I/O, and doesn't touch currentPresetName.
    juce::MemoryBlock getStateSnapshot() const;
    void restoreStateSnapshot (const juce::MemoryBlock& snapshot);

    // The name last passed to savePreset()/loadPreset(), or empty if none yet (e.g. the current
    // state has been edited since and no longer matches any saved preset).
    const juce::String& getCurrentPresetName() const noexcept { return currentPresetName; }

    static juce::File getPresetDirectory();

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentPresetName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

// ====================================================================================================== //
