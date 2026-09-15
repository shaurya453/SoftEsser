// SoftEsser - Built by Shaurya 13-05-2026
// Declares the audio processor: DSP state, plugin parameters, and JUCE AudioProcessor overrides.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// ====================================================================================================== //

// SoftEsser is a frequency-selective de-esser: it band-pass filters the signal around a target
// frequency to detect sibilance, then pulls the level down once that band's envelope crosses
// a threshold. See SoftEsserAudioProcessor::processBlock() for the full signal chain.
class SoftEsserAudioProcessor final : public juce::AudioProcessor
{
public:

    // Constructor and Destructor
    SoftEsserAudioProcessor();
    ~SoftEsserAudioProcessor() override;

    // Playback preparation and cleanup
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    // Bus layout support
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // Main processing function
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // Editor creation
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Plugin info
    const juce::String getName() const override;

    // MIDI support (unused - this is an audio-only effect, not a MIDI plugin)
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    // Plugin latency/tail info
    double getTailLengthSeconds() const override;

    // Program/preset handling (unused - plugin only ever has a single, fixed program)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // State save/load (not yet implemented - parameters are plain floats, not persisted)
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // USER PARAMETERS
    // Set directly by the editor's sliders (see PluginEditor::sliderValueChanged), read every
    // block in processBlock(). No smoothing is applied, so moving a slider changes the sound
    // immediately.
    float threshold  = -20.0f;   // dB level above which gain reduction begins
    float amount     = 4.0f;     // 0-100, how strongly the excess level is pulled down
    float frequency  = 6000.0f;  // Hz, centre frequency of the sibilance detection filter
    float mix        = 100.0f;   // 0-100, wet/dry blend of the processed signal
    float outputGain = 0.0f;     // dB, applied after the wet/dry mix

private:

    // Band-pass filters used only to detect the level of the target frequency band (sidechain
    // style); the gain reduction they produce is applied to the full, unfiltered signal.
    juce::dsp::IIR::Filter<float> bandPassFilterL;
    juce::dsp::IIR::Filter<float> bandPassFilterR;

    // One-pole envelope follower state, tracked separately per channel so the left and right
    // channels don't bleed into each other's gain reduction.
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;

    // Q factor of the detection band-pass filter
    static constexpr float filterQ = 2.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftEsserAudioProcessor)
};

// ====================================================================================================== //
