/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FiltersLabAudioProcessor::FiltersLabAudioProcessor()
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
    //Initialize the transport source.
    formatManager.registerBasicFormats(); //which formats we're gonna be using
}

FiltersLabAudioProcessor::~FiltersLabAudioProcessor()
{
}

//==============================================================================
const juce::String FiltersLabAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FiltersLabAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FiltersLabAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FiltersLabAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FiltersLabAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FiltersLabAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FiltersLabAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FiltersLabAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FiltersLabAudioProcessor::getProgramName (int index)
{
    return {};
}

void FiltersLabAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FiltersLabAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
    lowFilters.clear();
      for (int i = 0; i < getTotalNumInputChannels(); ++i) {
        juce::IIRFilter* filter;
        lowFilters.add(filter = new juce::IIRFilter());
      }
     
      highFilters.clear();
    
      for (int i = 0; i < getTotalNumInputChannels(); ++i) {
        juce::IIRFilter* filter;
        highFilters.add(filter = new juce::IIRFilter());
      }
}


void FiltersLabAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FiltersLabAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    
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
void FiltersLabAudioProcessor::loadFile(const juce::File& file)
{
    transportSource.stop();
    transportSource.setSource (nullptr);
    
    auto* reader = formatManager.createReaderFor(file);//Read the file...

    if (reader != nullptr) //if the reader is not empty
    {
        DBG("File Loaded Successfully: " << file.getFileName());
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);//starting te variable who is gonna store the content.
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.swap(newSource);
        transportSource.start();
    }
    else
        {
            DBG("Failed to create reader for: " << file.getFileName());
        }
}
void FiltersLabAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels  = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    float sampleRate = (float)juce::AudioProcessor::getSampleRate();

    // Clear extra channels
    for (auto i = numInputChannels; i < numOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // Get parameters
    auto play = parameters.getRawParameterValue("play");
    auto stop = parameters.getRawParameterValue("stop");

    auto lowFrequency = parameters.getRawParameterValue("lowFrequency")->load();
    auto lowGain = pow(10.0f, parameters.getRawParameterValue("lowGain")->load() / 20.0f);
    auto lowQ = parameters.getRawParameterValue ("lowQ")->load();

    auto highFrequency = parameters.getRawParameterValue("highFrequency")->load();
    auto highGain = pow(10.0f, parameters.getRawParameterValue("highGain")->load() / 20.0f);
    auto highQ = parameters.getRawParameterValue ("highQ")->load();

    // Handle stop button
    if (stop->load() > 0.5f)
    {
        transportSource.stop();
        transportSource.setPosition(0.0);
        buffer.clear();
        return;
    }
    
    if (play->load() > 0.5f)
    {
        // If we have a loaded file, play it
        if (readerSource != nullptr)
        {
            if (!transportSource.isPlaying())
                transportSource.start();
            
            juce::AudioSourceChannelInfo loadedFile(buffer);
            transportSource.getNextAudioBlock(loadedFile);
        }
        else
        {
            // Generate noise if no file loaded
            for (int channel = 0; channel < numInputChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                for (int i = 0; i < numSamples; ++i)
                    channelData[i] = (2.0f * rand() / (float)RAND_MAX - 1.0f) * 0.2f;
            }
        }
    }
    else
    {
        buffer.clear();
        return;
    }

    // Apply filters
    auto lowCoefficients = juce::IIRCoefficients::makeLowShelf(sampleRate, lowFrequency, lowQ, lowGain);
    for (int i = 0; i < lowFilters.size(); i++)
        lowFilters[i]->setCoefficients(lowCoefficients);

    auto highCoefficients = juce::IIRCoefficients::makeHighShelf(sampleRate, highFrequency, highQ, highGain);
    for (int i = 0; i < highFilters.size(); i++)
        highFilters[i]->setCoefficients(highCoefficients);

    for (int channel = 0; channel < numInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        lowFilters[channel]->processSamples(channelData, numSamples);
        highFilters[channel]->processSamples(channelData, numSamples);
    }
}
//==============================================================================
bool FiltersLabAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FiltersLabAudioProcessor::createEditor()
{
    return new FiltersLabAudioProcessorEditor (*this);
}

//==============================================================================
void FiltersLabAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FiltersLabAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FiltersLabAudioProcessor();
}
juce::AudioProcessorValueTreeState::ParameterLayout FiltersLabAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    //Buttons
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"file", 1},"File", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"play", 1},"Play", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stop", 1},"Stop", 0));
    //Sliders
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highGain", 1},"HighGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highQ", 1},"HighQ", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highFrequency", 1},"highFrequency", 450.0f, 18000.0f, 3000.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowGain", 1},"lowGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowQ", 1},"LowQ", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowFrequency", 1},"LowFrequency", 10.0f, 600.0f, 150.0f));
    
    
    return {parameters.begin(),parameters.end()};
    
}
