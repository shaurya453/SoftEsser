// SoftEsser - Built by Shaurya 13-05-2026
// Declares the plugin editor class, GUI components, and member variables used by the interface.

#pragma once // Header file is included only once during the compilation of the source file

#include "PluginProcessor.h"
#include "PresetManager.h"
#include "SoftEsserLookAndFeel.h"
#include "GainReductionMeter.h"

// ====================================================================================================== //

// One rotary control: the slider, the name label shown above it, and the attachment that keeps
// the slider in sync with its apvts parameter (including updates from host automation).
struct ParameterControl
{
    juce::Slider slider;
    juce::Label nameLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// Main plugin editor class definitions
class SoftEsserAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:

    // Constructor and Destructor
    explicit SoftEsserAudioProcessorEditor (SoftEsserAudioProcessor&);
    ~SoftEsserAudioProcessorEditor() override;

    // GUI drawing and layout functions
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Reference to the audio processor whose parameters these sliders control
    SoftEsserAudioProcessor& processorRef;

    // Save/load of named parameter snapshots (see PresetManager.h)
    PresetManager presetManager;

    // Flat, modern knob styling shared by all sliders (see SoftEsserLookAndFeel.h)
    SoftEsserLookAndFeel lookAndFeel;

    // Shows the tooltip set on each control when the mouse hovers over it
    juce::TooltipWindow tooltipWindow { this };

    juce::Label titleLabel;

    // Plugin controls
    ParameterControl thresholdControl;
    ParameterControl amountControl;
    ParameterControl frequencyControl;
    ParameterControl qControl;
    ParameterControl mixControl;
    ParameterControl outputControl;

    // Solos the detection band to the output (see PluginProcessor::listenParamID)
    juce::TextButton listenButton { "Listen" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> listenAttachment;

    // Live readout of the current gain reduction, polled from processorRef by timerCallback()
    GainReductionMeter gainReductionMeter;

    // Quick A/B compare: two in-memory parameter snapshots, switched between by the buttons
    // below (see PresetManager::getStateSnapshot()/restoreStateSnapshot() - separate from the
    // named preset system, and never touches disk).
    juce::TextButton abButtonA { "A" };
    juce::TextButton abButtonB { "B" };
    juce::MemoryBlock stateSnapshotA, stateSnapshotB;
    bool currentlyOnSlotA = true;
    void switchAbSlot (bool switchToA);

    // Preset selection/creation
    juce::ComboBox presetBox;
    juce::TextButton savePresetButton { "Save As..." };
    std::unique_ptr<juce::AlertWindow> presetNameWindow; // owns the "name this preset" dialog while it's open

    // Background image, loaded from BinaryData in the constructor
    juce::Image backgroundImage;

    // Configures one rotary control: attaches it to its apvts parameter, sets its unit suffix,
    // hover tooltip, and its name label. Decimal-place formatting lives on the parameter itself
    // (see PluginProcessor.cpp's withOneDecimalPlace()), not here - the slider's own displayed
    // text comes straight from the parameter's getText() once it's attached.
    void setupControl (ParameterControl& control, const juce::String& parameterID, const juce::String& displayName,
                        const juce::String& tooltip, const juce::String& suffix);

    // Repopulates presetBox from PresetManager's current preset list, selecting the active one
    void refreshPresetBox();

    // Opens a text-entry dialog asking for a preset name, then saves under that name
    void showSavePresetDialog();

    // Polls processorRef's current gain reduction and pushes it into gainReductionMeter
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftEsserAudioProcessorEditor)
};

// ====================================================================================================== //
