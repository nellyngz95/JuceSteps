/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(Button=new juce::AudioParameterBool("on","ON",false));
    addParameter(delayTimeParam = new juce::AudioParameterFloat("delayTime", "Delay time", 0.0f, 2.0f, 0.0f));
    addParameter(feedbackParam = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 0.999f, 0.0f));
    addParameter(dryWetMixParam = new juce::AudioParameterFloat("dryWetMix", "Dry/wet mix", 0.0f, 1.0, 0.0f));
    
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Allocate and zero the delay buffer (size depends on sample rate)
      auto delayTime = delayTimeParam->get();
      delayBufferLength = (int)(2.0 * sampleRate);
      delayBuffer.setSize(2, delayBufferLength);
      delayBuffer.clear();

      // Since delayTime is in seconds, use sample rate to find out what delay position offset should be
      delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;

}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Helpful information about this block of samples:
    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    float sampleRate = (float)juce::AudioProcessor::getSampleRate();
    auto delayTime = delayTimeParam->get();
    auto feedback = feedbackParam->get();
    auto dryWetMix = dryWetMixParam->get();
    auto playSine = Button->get();  // Control when to stop the sine wave

    static float phase = 0.0f;
    
    float frequency = 440.0f;      // 440 Hz (A note)
    float amplitude = 0.5f;

    // Update read position based on current value of delay time
    delayReadPosition = (int)(delayWritePosition - (delayTime * sampleRate) + delayBufferLength) % delayBufferLength;

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < numInputChannels; ++j) {
            float in = 0.0f; // Default input is silence
                        
                        // Generate sine wave for 1 second, then stop
                        if (playSine) {
                            in = amplitude * sinf(2.0f * juce::MathConstants<float>::pi * frequency * phase / sampleRate);
                            phase += 1.0f;

                        }

                        // Get delay buffer for this channel
                        float* delayData = delayBuffer.getWritePointer(j);

                        // Safeguard delay buffer access
                        if (delayData != nullptr) {
                            delayData[delayWritePosition] = in + delayData[delayReadPosition] * feedback;

                            // Read from delay buffer and apply dry/wet mix
                            float out = (1 - dryWetMix) * in + dryWetMix * delayData[delayReadPosition];

                            // Write processed output
                            buffer.getWritePointer(j)[i] = out;
                        }
                    }

                    // Increment delay buffer positions safely
                    delayReadPosition = (delayReadPosition + 1) % delayBufferLength;
                    delayWritePosition = (delayWritePosition + 1) % delayBufferLength;
                }

    // Clear any unused output channels
    for (int i = numInputChannels; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
