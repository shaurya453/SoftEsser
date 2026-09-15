// SoftEsser - Built by Shaurya 13-05-2026
// This file handles the graphical user interface (GUI) layout, knob styling, labels, and parameter attachments for the plugin window.

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h" // Added to access the packaged assets

// ====================================================================================================== //

namespace
{
    // Window size
    constexpr int windowWidth = 500;
    constexpr int windowHeight = 250;

    // Shared layout for the five rotary knobs and their labels, kept in one place so
    // paint() (labels) and resized() (knobs) can't drift out of sync with each other.
    constexpr int knobY = 70;
    constexpr int knobSize = 80;
    constexpr int labelY = 180;
    constexpr int labelHeight = 20;
    constexpr int labelWidth = 80;

    constexpr int titleY = 20;
    constexpr int titleHeight = 40;
}

// ====================================================================================================== //

// Constructor for the plugin editor.
SoftEsserAudioProcessorEditor::SoftEsserAudioProcessorEditor (SoftEsserAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Load background image from binary data
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::bg_png, BinaryData::bg_pngSize);

    // Initializing sliders
    setupSlider (thresholdSlider);
    thresholdSlider.setRange (-60.0f, 0.0f);
    thresholdSlider.setValue (-20.0f);
    thresholdSlider.setNumDecimalPlacesToDisplay (2);
    thresholdSlider.setDoubleClickReturnValue (true, -20.0);
    thresholdSlider.setTextValueSuffix (" dB");

    setupSlider (amountSlider);
    amountSlider.setRange (0.0f, 100.0f);
    amountSlider.setValue (50.0f);
    amountSlider.setNumDecimalPlacesToDisplay (2);
    amountSlider.setDoubleClickReturnValue (true, 50.0);
    amountSlider.setTextValueSuffix (" %");

    setupSlider (frequencySlider);
    frequencySlider.setRange (4000.0f, 10000.0f);
    frequencySlider.setValue (7000.0f);
    frequencySlider.setNumDecimalPlacesToDisplay (2);
    frequencySlider.setDoubleClickReturnValue (true, 7000.0);
    frequencySlider.setTextValueSuffix (" Hz");

    setupSlider (mixSlider);
    mixSlider.setRange (0.0f, 100.0f);
    mixSlider.setValue (100.0f);
    mixSlider.setNumDecimalPlacesToDisplay (2);
    mixSlider.setDoubleClickReturnValue (true, 100.0);
    mixSlider.setTextValueSuffix (" %");

    setupSlider (outputSlider);
    outputSlider.setRange (-12.0f, 12.0f);
    outputSlider.setValue (0.0f);
    outputSlider.setNumDecimalPlacesToDisplay (2);
    outputSlider.setDoubleClickReturnValue (true, 0.0);
    outputSlider.setTextValueSuffix (" dB");

    // Setting size of the window
    setSize (windowWidth, windowHeight);
}

// ====================================================================================================== //

// Destructor for the editor
SoftEsserAudioProcessorEditor::~SoftEsserAudioProcessorEditor()
{
}

// ====================================================================================================== //

// Helper function to configure a slider's shared properties (style, text box, visibility, listener)
void SoftEsserAudioProcessorEditor::setupSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);

    addAndMakeVisible (slider);

    slider.addListener (this);
}

// ====================================================================================================== //

// Drawing the UI background, title text, and knob labels
void SoftEsserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw the background image if it successfully loaded, otherwise fall back to a solid color
    if (backgroundImage.isValid())
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll (juce::Colour (15, 20, 35));

    g.setColour (juce::Colours::linen);

    g.setFont (28.0f);
    g.drawFittedText ("/ SoftEsser V1.1", 0, titleY, getWidth(), titleHeight,
                       juce::Justification::centred, 1);

    g.setFont (14.0f);

    g.drawText ("Threshold", 35, labelY, labelWidth, labelHeight, juce::Justification::centred);
    g.drawText ("Amount", 125, labelY, labelWidth, labelHeight, juce::Justification::centred);
    g.drawText ("Frequency", 215, labelY, labelWidth, labelHeight, juce::Justification::centred);
    g.drawText ("Mix", 305, labelY, labelWidth, labelHeight, juce::Justification::centred);
    g.drawText ("Output", 395, labelY, labelWidth, labelHeight, juce::Justification::centred);
}

// ====================================================================================================== //

// Slider position and size within the editor window
void SoftEsserAudioProcessorEditor::resized()
{
    thresholdSlider.setBounds (30, knobY, knobSize, knobSize);
    amountSlider.setBounds   (120, knobY, knobSize, knobSize);
    frequencySlider.setBounds (210, knobY, knobSize, knobSize);
    mixSlider.setBounds      (300, knobY, knobSize, knobSize);
    outputSlider.setBounds   (390, knobY, knobSize, knobSize);
}

// ====================================================================================================== //

// Called whenever a slider value changes and updates the processor parameter values in real time
void SoftEsserAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &thresholdSlider)
        processorRef.threshold = (float) thresholdSlider.getValue();

    else if (slider == &amountSlider)
        processorRef.amount = (float) amountSlider.getValue();

    else if (slider == &frequencySlider)
        processorRef.frequency = (float) frequencySlider.getValue();

    else if (slider == &mixSlider)
        processorRef.mix = (float) mixSlider.getValue();

    else if (slider == &outputSlider)
        processorRef.outputGain = (float) outputSlider.getValue();
}

// ====================================================================================================== //
