// SoftEsser - Built by Shaurya 13-05-2026
// Declares the audio processor: DSP state, plugin parameters, and JUCE AudioProcessor overrides.

#pragma once

#include <atomic>
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

    // Program/preset handling (unused - JUCE's "programs" are a legacy VST2 concept; user
    // presets are handled separately by PresetManager, which saves/loads apvts's state)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // State save/load, so the host can persist parameter values with the project (and restore
    // them on reload). Implemented by serialising apvts's ValueTree to XML.
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter IDs, shared by createParameterLayout() below, the editor's slider attachments,
    // and PresetManager.
    static constexpr const char* thresholdParamID  = "threshold";
    static constexpr const char* amountParamID     = "amount";
    static constexpr const char* frequencyParamID  = "frequency";
    static constexpr const char* mixParamID        = "mix";
    static constexpr const char* outputGainParamID = "outputGain";

    // All five user parameters. Backs host automation, project save/load (getStateInformation),
    // and presets (PresetManager saves/loads snapshots of this same state).
    juce::AudioProcessorValueTreeState apvts;

private:

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Cached pointers into apvts for lock-free reads on the audio thread in processBlock().
    std::atomic<float>* thresholdParam  = nullptr;
    std::atomic<float>* amountParam     = nullptr;
    std::atomic<float>* frequencyParam  = nullptr;
    std::atomic<float>* mixParam        = nullptr;
    std::atomic<float>* outputGainParam = nullptr;

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
