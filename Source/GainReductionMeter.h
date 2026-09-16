// SoftEsser - Built by Shaurya 13-05-2026
// Vertical meter showing how much gain reduction the de-esser is currently applying to the high
// band, with a dB tick scale and a live numeric readout. Purely a display - the editor's timer
// polls the processor and pushes the value in via setLevel(); this class has no knowledge of the
// processor or apvts.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SoftEsserLookAndFeel.h"

// ====================================================================================================== //

class GainReductionMeter : public juce::Component
{
public:
    // Reduction amounts are clamped to this range for display purposes.
    static constexpr float maxDisplayDb = 24.0f;

    // Keeps the meter's text sized in proportion with the rest of the UI - see
    // SoftEsserLookAndFeel::setFontScale(), called alongside this from the editor's resized().
    void setFontScale (float newScale) noexcept
    {
        fontScale = newScale;
        repaint();
    }

    void setLevel (float newReductionDb) noexcept
    {
        auto clamped = juce::jlimit (0.0f, maxDisplayDb, newReductionDb);

        if (! juce::approximatelyEqual (clamped, reductionDb))
        {
            reductionDb = clamped;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // "GR" title along the top
        auto titleArea = bounds.removeFromTop (bounds.getHeight() * 0.09f);
        g.setColour (SoftEsserLookAndFeel::textSecondaryColour);
        g.setFont (juce::Font (13.0f * fontScale, juce::Font::bold));
        g.drawText ("GR", titleArea, juce::Justification::centred);

        // Live numeric readout along the bottom
        auto valueArea = bounds.removeFromBottom (bounds.getHeight() * 0.12f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (13.0f * fontScale));
        g.drawText (juce::String (reductionDb, 1) + " dB", valueArea, juce::Justification::centred);

        auto gap = bounds.getHeight() * 0.03f;
        bounds.removeFromTop (gap);
        bounds.removeFromBottom (gap);

        // dB tick labels on the left, the bar itself on the right
        auto tickArea = bounds.removeFromLeft (bounds.getWidth() * 0.46f);
        bounds.removeFromLeft (bounds.getWidth() * 0.08f);
        auto barBounds = bounds;

        auto corner = barBounds.getWidth() * 0.28f;

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (barBounds, corner);

        // Filled from the top down, proportional to the current reduction - 0 dB (no reduction)
        // sits at the top of an empty meter, maxDisplayDb sits at the bottom of a full one.
        auto fillHeight = barBounds.getHeight() * (reductionDb / maxDisplayDb);

        if (fillHeight > 0.0f)
        {
            g.setColour (SoftEsserLookAndFeel::accentColour);
            g.fillRoundedRectangle (barBounds.withHeight (fillHeight), corner);
        }

        g.setColour (SoftEsserLookAndFeel::accentColour.withAlpha (0.4f));
        g.drawRoundedRectangle (barBounds.reduced (0.5f), corner, 1.0f);

        // dB scale ticks, 0 at the top down to -maxDisplayDb at the bottom
        static constexpr float tickValues[] = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f };

        g.setFont (juce::Font (10.0f * fontScale));
        g.setColour (SoftEsserLookAndFeel::textSecondaryColour);

        for (auto tick : tickValues)
        {
            auto proportion = (-tick) / maxDisplayDb;
            auto tickY = barBounds.getY() + barBounds.getHeight() * proportion;
            auto labelHeight = 14.0f * fontScale;

            g.drawText (juce::String ((int) tick),
                        juce::Rectangle<float> (tickArea.getX(), tickY - labelHeight * 0.5f, tickArea.getWidth(), labelHeight),
                        juce::Justification::centredRight);
        }
    }

private:
    float reductionDb = 0.0f;
    float fontScale = 1.0f;
};

// ====================================================================================================== //
