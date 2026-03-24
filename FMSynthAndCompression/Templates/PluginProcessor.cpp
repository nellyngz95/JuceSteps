/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FMSynthCompressionAudioProcessor::FMSynthCompressionAudioProcessor()
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

FMSynthCompressionAudioProcessor::~FMSynthCompressionAudioProcessor()
{
}

//==============================================================================
const juce::String FMSynthCompressionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FMSynthCompressionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthCompressionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthCompressionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FMSynthCompressionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FMSynthCompressionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FMSynthCompressionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FMSynthCompressionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FMSynthCompressionAudioProcessor::getProgramName (int index)
{
    return {};
}

void FMSynthCompressionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FMSynthCompressionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //Declare the spec object.
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
 //Prepare the spec for the compressor
}

void FMSynthCompressionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FMSynthCompressionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FMSynthCompressionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
     auto numInputChannels  = getTotalNumInputChannels();
     auto numOutputChannels = getTotalNumOutputChannels();
    //FM modulation values.
    float modulationDepth = parameters.getRawParameterValue("ModulationDepth")->load();
    float modulatorFrequency = parameters.getRawParameterValue("ModulatorFrequency")->load();
    float carrierFrequency = parameters.getRawParameterValue("CarrierFrequency")->load();
    // Load Compressor values. thereshold,ratio, makeupgain, attack and release time
    //float threshold=
   
    
    float sampleRate = (float) juce::AudioProcessor::getSampleRate();
    

     for (auto i = numInputChannels; i < numOutputChannels; ++i)
         buffer.clear (i, 0, buffer.getNumSamples());

     for (int i = 0; i < buffer.getNumSamples(); ++i)
     {
       //Declare your modulatorSignal
        //Declare the modulatedCarrierSignal
       for (int j = 0; j < numOutputChannels; ++j) buffer.getWritePointer(j)[i] = modulatedCarrierSignal;
        //Declare and increment the modulatorPhase.
       while (modulatorPhase >= 1.0) modulatorPhase -= 1.0;
       
        //declare and increment the modulatedCarrierPhase.
       while (modulatedCarrierPhase >= 1.0) modulatedCarrierPhase -= 1.0;
     }
    //Send  the compressor parameters to the to the dsp module compressor.Set(variable)
    //Create the dsp AudioBlock<float>
    juce::dsp::ProcessContextReplacing<float> context(block);
    //process the context within the compressor.
    buffer.applyGain(juce::Decibels::decibelsToGain(makeUpGain)); //another way to turn the linear to Db.
}

//==============================================================================
bool FMSynthCompressionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FMSynthCompressionAudioProcessor::createEditor()
{
    //return new juce::GenericAudioProcessorEditor(this);
    return new FMSynthCompressionAudioProcessorEditor (*this);
}

//==============================================================================
void FMSynthCompressionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FMSynthCompressionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FMSynthCompressionAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout FMSynthCompressionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    //Sliders FM
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ModulationDepth", 1},"Modulation Depth", 0.0f, 1000.0f, 200.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ModulatorFrequency", 1},"Modulator Frequency",10.0f, 200.0f, 100.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CarrierFrequency", 1},"Carrier Frequency", 50.0f, 2000.0f, 800.0f));
    
    //Sliders compression
    //Push back parameters: Threshold (-60.0f, 0.0f, 0.0f)),Ratio (1.0f, 20.0, 1.0f) and MakeUpGain(0.0f, 24.0, 0.0f)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"AttackTime", 1},"AttackTime", 0.1f, 80.0f, 15.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ReleaseTime", 1},"ReleaseTime", 1.0f, 1000.0f, 100.0f));
    
    
    return {parameters.begin(),parameters.end()};
}
