/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PanningAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PanningAudioProcessorEditor (PanningAudioProcessor&);
    ~PanningAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
   
    juce::Slider panning,distance;
    juce::Label distanceLabel,panningLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panningAttachment,distanceAttachment;
    PanningAudioProcessor& audioProcessor;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanningAudioProcessorEditor)
};
