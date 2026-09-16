// SoftEsser - Built by Shaurya 13-05-2026
// Small horizontal meter showing how much gain reduction the de-esser is currently applying.
// Purely a display - the editor's timer polls the processor and pushes the value in via
// setLevel(); this class has no knowledge of the processor or apvts.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SoftEsserLookAndFeel.h"

// ====================================================================================================== //

class GainReductionMeter : public juce::Component
{
public:
    // Reduction amounts are clamped to this range for display purposes.
    static constexpr float maxDisplayDb = 24.0f;

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
        auto corner = bounds.getHeight() * 0.5f;

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (bounds, corner);

        auto fillBounds = bounds.withWidth (bounds.getWidth() * (reductionDb / maxDisplayDb));

        if (fillBounds.getWidth() > 0.0f)
        {
            g.setColour (SoftEsserLookAndFeel::accentColour);
            g.fillRoundedRectangle (fillBounds, corner);
        }

        g.setColour (SoftEsserLookAndFeel::accentColour.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);
    }

private:
    float reductionDb = 0.0f;
};

// ====================================================================================================== //
