// SoftEsser - Built by Shaurya 13-05-2026
// Declares the plugin editor class, GUI components, and member variables used by the interface.

#pragma once // Header file is included only once during the compilation of the source file

#include "PluginProcessor.h"
#include "SoftEsserLookAndFeel.h"

// ====================================================================================================== //

// One rotary control: the slider itself plus the name label shown above it. Grouped together
// so the two stay in sync (both get the same tooltip, both get positioned as a unit).
struct ParameterControl
{
    juce::Slider slider;
    juce::Label nameLabel;
};

// Main plugin editor class definitions
class SoftEsserAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Slider::Listener
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

    // Flat, modern knob styling shared by all sliders (see SoftEsserLookAndFeel.h)
    SoftEsserLookAndFeel lookAndFeel;

    // Shows the tooltip set on each control when the mouse hovers over it
    juce::TooltipWindow tooltipWindow { this };

    juce::Label titleLabel;

    // Plugin controls
    ParameterControl thresholdControl;
    ParameterControl amountControl;
    ParameterControl frequencyControl;
    ParameterControl mixControl;
    ParameterControl outputControl;

    // Background image, loaded from BinaryData in the constructor
    juce::Image backgroundImage;

    // Called when any slider changes value
    void sliderValueChanged (juce::Slider* slider) override;

    // Configures one rotary control: range, default value, decimal places, unit suffix,
    // hover tooltip, and its name label.
    void setupControl (ParameterControl& control, const juce::String& name, const juce::String& tooltip,
                        float minValue, float maxValue, float defaultValue, int decimalPlaces,
                        const juce::String& suffix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftEsserAudioProcessorEditor)
};

// ====================================================================================================== //
