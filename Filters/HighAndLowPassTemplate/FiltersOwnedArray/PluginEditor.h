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
class FiltersLabAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FiltersLabAudioProcessorEditor (FiltersLabAudioProcessor&);
    ~FiltersLabAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    FiltersLabAudioProcessor& audioProcessor;
    juce::Slider highGain,lowGain,highQ,lowQ, highFrequency,lowFrequency;
    juce::Label highLabel,lowLabel, highQLabel,lowQLabel,highFrequencyLabel,lowFrequencyLabel;
    juce::TextButton file,play,stop;
    std::unique_ptr<juce::FileChooser> chooser;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment,lowGainAttachment, highQAttachment,lowQattachment, lowFrequencyAttachment,highFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fileAttachment,playAttachment,stopAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FiltersLabAudioProcessorEditor)
};
