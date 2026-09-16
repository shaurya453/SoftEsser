// SoftEsser - Built by Shaurya 13-05-2026
// This file handles the graphical user interface (GUI) layout, knob styling, tooltips, presets,
// and parameter attachments for the plugin window.

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
    : AudioProcessorEditor (&p), processorRef (p), presetManager (p.apvts)
{
    setLookAndFeel (&lookAndFeel);

    // Load background image from binary data
    backgroundImage = juce::ImageCache::getFromMemory (BinaryData::bg_png, BinaryData::bg_pngSize);

    titleLabel.setText ("SoftEsser", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    titleLabel.setName (SoftEsserLookAndFeel::titleLabelName); // picks its font size from the LookAndFeel
    addAndMakeVisible (titleLabel);

    setupControl (thresholdControl, SoftEsserAudioProcessor::thresholdParamID, "Threshold",
                  "Level, in dB, above which gain reduction begins.", 1, " dB");

    setupControl (amountControl, SoftEsserAudioProcessor::amountParamID, "Amount",
                  "How strongly the level above the threshold is pulled down.", 0, " %");

    setupControl (frequencyControl, SoftEsserAudioProcessor::frequencyParamID, "Frequency",
                  "Centre frequency of the band that is monitored for excess level (e.g. sibilance).", 0, " Hz");

    setupControl (mixControl, SoftEsserAudioProcessor::mixParamID, "Mix",
                  "Blend between the processed (wet) and original (dry) signal.", 0, " %");

    setupControl (outputControl, SoftEsserAudioProcessor::outputGainParamID, "Output",
                  "Output level trim, in dB, applied after processing.", 1, " dB");

    presetBox.setTooltip ("Load a saved preset.");
    presetBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha (0.35f));
    presetBox.setColour (juce::ComboBox::outlineColourId, SoftEsserLookAndFeel::accentColour.withAlpha (0.4f));
    presetBox.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    presetBox.onChange = [this]
    {
        auto selected = presetBox.getText();
        if (selected.isNotEmpty() && selected != presetManager.getCurrentPresetName())
            presetManager.loadPreset (selected);
    };
    addAndMakeVisible (presetBox);

    savePresetButton.setTooltip ("Save the current knob settings as a new preset.");
    savePresetButton.setColour (juce::TextButton::buttonColourId, SoftEsserLookAndFeel::accentColour.withAlpha (0.25f));
    savePresetButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    savePresetButton.onClick = [this] { showSavePresetDialog(); };
    addAndMakeVisible (savePresetButton);

    refreshPresetBox();

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

// Configures one rotary control (slider + its name label) and attaches it to its apvts parameter
void SoftEsserAudioProcessorEditor::setupControl (ParameterControl& control, const juce::String& parameterID,
                                                   const juce::String& displayName, const juce::String& tooltip,
                                                   int decimalPlaces, const juce::String& suffix)
{
    auto& slider = control.slider;
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setNumDecimalPlacesToDisplay (decimalPlaces);
    slider.setTextValueSuffix (suffix);
    slider.setTooltip (tooltip);
    addAndMakeVisible (slider);

    // Binds the slider's range/value to the apvts parameter (including its default value for
    // double-click-to-reset) and keeps both in sync from then on, in both directions.
    control.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.apvts, parameterID, slider);

    auto& label = control.nameLabel;
    label.setText (displayName, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setName (SoftEsserLookAndFeel::paramNameLabelName); // picks its font size from the LookAndFeel
    label.setTooltip (tooltip);
    addAndMakeVisible (label);
}

// ====================================================================================================== //

// Repopulates presetBox from PresetManager's current preset list, selecting the active preset
void SoftEsserAudioProcessorEditor::refreshPresetBox()
{
    auto presets = presetManager.getAllPresets();

    presetBox.clear (juce::dontSendNotification);
    presetBox.addItemList (presets, 1);

    auto currentIndex = presets.indexOf (presetManager.getCurrentPresetName());
    presetBox.setSelectedItemIndex (currentIndex, juce::dontSendNotification);
}

// ====================================================================================================== //

// Opens a text-entry dialog asking for a preset name, then saves under that name
void SoftEsserAudioProcessorEditor::showSavePresetDialog()
{
    presetNameWindow = std::make_unique<juce::AlertWindow> (
        "Save Preset", "Enter a name for this preset:", juce::MessageBoxIconType::NoIcon);

    presetNameWindow->addTextEditor ("name", presetManager.getCurrentPresetName());
    presetNameWindow->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    presetNameWindow->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    presetNameWindow->enterModalState (true, juce::ModalCallbackFunction::create (
        [this] (int result)
        {
            if (result == 1 && presetNameWindow != nullptr)
            {
                auto name = presetNameWindow->getTextEditorContents ("name").trim();

                if (name.isNotEmpty())
                {
                    presetManager.savePreset (name);
                    refreshPresetBox();
                }
            }

            presetNameWindow.reset();
        }), false);
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

    // Aspect ratio is locked (see the constructor), so height/baseHeight alone is an exact
    // scale factor for both dimensions - used to keep every font size in proportion too.
    lookAndFeel.setFontScale (height / (float) baseHeight);

    titleLabel.setBounds (juce::Rectangle<float> (0.0f, height * 0.04f, width, height * 0.18f).toNearestInt());

    ParameterControl* controls[numControls] = {
        &thresholdControl, &amountControl, &frequencyControl, &mixControl, &outputControl
    };

    auto labelAreaY      = height * 0.24f;
    auto labelAreaHeight = height * 0.10f;
    auto sliderAreaY     = labelAreaY + labelAreaHeight;
    auto sliderAreaHeight = height * 0.44f;

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

    // Preset row along the bottom
    auto presetRowY      = height * 0.88f;
    auto presetRowHeight = height * 0.09f;
    auto presetRowWidth  = width * 0.7f;
    auto presetRowX      = (width - presetRowWidth) * 0.5f;
    auto presetGap       = presetRowWidth * 0.02f;
    auto saveButtonWidth = presetRowWidth * 0.28f;
    auto presetBoxWidth  = presetRowWidth - saveButtonWidth - presetGap;

    presetBox.setBounds (
        juce::Rectangle<float> (presetRowX, presetRowY, presetBoxWidth, presetRowHeight).toNearestInt());
    savePresetButton.setBounds (
        juce::Rectangle<float> (presetRowX + presetBoxWidth + presetGap, presetRowY, saveButtonWidth, presetRowHeight).toNearestInt());

    // Bounds changes above already trigger repaints for components whose size actually moved,
    // but a font-scale-only change can leave some bounds numerically unchanged - repaint
    // everything explicitly so text always reflects the current scale.
    repaint();
}

// ====================================================================================================== //
