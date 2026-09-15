// SoftEsser - Built by Shaurya 13-05-2026
// Custom LookAndFeel: flat, single-accent-colour rotary knobs shared by all parameter sliders,
// plus matching colours for their text boxes and labels.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ====================================================================================================== //

class SoftEsserLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SoftEsserLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, textSecondaryColour);
    }

    static inline const juce::Colour accentColour       { 0xff35D8CE };
    static inline const juce::Colour trackColour         { 0x4dffffff };
    static inline const juce::Colour textSecondaryColour { 0xffB7C0CC };

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
        g.setColour (juce::Colours::white);
        g.fillPath (pointer);
    }
};

// ====================================================================================================== //
