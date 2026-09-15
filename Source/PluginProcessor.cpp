// SoftEsser - Built by Shaurya 13-05-2026
// Core DSP: band-pass detection, envelope following, threshold-based gain reduction, wet/dry mix.

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ====================================================================================================== //

// One-pole smoothing coefficient for the envelope follower (closer to 1 = slower/smoother).
static constexpr float envelopeSmoothing = 0.99f;

// Small offset added before converting the envelope to dB, so a silent signal (envelope == 0)
// doesn't produce -infinity dB.
static constexpr float envelopeNoiseFloor = 0.0001f;

// ====================================================================================================== //

// Constructor for audio processor, defines stereo input and output buses
SoftEsserAudioProcessor::SoftEsserAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

// ====================================================================================================== //

// Destructor
SoftEsserAudioProcessor::~SoftEsserAudioProcessor()
{
}

// ====================================================================================================== //

// Returns the plugin name shown by the host
const juce::String SoftEsserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

// ====================================================================================================== //

// Plugin capability functions
bool SoftEsserAudioProcessor::acceptsMidi() const { return false; }
bool SoftEsserAudioProcessor::producesMidi() const { return false; }
bool SoftEsserAudioProcessor::isMidiEffect() const { return false; }
double SoftEsserAudioProcessor::getTailLengthSeconds() const { return 0.0; }

// ====================================================================================================== //

// Program/preset handling functions
int SoftEsserAudioProcessor::getNumPrograms() { return 1; }
int SoftEsserAudioProcessor::getCurrentProgram() { return 0; }
void SoftEsserAudioProcessor::setCurrentProgram (int /*index*/) {}
const juce::String SoftEsserAudioProcessor::getProgramName (int /*index*/) { return {}; }
void SoftEsserAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/) {}

// ====================================================================================================== //

// Called before audio playback begins, initializes the detection filters using the current
// sample rate. processBlock() also recomputes these every block so they keep tracking the
// frequency slider while it's being moved.
void SoftEsserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    bandPassFilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, frequency, filterQ);
    bandPassFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, frequency, filterQ);
}

// ====================================================================================================== //

// Called when playback stops, used for cleanup if necessary
void SoftEsserAudioProcessor::releaseResources()
{
}

// ====================================================================================================== //

// Checks whether the requested bus layout is supported, allows mono and stereo only
bool SoftEsserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// ====================================================================================================== //

// MAIN AUDIO PROCESSING FUNCTION
//     > Band-pass filtering, envelope following, threshold comparison, gain reduction, wet/dry mixing, output gain adjustment

void SoftEsserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto sampleRate = getSampleRate();

    // Convert percentage parameters to normalized values
    float wet = mix / 100.0f;
    float amountNormalized = amount / 100.0f;

    // Recompute the detection filters every block so they track live slider changes
    bandPassFilterL.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, frequency, filterQ);

    bandPassFilterR.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, frequency, filterQ);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        auto& filter = (channel == 0) ? bandPassFilterL : bandPassFilterR;
        auto& envelope = (channel == 0) ? envelopeL : envelopeR;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float input = channelData[sample];

            // Filter application
            float filtered = filter.processSample (input);

            // Get absolute amplitude values (without polarity)
            float detector = std::abs (filtered);

            // Envelope follower, smoothens the signal
            envelope = envelopeSmoothing * envelope + (1.0f - envelopeSmoothing) * detector;

            float envelopeDb =
                juce::Decibels::gainToDecibels (envelope + envelopeNoiseFloor);

            // No compression below the threshold
            float gainReductionDb = 0.0f;

            // De-essing logic
            if (envelopeDb > threshold)
            {
                // Calculate excess amplitude
                float excess = envelopeDb - threshold;

                // Calculate gain reduction value
                gainReductionDb = -excess * amountNormalized;
            }

            // Convert dB to linear gain for processing
            float gain =
                juce::Decibels::decibelsToGain (gainReductionDb);

            // Apply processing to the original (unfiltered) signal
            float processed = input * gain;

            // Wet/dry mix
            float output =
                (processed * wet) +
                (input * (1.0f - wet));

            // Output gain conversion
            output *= juce::Decibels::decibelsToGain (outputGain);

            channelData[sample] = output;
        }
    }
}

// ====================================================================================================== //

// Returns whether the plugin has a GUI editor
bool SoftEsserAudioProcessor::hasEditor() const
{
    return true;
}

// ====================================================================================================== //

// Creates and returns the plugin editor instance
juce::AudioProcessorEditor* SoftEsserAudioProcessor::createEditor()
{
    return new SoftEsserAudioProcessorEditor (*this);
}

// ====================================================================================================== //

// Saves plugin state info (not yet implemented)
void SoftEsserAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

// ====================================================================================================== //

// Restores plugin state info (not yet implemented)
void SoftEsserAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

// ====================================================================================================== //

// Creates plugin filter instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoftEsserAudioProcessor();
}

// ====================================================================================================== //
