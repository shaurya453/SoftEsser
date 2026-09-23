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

    // Called ~30 times a second from the editor's timer with the processor's latest raw gain
    // reduction reading. Rather than drawing that value directly (which would make the meter
    // jump around), it's eased toward with fast-attack/slow-release ballistics - like a real
    // hardware meter - and a peak marker is tracked that holds briefly before decaying back
    // down, so short peaks stay visible instead of flashing by.
    void setLevel (float newReductionDb) noexcept
    {
        targetDb = juce::jlimit (0.0f, maxDisplayDb, newReductionDb);
        displayDb += (targetDb - displayDb) * (targetDb > displayDb ? attackCoeff : releaseCoeff);

        if (displayDb >= peakDb)
        {
            peakDb = displayDb;
            peakHoldFramesLeft = peakHoldFrames;
        }
        else if (peakHoldFramesLeft > 0)
        {
            --peakHoldFramesLeft;
        }
        else
        {
            peakDb = juce::jmax (displayDb, peakDb - peakDecayPerFrame);
        }

        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // "GR" title along the top
        auto titleArea = bounds.removeFromTop (bounds.getHeight() * 0.09f);
        g.setColour (SoftEsserLookAndFeel::textSecondaryColour);
        g.setFont (juce::Font (SoftEsserLookAndFeel::uiTypefaceName, 12.0f * fontScale, juce::Font::plain));
        g.drawText ("GR", titleArea, juce::Justification::centred);

        // Live numeric readout along the bottom
        auto valueArea = bounds.removeFromBottom (bounds.getHeight() * 0.12f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (SoftEsserLookAndFeel::uiTypefaceName, 13.0f * fontScale, juce::Font::plain));
        g.drawText (juce::String (displayDb, 1) + " dB", valueArea, juce::Justification::centred);

        auto gap = bounds.getHeight() * 0.03f;
        bounds.removeFromTop (gap);
        bounds.removeFromBottom (gap);

        // dB tick labels on the left, the bar itself on the right
        auto tickArea = bounds.removeFromLeft (bounds.getWidth() * 0.42f);
        bounds.removeFromLeft (bounds.getWidth() * 0.10f);
        auto barBounds = bounds;

        auto corner = barBounds.getWidth() * 0.2f;

        // Recessed track, so the bar reads as an actual meter well rather than a flat shape.
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (barBounds, corner);

        // Filled from the top down, proportional to the current (smoothed) reduction - 0 dB (no
        // reduction) sits at the top of an empty meter, maxDisplayDb sits at the bottom of a
        // full one.
        auto fillHeight = barBounds.getHeight() * (displayDb / maxDisplayDb);

        if (fillHeight > 1.0f)
        {
            auto fillBounds = barBounds.withHeight (fillHeight);

            juce::Path fillPath;
            fillPath.addRoundedRectangle (fillBounds, corner);

            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (fillPath);

            // A cool-to-warm gradient spanning the whole track (not just the filled part), so
            // heavier reduction reads as visually "hotter" as the fill grows down into it,
            // rather than the whole bar just being one flat colour.
            juce::ColourGradient gradient (SoftEsserLookAndFeel::accentColour, barBounds.getX(), barBounds.getY(),
                                            juce::Colour (0xffE8544A), barBounds.getX(), barBounds.getBottom(), false);
            gradient.addColour (0.6, juce::Colour (0xffF2B33D));
            g.setGradientFill (gradient);
            g.fillRect (barBounds);

            // Thin segment gaps over the fill, like the cells of an LED meter, so it reads as a
            // proper level meter rather than a plain stretched pill.
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            constexpr int numSegments = 18;
            auto segmentHeight = barBounds.getHeight() / (float) numSegments;
            for (int i = 1; i < numSegments; ++i)
            {
                auto y = barBounds.getY() + segmentHeight * (float) i;
                g.fillRect (juce::Rectangle<float> (barBounds.getX(), y - 0.5f, barBounds.getWidth(), 1.0f));
            }
        }

        g.setColour (SoftEsserLookAndFeel::accentColour.withAlpha (0.5f));
        g.drawRoundedRectangle (barBounds.reduced (0.5f), corner, 1.0f);

        // Peak-hold marker: briefly sits at the highest recent reduction before decaying back
        // down, so a quick transient is still visible after the bar itself has relaxed.
        if (peakDb > 0.4f)
        {
            auto peakY = barBounds.getY() + barBounds.getHeight() * (peakDb / maxDisplayDb);
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.fillRect (juce::Rectangle<float> (barBounds.getX() + 1.0f, peakY - 1.0f, barBounds.getWidth() - 2.0f, 2.0f));
        }

        // dB scale ticks, 0 at the top down to -maxDisplayDb at the bottom
        static constexpr float tickValues[] = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f };

        g.setFont (juce::Font (SoftEsserLookAndFeel::uiTypefaceName, 10.0f * fontScale, juce::Font::plain));
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
    float targetDb  = 0.0f; // raw value most recently passed to setLevel()
    float displayDb = 0.0f; // eased value actually drawn
    float peakDb    = 0.0f;
    int peakHoldFramesLeft = 0;
    float fontScale = 1.0f;

    // Tuned for the editor's 30Hz poll rate: attack reaches ~99% of a step in ~5 frames (~165ms),
    // release in ~25 frames (~830ms), so the meter follows increases briskly but falls smoothly.
    static constexpr float attackCoeff  = 0.55f;
    static constexpr float releaseCoeff = 0.18f;
    static constexpr int peakHoldFrames = 45;          // ~1.5s at 30Hz before the peak starts decaying
    static constexpr float peakDecayPerFrame = 0.35f;  // dB/frame after the hold expires
};

// ====================================================================================================== //
