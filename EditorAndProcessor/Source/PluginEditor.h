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
class HiAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    HiAudioProcessorEditor (HiAudioProcessor&);
    ~HiAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Slider dial;
    juce::TextButton button;
    juce::ComboBox menu;
    // ATTTACHMENTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment; //POINTER THAT WILL
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> choiceAttachment;
    HiAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HiAudioProcessorEditor)
};
