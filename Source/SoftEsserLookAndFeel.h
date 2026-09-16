// SoftEsser - Built by Shaurya 13-05-2026
// Custom LookAndFeel: flat, single-accent-colour rotary knobs shared by all parameter sliders,
// matching colours for their text boxes/labels, and a shared font scale so every piece of text
// in the editor grows and shrinks together as the window is resized.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ====================================================================================================== //

class SoftEsserLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SoftEsserLookAndFeel()
    {
        setColour (juce::Label::textColourId, textSecondaryColour);
        setColour (juce::Label::backgroundWhenEditingColourId, juce::Colours::black.withAlpha (0.35f));
        setColour (juce::Label::outlineWhenEditingColourId, accentColour);
        setColour (juce::Label::textWhenEditingColourId, juce::Colours::white);

        setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, accentColour.withAlpha (0.3f));
    }

    // Sampled directly from Source/assets/bg.png's brightest glow, so every accent colour in the
    // UI matches the background exactly rather than being eyeballed separately.
    static inline const juce::Colour accentColour { 0xff2EBCD6 };
    static inline const juce::Colour trackColour   { accentColour.withAlpha (0.18f) };
    static inline const juce::Colour pointerColour { accentColour.brighter (0.5f) };
    static inline const juce::Colour textSecondaryColour { 0xffB7C0CC };

    // Component names used to pick the right size in getLabelFont() below. Set via
    // Component::setName() on the labels that need a specific role; anything else
    // (including the Slider's own internal value/edit-box label) falls through to the
    // default case, since Slider never names the label it creates for itself.
    static constexpr const char* titleLabelName     = "title";
    static constexpr const char* paramNameLabelName = "paramName";

    void setFontScale (float newScale) noexcept { fontScale = newScale; }

    // All label text in the editor - the title, the parameter names, and the slider's own
    // value/edit box - is sized from here, so a single scale factor keeps everything in
    // proportion as the window is resized.
    juce::Font getLabelFont (juce::Label& label) override
    {
        if (label.getName() == titleLabelName)
            return juce::Font (26.0f * fontScale, juce::Font::bold);

        if (label.getName() == paramNameLabelName)
            return juce::Font (13.0f * fontScale);

        return juce::Font (15.0f * fontScale).withExtraKerningFactor (0.02f);
    }

    // Draws a flat arc-style rotary knob: a dim background track, a bright accent arc showing
    // the current value, and a short pointer line - no bevels or gradients.
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                            juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto lineThickness = juce::jmax (2.0f, radius * 0.14f);
        auto arcRadius = radius - lineThickness * 0.5f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                      rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (trackColour);
        g.strokePath (backgroundArc, juce::PathStrokeType (lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, toAngle, true);
        g.setColour (accentColour);
        g.strokePath (valueArc, juce::PathStrokeType (lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto pointerLength = radius * 0.55f;
        auto pointerThickness = juce::jmax (2.0f, lineThickness * 0.6f);
        juce::Path pointer;
        pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, pointerThickness * 0.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (centre));
        g.setColour (pointerColour);
        g.fillPath (pointer);
    }

private:
    float fontScale = 1.0f;
};

// ====================================================================================================== //
