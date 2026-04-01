/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbDistortionAudioProcessor::ReverbDistortionAudioProcessor()
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

ReverbDistortionAudioProcessor::~ReverbDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReverbDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReverbDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReverbDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb.setSampleRate(sampleRate);
    reverb.reset();
}

void ReverbDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ReverbDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    //Loading distortion parameters
    auto gain = parameters.getRawParameterValue("GainDistortion")->load();
    int distortiontype = parameters.getRawParameterValue("DistortionSelection")->load();
    DBG("Distortion type: " << distortiontype);
    //Reverb parameters
    juce::Reverb::Parameters params;
    params.roomSize=parameters.getRawParameterValue("roomSize")->load();
    params.damping=parameters.getRawParameterValue("damping")->load();
    params.wetLevel=parameters.getRawParameterValue("wetLevel")->load();
    params.dryLevel=parameters.getRawParameterValue("dryLevel")->load();
    params.width=parameters.getRawParameterValue("width")->load();
    reverb.setParameters(params);
    

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

   
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for (int j = 0; j < totalNumInputChannels; ++j)
               {
                 auto sample = gain * buffer.getReadPointer(j)[i];
                 switch (distortiontype)
                 {
                   case 1://Hard Clipping. Threshold is half the maximum.
                   {
                     const float threshold = 0.5f;
                     if (sample > threshold) sample = threshold;         // positive clip
                     else if (sample < -threshold) sample = -threshold;  // negative clip
                     break;
                   }
                   case 2://Soft Clipping.
                   {
                     const float threshold1 = 1.0f / 3.0f;
                     const float threshold2 = 2.0f / 3.0f;
                     if (sample > threshold1) // sample greater than threshold 1
                     {
                        //Positive clip
                       if (sample > threshold2) sample = 0.5f; //Threshold greater than threshold 2
                       else sample = (3.0f - (2.0f - 3.0f * sample) * (2.0f - 3.0f * sample)) / 6.0f;// soft knee (positive)
                     }
                     else if (sample < -threshold1)
                     {
                       if (sample < -threshold2) sample = -0.5f;       // negative clip
                       else sample = -(3.0f -  powf((2.0f + 3.0f * sample),2.0f)) / 6.0f;// soft knee (negative)
                     }
                     break;
                   }
                   case 3: //Exponential Soft Clipping
                   {
                     if (sample > 0.0f) sample = 1.0f - expf(-sample);// positive
                     else sample = -1.0f + expf(sample);// negative
                     break;
                   }
                   case 4: //FullWave Rectifier
                   {
                     sample = fabs(sample);
                     break;
                   }
                   case 5: //Half Wave Rectifier
                   { //Be careful compensate for  the volume increase, signal cannot surpass 1. What do you do?
                     sample = 0.5f * (fabs(sample) + sample);
                     break;
                   }
                   default:
                     break;
                 }
                 buffer.getWritePointer(j)[i] = sample;
               }
    }
    //Reverb (not smaple by sample)
    if (totalNumInputChannels == 1)
        reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
    else if (totalNumInputChannels == 2)
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

//==============================================================================
bool ReverbDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbDistortionAudioProcessor::createEditor()
{
    return new ReverbDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void ReverbDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReverbDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbDistortionAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbDistortionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    //Switch
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"DistortionSelection", 1},"Distortion Selection",juce::StringArray {"Hard Clipping", "Soft Clipping", "Exponential Soft","Full Wave Rectifier", "Half Wave Rectifier"},0));
    //Distortion
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"GainDistortion", 1},"Gain Distortion", 0.0f, 10.0, 1.0f));
    //Reverb
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"roomSize", 1},"roomSize", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"damping", 1},"damping",0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"wetLevel", 1},"wetLevel", 0.0f, 1.0f, 0.33f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dryLevel", 1},"dryLevel", 0.0f, 1.0f, 0.4f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"width", 1},"width",  0.0f, 1.0f, 1.0f));
    return {parameters.begin(),parameters.end()};
    
}
