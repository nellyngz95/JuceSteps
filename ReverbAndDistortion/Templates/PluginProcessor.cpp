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
    //Loading distortion parameters variables auto gain, int distortiontype.
  
    //DBG("Distortion type: " << distortiontype);
    //Reverb parameters
    juce::Reverb::Parameters params;
    params.roomSize=parameters.getRawParameterValue("roomSize")->load();
    //Push the rest of the parameters of the reverb to the params object. Use te load method, shown above.
    reverb.setParameters(params);
    

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

   
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        for (int j = 0; j < totalNumInputChannels; ++j)
               {
                 auto sample = gain * buffer.getReadPointer(j)[i]; //Input signal.Microphone.
                 switch (distortiontype) //switching the types of distortion
                 {
                   case 1://Hard Clipping. Threshold is half the maximum.
                   {
                     //Declare const float threshold, will be 0.5 f. (Half of the amplitude).
                       //what happens when the signal is above or below the threshold?
                     if         // positive clip
                     else if // negative clip
                     break;
                   }
                   case 2://Soft Clipping.
                   {
                   //Declare two const float  threshold1, threshold 2. One will be 1/3 and the other 2/3.
                     if () // sample greater than threshold 1
                     {
                        //Positive clip
                       if (sample > threshold2) sample = 0.5f; //Threshold greater than threshold 2.. then it cuts to 0.5
                       else sample = ;// soft knee (positive)
                     }
                     else if (sample < -threshold1)
                     {
                       if (sample < -threshold2) sample = ;       // negative clip What would be the value of the signal?
                       else sample = ;// soft knee (negative)
                     }
                     break;
                   }
                   case 3: //Exponential Soft Clipping
                   {
                     if ()// positive
                     else // negative
                     break;
                   }
                   case 4: //FullWave Rectifier
                   {
                     
                     break;
                   }
                   case 5: //Half Wave Rectifier
                   { //Be careful compensate for  the volume increase, signal cannot surpass 1. What do you do?
                     
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
       //processMono
    else if (totalNumInputChannels == 2)
        //processStereo.

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
