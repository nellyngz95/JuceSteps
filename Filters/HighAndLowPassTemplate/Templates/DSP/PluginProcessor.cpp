/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FiltersLab2AudioProcessor::FiltersLab2AudioProcessor()
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
    formatManager.registerBasicFormats(); //which formats we're gonna be using
        DBG("Constructor complete");
}

FiltersLab2AudioProcessor::~FiltersLab2AudioProcessor()
{
}

//==============================================================================
const juce::String FiltersLab2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FiltersLab2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FiltersLab2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FiltersLab2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FiltersLab2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FiltersLab2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FiltersLab2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void FiltersLab2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FiltersLab2AudioProcessor::getProgramName (int index)
{
    return {};
}

void FiltersLab2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FiltersLab2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec; ///specification object.
    //using spec get the samplerate.
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock; //audio is processed in blocks (set of samples) How many samples will you give me at once?
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();
    
    lowFilter.prepare(spec); //allocate memory for the filters
   //Prepare the high pass
    
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    
}

void FiltersLab2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FiltersLab2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FiltersLab2AudioProcessor::loadFile(const juce::File& file)
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
void FiltersLab2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels  = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    
    // Clear extra channels
    for (auto i = numInputChannels; i < numOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);
    
    auto play = parameters.getRawParameterValue("play");
    auto stop = parameters.getRawParameterValue("stop");
    
    // Handle stop button
    if (stop->load() > 0.5f)
    {
        transportSource.stop();
        transportSource.setPosition(0.0);
        buffer.clear();
        return;
    }
    
    // If not playing, just return
    if (play->load() < 0.5f)
    {
        buffer.clear();
        return;
    }
    
    // Now we know we're playing - get audio
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
    
    // Get filter parameters
    
    
    auto lowFrequency = parameters.getRawParameterValue("lowFrequency")->load();
    auto lowGain = pow(10.0f, parameters.getRawParameterValue("lowGain")->load() / 20.0f);
    auto lowQ = parameters.getRawParameterValue("lowQ")->load();
    
    auto highFrequency = parameters.getRawParameterValue("highFrequency")->load();
    auto highGain = pow(10.0f, parameters.getRawParameterValue("highGain")->load() / 20.0f);
    auto highQ = parameters.getRawParameterValue("highQ")->load();
    
    auto sampleRate = getSampleRate();
    
   //get the low filter state
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, highFrequency, highQ, highGain);
    
    juce::dsp::AudioBlock<float> block(buffer); //buffer = your raw audio data (the actual sound samples) //  AudioBlock = wraps it in a format the DSP filters understand
    juce::dsp::ProcessContextReplacing<float> context(block); //"process this audio and replace it with the filtered version"
    
    //process the low filter.
    highFilter.process(context);
}
//==============================================================================
bool FiltersLab2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FiltersLab2AudioProcessor::createEditor()
{
    return new FiltersLab2AudioProcessorEditor (*this);
}

//==============================================================================
void FiltersLab2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FiltersLab2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FiltersLab2AudioProcessor();
}
juce::AudioProcessorValueTreeState::ParameterLayout FiltersLab2AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    //Buttons
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"file", 1},"File", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"play", 1},"Play", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stop", 1},"Stop", 0));
    //Sliders
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highGain", 1},"HighGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highQ", 1},"HighQ", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highFrequency", 1},"highFrequency", 450.0f, 18000.0f, 3000.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowGain", 1},"lowGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowQ", 1},"LowQ", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowFrequency", 1},"LowFrequency", 10.0f, 600.0f, 150.0f));
    
    
    return {parameters.begin(),parameters.end()};
    
}
