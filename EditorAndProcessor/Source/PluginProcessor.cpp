/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiAudioProcessor::HiAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), pvms(*this, nullptr, "params", createParameterLayout()) // Loads parameters to the tree :)
#endif
{
}

HiAudioProcessor::~HiAudioProcessor()
{
}

//==============================================================================
const juce::String HiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HiAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HiAudioProcessor::getProgramName (int index)
{
    return {};
}

void HiAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void HiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    //GETTING THE VALUES OF THE EDITOR
    auto *dialParam=pvms.getRawParameterValue("gain");
    float gainValue=dialParam->load();
    auto *buttonValue= pvms.getRawParameterValue("power");
    bool powerValue= buttonValue->load();

    DBG("Power button state: " << (powerValue ? "ON" : "OFF"));
    auto *menuChoice=pvms.getRawParameterValue("choice");
    float choiceMenu=menuChoice->load();
    
    DBG("Choice selected: " << choiceMenu);
    
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = 2.0 * gainValue * ((double)rand() / RAND_MAX) - 1.0;
        }
    }
}

//==============================================================================
bool HiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HiAudioProcessor::createEditor()
{
    return new HiAudioProcessorEditor (*this);
}

//==============================================================================
void HiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HiAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout HiAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params; //POINTER
    
   params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain", 1}, "Gain", 0.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"power", 1}, "Power", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"choice", 1}, "Choice",juce::StringArray{"Choice1", "Choice2", "Choice3"}, 0));

    //params.push_back(std::make_unique<juce::AudioParameterBool>("power" "Power",false));
    



    return { params.begin(), params.end()};
    
}
