// SoftEsser - Built by Shaurya 13-05-2026
// Declares the plugin editor class, GUI components, and member variables used by the interface.

#pragma once // Header file is included only once during the compilation of the source file

#include "PluginProcessor.h"

// ====================================================================================================== //

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

    // Plugin sliders
    juce::Slider thresholdSlider;
    juce::Slider amountSlider;
    juce::Slider frequencySlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;

    // Background image, loaded from BinaryData in the constructor
    juce::Image backgroundImage;

    // Called when any slider changes value
    void sliderValueChanged (juce::Slider* slider) override;

    // Helper function for slider setup
    void setupSlider (juce::Slider& slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftEsserAudioProcessorEditor)
};

// ====================================================================================================== //
