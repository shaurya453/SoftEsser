// SoftEsser - Built by Shaurya 13-05-2026
// Core DSP: low/high band split, envelope following, threshold-based gain reduction, wet/dry mix.

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// ====================================================================================================== //

// Small offset added before converting the envelope to dB, so a silent signal (envelope == 0)
// doesn't produce -infinity dB.
static constexpr float envelopeNoiseFloor = 0.0001f;

// ====================================================================================================== //

// Constructor for audio processor, defines stereo input and output buses, the parameter tree,
// and caches raw pointers into it for fast reads on the audio thread.
SoftEsserAudioProcessor::SoftEsserAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    thresholdParam  = apvts.getRawParameterValue (thresholdParamID);
    amountParam     = apvts.getRawParameterValue (amountParamID);
    frequencyParam  = apvts.getRawParameterValue (frequencyParamID);
    qParam          = apvts.getRawParameterValue (qParamID);
    attackParam     = apvts.getRawParameterValue (attackParamID);
    releaseParam    = apvts.getRawParameterValue (releaseParamID);
    mixParam        = apvts.getRawParameterValue (mixParamID);
    outputGainParam = apvts.getRawParameterValue (outputGainParamID);
    listenParam     = apvts.getRawParameterValue (listenParamID);
}

// ====================================================================================================== //

// Destructor
SoftEsserAudioProcessor::~SoftEsserAudioProcessor()
{
}

// ====================================================================================================== //

// Declares the user parameters: ID, display name, range, and default value. This is the
// single source of truth for parameter ranges/defaults - the editor's sliders read them back
// via their attachments rather than duplicating these numbers.
namespace
{
    // AudioParameterFloat's *default* text formatting derives its decimal-place count from the
    // NormalisableRange's interval, which for a continuous (interval == 0) range can come out as
    // up to 7 decimal places (e.g. "7000.0000000") - and critically, this is what the editor's
    // sliders actually display, since SliderAttachment wires the slider's text display straight
    // to the parameter's getText() rather than to Slider::setNumDecimalPlacesToDisplay(). Setting
    // this explicitly on every parameter is the only way to control what's shown.
    juce::AudioParameterFloatAttributes withOneDecimalPlace (const juce::String& label)
    {
        return juce::AudioParameterFloatAttributes().withLabel (label)
            .withStringFromValueFunction ([] (float value, int) { return juce::String (value, 1); });
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SoftEsserAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { thresholdParamID, 1 }, "Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f), -20.0f,
        withOneDecimalPlace ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { amountParamID, 1 }, "Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f), 50.0f,
        withOneDecimalPlace ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { frequencyParamID, 1 }, "Frequency",
        juce::NormalisableRange<float> (4000.0f, 10000.0f), 7000.0f,
        withOneDecimalPlace ("Hz")));

    // Resonance of the low-pass crossover filter that splits the signal at Frequency. 0.707
    // (Butterworth) is a flat, gentle split; higher values sharpen the corner for a more
    // surgical separation between the two bands.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { qParamID, 1 }, "Q",
        juce::NormalisableRange<float> (0.3f, 6.0f), 0.707f,
        withOneDecimalPlace ("")));

    // Envelope follower ballistics for the high band's detector. Fast attack catches sibilant
    // transients quickly; release controls how long the reduction holds before letting go
    // (too fast can pump/chatter, too slow can dull consonants after the "s").
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { attackParamID, 1 }, "Attack",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.0f, 0.4f), 2.0f,
        withOneDecimalPlace ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { releaseParamID, 1 }, "Release",
        juce::NormalisableRange<float> (5.0f, 300.0f, 0.0f, 0.4f), 60.0f,
        withOneDecimalPlace ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { mixParamID, 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f), 100.0f,
        withOneDecimalPlace ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { outputGainParamID, 1 }, "Output",
        juce::NormalisableRange<float> (-12.0f, 12.0f), 0.0f,
        withOneDecimalPlace ("dB")));

    // Solos the high band to the output, so you can hear exactly what's being de-essed while
    // tuning Frequency/Q. Off by default (0 = normal processed output).
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { listenParamID, 1 }, "Listen", false));

    return { params.begin(), params.end() };
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

// Called before audio playback begins, initializes the crossover filter using the current
// sample rate and resets the envelope followers. processBlock() also recomputes the filter every
// block so it keeps tracking the frequency slider while it's being moved.
void SoftEsserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    crossoverFilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequencyParam->load(), qParam->load());
    crossoverFilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequencyParam->load(), qParam->load());

    envelopeL = 0.0f;
    envelopeR = 0.0f;
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
//     > Band split, envelope following, threshold comparison, gain reduction, wet/dry mixing, output gain adjustment

void SoftEsserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto sampleRate = getSampleRate();

    // Read the current parameter values once per block (lock-free atomic reads)
    auto threshold  = thresholdParam->load();
    auto frequency  = frequencyParam->load();
    auto q          = qParam->load();
    auto attackMs   = attackParam->load();
    auto releaseMs  = releaseParam->load();
    auto outputGain = outputGainParam->load();
    bool listen     = listenParam->load() > 0.5f;
    float wet = mixParam->load() / 100.0f;
    float amountNormalized = amountParam->load() / 100.0f;

    // Recompute the crossover filter every block so it tracks live slider changes
    crossoverFilterL.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequency, q);

    crossoverFilterR.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, frequency, q);

    // One-pole attack/release coefficients for the envelope follower, derived from the current
    // sample rate so Attack/Release keep the same real-world timing regardless of project
    // sample rate (a fixed coefficient would run faster at higher sample rates).
    auto attackCoeff  = std::exp (-1.0f / (float) (sampleRate * (attackMs  * 0.001)));
    auto releaseCoeff = std::exp (-1.0f / (float) (sampleRate * (releaseMs * 0.001)));

    // Largest reduction applied anywhere in this block, across both channels - drives the
    // editor's gain-reduction meter.
    float peakReductionDb = 0.0f;

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        auto& crossover = (channel == 0) ? crossoverFilterL : crossoverFilterR;
        auto& envelope = (channel == 0) ? envelopeL : envelopeR;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float input = channelData[sample];

            // Split the signal at Frequency. highBand is obtained by subtraction rather than a
            // second filter, so lowBand + highBand always sums back to the original input
            // exactly - the split can never introduce a gap or bump at the crossover point.
            float lowBand = crossover.processSample (input);
            float highBand = input - lowBand;

            // Get absolute amplitude of the band being de-essed (without polarity)
            float detector = std::abs (highBand);

            // Envelope follower: rises at the Attack rate, falls at the Release rate
            envelope = (detector > envelope)
                ? attackCoeff * envelope + (1.0f - attackCoeff) * detector
                : releaseCoeff * envelope + (1.0f - releaseCoeff) * detector;

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

            peakReductionDb = juce::jmax (peakReductionDb, -gainReductionDb);

            // Listen mode: send the high band itself to the output, unprocessed, so you can
            // hear exactly what's being de-essed.
            if (listen)
            {
                channelData[sample] = highBand;
                continue;
            }

            // Convert dB to linear gain and apply it to the high band only - the low band
            // passes through completely untouched.
            float gain =
                juce::Decibels::decibelsToGain (gainReductionDb);

            float processed = lowBand + (highBand * gain);

            // Wet/dry mix
            float output =
                (processed * wet) +
                (input * (1.0f - wet));

            // Output gain conversion
            output *= juce::Decibels::decibelsToGain (outputGain);

            channelData[sample] = output;
        }
    }

    currentGainReductionDb.store (peakReductionDb, std::memory_order_relaxed);
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

// Saves plugin state info: serialises apvts's current parameter values to XML
void SoftEsserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    xml->setAttribute ("editorWidth", lastEditorWidth);
    xml->setAttribute ("editorHeight", lastEditorHeight);
    copyXmlToBinary (*xml, destData);
}

// ====================================================================================================== //

// Restores plugin state info: replaces apvts's state from previously-saved XML, plus the last
// editor window size stashed alongside it by getStateInformation() above.
void SoftEsserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));

        lastEditorWidth  = juce::jlimit (defaultEditorWidth, maxEditorWidth,
                                          xmlState->getIntAttribute ("editorWidth", defaultEditorWidth));
        lastEditorHeight = juce::jlimit (defaultEditorHeight, maxEditorHeight,
                                          xmlState->getIntAttribute ("editorHeight", defaultEditorHeight));
    }
}

// ====================================================================================================== //

// Creates plugin filter instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoftEsserAudioProcessor();
}

// ====================================================================================================== //
