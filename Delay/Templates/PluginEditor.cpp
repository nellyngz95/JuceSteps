/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //addAndMakeVisible your sliders
    addAndMakeVisible(delayTimeSlider);
   
    
    //Set the styles  and attach them to the components
   delayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
   delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
   delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
   delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.attachToComponent(&delayTimeSlider, false);
    
    
   
    
    //Attach them to the processor
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delayTime", delayTimeSlider);
      
    
    setSize (400, 300);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
}

//==============================================================================
void DelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setColour (juce::Colours::white); // A nice color for the title
        g.setFont (juce::FontOptions (24.0f, juce::Font::bold)); // Make it bigger and bold

        auto headerArea = getLocalBounds().removeFromTop(50);
            
        g.drawFittedText ("Delay", headerArea, juce::Justification::centred, 1);
}

void DelayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
        
        area.removeFromTop(50);

        auto topRow = area.removeFromTop(100);
             
        delayTimeSlider.setBounds(topRow.removeFromLeft(100));
       
}
