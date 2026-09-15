// SoftEsser - Built by Shaurya 13-05-2026
// This file handles the graphical user interface (GUI) layout, knob styling, tooltips, and
// parameter attachments for the plugin window.

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h" // Added to access the packaged assets

// ====================================================================================================== //

namespace
{
    // Design size of the background art (also the minimum window size). The resize constraint
    // below locks the window to this same aspect ratio, so the background always scales
    // uniformly instead of stretching out of shape.
    constexpr int baseWidth = 500;
    constexpr int baseHeight = 250;

    // Largest the window can be dragged to (same aspect ratio as baseWidth/baseHeight)
    constexpr int maxWidth = 1400;
    constexpr int maxHeight = 700;

    constexpr int numControls = 5;
}

// ====================================================================================================== //

// Constructor for the plugin editor.
SoftEsserAudioProcessorEditor::SoftEsserAudioProcessorEditor (SoftEsserAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&lookAndFeel);

    // Load background image from binary data
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::bg_png, BinaryData::bg_pngSize);

    titleLabel.setText ("SoftEsser", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    setupControl (thresholdControl, "Threshold",
                  "Level, in dB, above which gain reduction begins.",
                  -60.0f, 0.0f, -20.0f, 1, " dB");

    setupControl (amountControl, "Amount",
                  "How strongly the level above the threshold is pulled down.",
                  0.0f, 100.0f, 50.0f, 0, " %");

    setupControl (frequencyControl, "Frequency",
                  "Centre frequency of the band that is monitored for excess level (e.g. sibilance).",
                  4000.0f, 10000.0f, 7000.0f, 0, " Hz");

    setupControl (mixControl, "Mix",
                  "Blend between the processed (wet) and original (dry) signal.",
                  0.0f, 100.0f, 100.0f, 0, " %");

    setupControl (outputControl, "Output",
                  "Output level trim, in dB, applied after processing.",
                  -12.0f, 12.0f, 0.0f, 1, " dB");

    // Allow the window to be resized while keeping the background's original proportions
    setResizable (true, true);
    setResizeLimits (baseWidth, baseHeight, maxWidth, maxHeight);
    getConstrainer()->setFixedAspectRatio ((double) baseWidth / (double) baseHeight);

    setSize (baseWidth, baseHeight);
}

// ====================================================================================================== //

// Destructor for the editor
SoftEsserAudioProcessorEditor::~SoftEsserAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

// ====================================================================================================== //

// Configures one rotary control (slider + its name label) and registers this as its listener
void SoftEsserAudioProcessorEditor::setupControl (ParameterControl& control, const juce::String& name,
                                                   const juce::String& tooltip,
                                                   float minValue, float maxValue, float defaultValue,
                                                   int decimalPlaces, const juce::String& suffix)
{
    auto& slider = control.slider;
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setRange (minValue, maxValue);
    slider.setValue (defaultValue);
    slider.setNumDecimalPlacesToDisplay (decimalPlaces);
    slider.setDoubleClickReturnValue (true, defaultValue);
    slider.setTextValueSuffix (suffix);
    slider.setTooltip (tooltip);
    slider.addListener (this);
    addAndMakeVisible (slider);

    auto& label = control.nameLabel;
    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (14.0f));
    label.setTooltip (tooltip);
    addAndMakeVisible (label);
}

// ====================================================================================================== //

// Drawing the UI background
void SoftEsserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // The window's aspect ratio is locked to the image's own (see the constructor), so this
    // stretch never distorts the image - it's always a uniform scale.
    if (backgroundImage.isValid())
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll (juce::Colour (0xff0F1423));
}

// ====================================================================================================== //

// Lays out every control as a proportion of the current window size, so the UI scales smoothly
// as the plugin is resized rather than staying pinned to fixed pixel positions.
void SoftEsserAudioProcessorEditor::resized()
{
    auto width  = (float) getWidth();
    auto height = (float) getHeight();

    titleLabel.setBounds (juce::Rectangle<float> (0.0f, height * 0.04f, width, height * 0.18f).toNearestInt());

    ParameterControl* controls[numControls] = {
        &thresholdControl, &amountControl, &frequencyControl, &mixControl, &outputControl
    };

    auto labelAreaY      = height * 0.24f;
    auto labelAreaHeight = height * 0.10f;
    auto sliderAreaY     = labelAreaY + labelAreaHeight;
    auto sliderAreaHeight = height * 0.52f;

    auto columnWidth = width / (float) numControls;
    auto sliderSize  = juce::jmin (columnWidth * 0.85f, sliderAreaHeight);

    for (int i = 0; i < numControls; ++i)
    {
        auto columnX = columnWidth * (float) i;

        controls[i]->nameLabel.setBounds (
            juce::Rectangle<float> (columnX, labelAreaY, columnWidth, labelAreaHeight).toNearestInt());

        auto sliderX = columnX + (columnWidth - sliderSize) * 0.5f;
        controls[i]->slider.setBounds (
            juce::Rectangle<float> (sliderX, sliderAreaY, sliderSize, sliderSize).toNearestInt());
    }
}

// ====================================================================================================== //

// Called whenever a slider value changes and updates the processor parameter values in real time
void SoftEsserAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &thresholdControl.slider)
        processorRef.threshold = (float) slider->getValue();

    else if (slider == &amountControl.slider)
        processorRef.amount = (float) slider->getValue();

    else if (slider == &frequencyControl.slider)
        processorRef.frequency = (float) slider->getValue();

    else if (slider == &mixControl.slider)
        processorRef.mix = (float) slider->getValue();

    else if (slider == &outputControl.slider)
        processorRef.outputGain = (float) slider->getValue();
}

// ====================================================================================================== //
