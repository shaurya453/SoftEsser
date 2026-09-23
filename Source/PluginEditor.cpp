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
    // uniformly instead of stretching out of shape. Shared with SoftEsserAudioProcessor so the
    // last-used window size it remembers is always within these same bounds.
    constexpr int baseWidth  = SoftEsserAudioProcessor::defaultEditorWidth;
    constexpr int baseHeight = SoftEsserAudioProcessor::defaultEditorHeight;

    // Largest the window can be dragged to (same aspect ratio as baseWidth/baseHeight)
    constexpr int maxWidth  = SoftEsserAudioProcessor::maxEditorWidth;
    constexpr int maxHeight = SoftEsserAudioProcessor::maxEditorHeight;

    constexpr int numControls = 8;
    constexpr int numControlColumns = 4;
    constexpr int numControlRows = numControls / numControlColumns;

    // Base (unscaled) size of each slider's value text box - see resized(), which rescales
    // this every time the window size changes.
    constexpr int textBoxBaseWidth = 70;
    constexpr int textBoxBaseHeight = 20;
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
                  "Level, in dB, above which gain reduction begins.", " dB");

    setupControl (amountControl, SoftEsserAudioProcessor::amountParamID, "Amount",
                  "How strongly the level above the threshold is pulled down.", " %");

    setupControl (frequencyControl, SoftEsserAudioProcessor::frequencyParamID, "Frequency",
                  "Split point between the untouched low band and the de-essed high band.", " Hz");

    setupControl (qControl, SoftEsserAudioProcessor::qParamID, "Q",
                  "Resonance of the crossover split. Higher values create a sharper, more surgical separation at Frequency.", "");

    setupControl (attackControl, SoftEsserAudioProcessor::attackParamID, "Attack",
                  "How quickly the de-esser responds once the high band crosses the threshold.", " ms");

    setupControl (releaseControl, SoftEsserAudioProcessor::releaseParamID, "Release",
                  "How quickly the de-esser lets go once the high band falls back below the threshold.", " ms");

    setupControl (mixControl, SoftEsserAudioProcessor::mixParamID, "Mix",
                  "Blend between the processed (wet) and original (dry) signal.", " %");

    setupControl (outputControl, SoftEsserAudioProcessor::outputGainParamID, "Output",
                  "Output level trim, in dB, applied after processing.", " dB");

    listenButton.setTooltip ("Solo the high band so you can hear exactly what's being de-essed.");
    listenButton.setClickingTogglesState (true);
    listenButton.setColour (juce::TextButton::buttonColourId, juce::Colours::black.withAlpha (0.35f));
    listenButton.setColour (juce::TextButton::buttonOnColourId, SoftEsserLookAndFeel::accentColour);
    listenButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    listenButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    addAndMakeVisible (listenButton);
    listenAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processorRef.apvts, SoftEsserAudioProcessor::listenParamID, listenButton);

    addAndMakeVisible (gainReductionMeter);

    // A/B compare: both slots start out identical to the plugin's current state.
    stateSnapshotA = presetManager.getStateSnapshot();
    stateSnapshotB = stateSnapshotA;

    abButtonA.setTooltip ("Recall the A slot. Click while already on A to store the current settings into it.");
    abButtonB.setTooltip ("Recall the B slot. Click while already on B to store the current settings into it.");
    abButtonA.setClickingTogglesState (false);
    abButtonB.setClickingTogglesState (false);

    for (auto* button : { &abButtonA, &abButtonB })
    {
        button->setColour (juce::TextButton::buttonColourId, juce::Colours::black.withAlpha (0.35f));
        button->setColour (juce::TextButton::buttonOnColourId, SoftEsserLookAndFeel::accentColour);
        button->setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        button->setColour (juce::TextButton::textColourOnId, juce::Colours::black);
        addAndMakeVisible (*button);
    }

    abButtonA.setToggleState (true, juce::dontSendNotification);
    abButtonA.onClick = [this] { switchAbSlot (true); };
    abButtonB.onClick = [this] { switchAbSlot (false); };

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

    // Reopen at whatever size the window was last left at (persisted on processorRef - see
    // PluginProcessor::getStateInformation()/setStateInformation() - so this also survives a
    // host project reload), rather than always resetting back to the design size.
    setSize (processorRef.lastEditorWidth, processorRef.lastEditorHeight);

    startTimerHz (30);
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
                                                   const juce::String& suffix)
{
    auto& slider = control.slider;
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, textBoxBaseWidth, textBoxBaseHeight);
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

// Switches the active A/B slot. If the requested slot is already active, this instead re-stores
// the current settings into it (so you can update A or B without leaving it) - otherwise the
// current settings are stored into the slot being left, and the requested slot is recalled.
void SoftEsserAudioProcessorEditor::switchAbSlot (bool switchToA)
{
    if (switchToA == currentlyOnSlotA)
    {
        (switchToA ? stateSnapshotA : stateSnapshotB) = presetManager.getStateSnapshot();
        return;
    }

    (currentlyOnSlotA ? stateSnapshotA : stateSnapshotB) = presetManager.getStateSnapshot();
    presetManager.restoreStateSnapshot (switchToA ? stateSnapshotA : stateSnapshotB);
    currentlyOnSlotA = switchToA;

    abButtonA.setToggleState (currentlyOnSlotA, juce::dontSendNotification);
    abButtonB.setToggleState (! currentlyOnSlotA, juce::dontSendNotification);
}

// ====================================================================================================== //

// Polls the processor's current gain reduction a few times a second and pushes it into the meter
void SoftEsserAudioProcessorEditor::timerCallback()
{
    gainReductionMeter.setLevel (processorRef.currentGainReductionDb.load (std::memory_order_relaxed));
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
    // scale factor for both dimensions - used to keep every font size and the value text
    // boxes' size in proportion too.
    auto scale = height / (float) baseHeight;
    lookAndFeel.setFontScale (scale);
    gainReductionMeter.setFontScale (scale);

    // Remember this size so the next time the editor is opened - whether that's reopening the
    // window in the same session or reloading the host project - it comes back at the size it
    // was left at instead of resetting to the design size.
    processorRef.lastEditorWidth  = getWidth();
    processorRef.lastEditorHeight = getHeight();

    titleLabel.setBounds (juce::Rectangle<float> (0.0f, 0.0f, width, height * 0.10f).toNearestInt());

    // Knob grid (4 columns x 2 rows) on the left, vertical GR meter on the right - both share
    // the same content row so adding the meter never eats into the knobs' own vertical space.
    auto contentX      = width * 0.02f;
    auto contentWidth  = width * 0.96f;
    auto contentY      = height * 0.12f;
    auto contentHeight = height * 0.60f;

    auto meterWidth = contentWidth * 0.14f;
    auto gridGap    = contentWidth * 0.02f;
    auto gridWidth  = contentWidth - meterWidth - gridGap;

    gainReductionMeter.setBounds (
        juce::Rectangle<float> (contentX + gridWidth + gridGap, contentY, meterWidth, contentHeight).toNearestInt());

    ParameterControl* controls[numControls] = {
        &thresholdControl, &amountControl, &frequencyControl, &qControl,
        &attackControl, &releaseControl, &mixControl, &outputControl
    };

    auto columnWidth = gridWidth / (float) numControlColumns;
    auto rowHeight   = contentHeight / (float) numControlRows;
    auto labelAreaHeight  = rowHeight * 0.22f;
    auto sliderAreaHeight = rowHeight * 0.76f;
    auto sliderSize  = juce::jmin (columnWidth * 0.82f, sliderAreaHeight * 0.92f);

    for (int i = 0; i < numControls; ++i)
    {
        auto column = i % numControlColumns;
        auto row    = i / numControlColumns;

        auto columnX = contentX + columnWidth * (float) column;
        auto rowY    = contentY + rowHeight * (float) row;

        controls[i]->nameLabel.setBounds (
            juce::Rectangle<float> (columnX, rowY, columnWidth, labelAreaHeight).toNearestInt());

        auto sliderX = columnX + (columnWidth - sliderSize) * 0.5f;
        auto sliderY = rowY + labelAreaHeight + (sliderAreaHeight - sliderSize) * 0.5f;
        controls[i]->slider.setBounds (
            juce::Rectangle<float> (sliderX, sliderY, sliderSize, sliderSize).toNearestInt());

        // The value text box's own size is a fixed pixel size as far as JUCE is concerned, so
        // it has to be re-applied here on every resize to keep it in proportion with everything
        // else (including the rectangular outline drawn around it while it's being edited).
        controls[i]->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                              (int) (textBoxBaseWidth * scale), (int) (textBoxBaseHeight * scale));
    }

    // Listen toggle + A/B compare row
    auto controlRowY      = height * 0.76f;
    auto controlRowHeight = height * 0.08f;
    auto controlRowWidth  = width * 0.7f;
    auto controlRowX      = (width - controlRowWidth) * 0.5f;

    auto abGroupWidth = controlRowWidth * 0.45f;
    auto listenWidth = controlRowWidth * 0.45f;
    auto abGap = controlRowWidth * 0.03f;
    auto abButtonWidth = (abGroupWidth - abGap) * 0.5f;
    auto listenX = controlRowX + controlRowWidth - listenWidth;

    abButtonA.setBounds (
        juce::Rectangle<float> (controlRowX, controlRowY, abButtonWidth, controlRowHeight).toNearestInt());
    abButtonB.setBounds (
        juce::Rectangle<float> (controlRowX + abButtonWidth + abGap, controlRowY, abButtonWidth, controlRowHeight).toNearestInt());
    listenButton.setBounds (
        juce::Rectangle<float> (listenX, controlRowY, listenWidth, controlRowHeight).toNearestInt());

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
