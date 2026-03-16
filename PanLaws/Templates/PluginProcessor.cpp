/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PanningAudioProcessor::PanningAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),parameters(*this, nullptr, "parameters", createParameterLayout())
#endif
{
}

PanningAudioProcessor::~PanningAudioProcessor() {}

//==============================================================================
const juce::String PanningAudioProcessor::getName() const { return JucePlugin_Name; }

bool PanningAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PanningAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PanningAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PanningAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int PanningAudioProcessor::getNumPrograms()                { return 1; }
int PanningAudioProcessor::getCurrentProgram()             { return 0; }
void PanningAudioProcessor::setCurrentProgram (int)        {}
const juce::String PanningAudioProcessor::getProgramName (int) { return {}; }
void PanningAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void PanningAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PanningAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PanningAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


//==============================================================================
void PanningAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples        = buffer.getNumSamples();
    auto panning  = parameters.getRawParameterValue("panning");
    auto distance = parameters.getRawParameterValue("distance");
   //Load(->load)the values in float variabls pan and dist.

    
    //Program the pan laws. Float variables leftpan and right pan. std::cos orstd::sin for pi= juce::mothConstants<float>::pi
    //Try to program the distancegain in a float variable distanceGain

    if (numOutputChannels >= 2)
    {
        // Stereo output — full panning
        
        auto* leftData  = buffer.getWritePointer(0);
        // get the rightData from the buffer channel 1?
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float mono = (leftData[sample] + rightData[sample]) * 0.5f; //Avoid clipping multiply the data of your channels *0.5f. (1+1)=2 =DISTORTION.
            leftData[sample]  = //send the signal*panning*distance.
            rightData[sample] = //send the signal* panning* distance.
        }
    }
    else
    {
        // Mono output — just apply distance gain, no panning possible
        auto* monoData = buffer.getWritePointer(0);
        for (int sample = 0; sample < numSamples; ++sample)
            monoData[sample] *= distanceGain;
    }
}
//==============================================================================
bool PanningAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* PanningAudioProcessor::createEditor()
{
    return new PanningAudioProcessorEditor(*this);
}

//==============================================================================
void PanningAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void PanningAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PanningAudioProcessor();
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PanningAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    // Start your panning slider, Min :0.0, Max1.0 and 0.5 for balancing the signal between the two channels.
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"distance", 1}, "Distance", 0.0f, 1.0f, 0.0f));

    return {parameters.begin(), parameters.end()};
}
