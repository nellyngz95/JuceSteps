/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiAudioProcessorEditor::HiAudioProcessorEditor (HiAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    dial.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    dial.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 10);
    dial.setRange(0.0,24.0,0.0);
    addAndMakeVisible(dial);
    gainAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.pvms, "gain", dial);
    
    button.setClickingTogglesState(true);  // Ensures button toggles ON/OFF when clicked

    button.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);

    button.onClick = [this]() {
        bool isOn = button.getToggleState();
        DBG("Button state: " << (isOn ? "ON" : "OFF"));
    };

    addAndMakeVisible(button);

    // Attach to AudioProcessorValueTreeState
    buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.pvms, "power", button);

   
    
    menu.addItem("Choice1",1);
    menu.addItem("Choice2",2);
    menu.addItem("Choice3",3);
    menu.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(menu);
    choiceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.pvms, "choice", menu);
   
    setSize (1000, 1000);
}

HiAudioProcessorEditor::~HiAudioProcessorEditor()
{
}

//==============================================================================
void HiAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
}

void HiAudioProcessorEditor::resized()
{
    auto bounds= getLocalBounds();
    const int componentSize {100};
    dial.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize,componentSize));
    button.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize,componentSize));
    menu.setBounds(bounds.removeFromTop(200).withSizeKeepingCentre(componentSize,componentSize));
}
