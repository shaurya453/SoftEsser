// SoftEsser - Built by Shaurya 13-05-2026
// Declares the audio processor: DSP state, plugin parameters, and JUCE AudioProcessor overrides.

#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// ====================================================================================================== //

// SoftEsser is a split-band de-esser: the signal is split at Frequency into an untouched low
// band and a high band, and only the high band's level is pulled down once its envelope crosses
// a threshold, before the two bands are summed back together. See
// SoftEsserAudioProcessor::processBlock() for the full signal chain.
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
    static constexpr const char* qParamID          = "filterQ";
    static constexpr const char* attackParamID     = "attack";
    static constexpr const char* releaseParamID    = "release";
    static constexpr const char* mixParamID        = "mix";
    static constexpr const char* outputGainParamID = "outputGain";
    static constexpr const char* listenParamID     = "listen";

    // All user parameters. Backs host automation, project save/load (getStateInformation),
    // and presets (PresetManager saves/loads snapshots of this same state).
    juce::AudioProcessorValueTreeState apvts;

    // Peak gain reduction, in dB, applied during the most recently processed block. Written on
    // the audio thread in processBlock(), read on the message thread by the editor's meter -
    // an atomic is enough to make that safe since it's just a single scalar readout.
    std::atomic<float> currentGainReductionDb { 0.0f };

    // Editor window size, remembered across the editor being closed/reopened (message-thread
    // only, so plain ints are fine) and persisted into the saved state below so it survives a
    // host project reload too. Defaults/limits are shared with PluginEditor.cpp so the editor
    // never has to duplicate these numbers.
    static constexpr int defaultEditorWidth  = 500;
    static constexpr int defaultEditorHeight = 250;
    static constexpr int maxEditorWidth  = 1400;
    static constexpr int maxEditorHeight = 700;
    int lastEditorWidth  = defaultEditorWidth;
    int lastEditorHeight = defaultEditorHeight;

private:

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Cached pointers into apvts for lock-free reads on the audio thread in processBlock().
    std::atomic<float>* thresholdParam  = nullptr;
    std::atomic<float>* amountParam     = nullptr;
    std::atomic<float>* frequencyParam  = nullptr;
    std::atomic<float>* qParam          = nullptr;
    std::atomic<float>* attackParam     = nullptr;
    std::atomic<float>* releaseParam    = nullptr;
    std::atomic<float>* mixParam        = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* listenParam     = nullptr;

    // Low-pass crossover filter. The high band isn't filtered directly - it's obtained as
    // (input - lowBandOutput), so the two bands are guaranteed to sum back to the original
    // signal exactly (no phase-mismatch gap or bump at the crossover point, regardless of Q).
    juce::dsp::IIR::Filter<float> crossoverFilterL;
    juce::dsp::IIR::Filter<float> crossoverFilterR;

    // One-pole envelope follower state, tracked separately per channel so the left and right
    // channels don't bleed into each other's gain reduction. Its ballistics (Attack/Release)
    // are recomputed from the current sample rate every block - see processBlock().
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftEsserAudioProcessor)
};

// ====================================================================================================== //
