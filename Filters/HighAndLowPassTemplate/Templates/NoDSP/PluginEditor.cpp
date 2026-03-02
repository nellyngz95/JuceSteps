/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FiltersLabAudioProcessorEditor::FiltersLabAudioProcessorEditor (FiltersLabAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    //BUTTONS:
    auto buttons={&file, &play, &stop};
     for (auto* button:buttons)
     {
         addAndMakeVisible(button);
         if (button != &file)
                 button->setClickingTogglesState(true);
     }
    file.setButtonText("Load File");
    fileAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "file", file);
    play.setButtonText("Play");
    playAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "play", play);
    stop.setButtonText("Stop");
    stopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stop", stop);
    
    auto sliders = { &highGain, &lowGain, &highQ, &lowQ, &lowFrequency,&highFrequency };
    auto labels  = { &highLabel, &lowLabel, &highQLabel, &lowQLabel, &lowFrequencyLabel,&highFrequencyLabel };
    //instead of writing add andmake visible to every single slider we create foor loops :)
    for (auto* slider : sliders)
        {
            addAndMakeVisible (slider);
            slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
        }
    for (auto* label : labels)
        {
            addAndMakeVisible (label);
            label->setJustificationType (juce::Justification::centred);
            label->setBorderSize(juce::BorderSize<int>(0));
        }
    
    highFrequencyLabel.setText ("High Frequency", juce::dontSendNotification);
    highFrequencyLabel.setJustificationType(juce::Justification::centred);
    highFrequencyLabel.attachToComponent (&highFrequency, false);
    
    lowFrequencyLabel.setText ("Low Q", juce::dontSendNotification);
    lowFrequencyLabel.setJustificationType(juce::Justification::centred);
    lowFrequencyLabel.attachToComponent (&lowFrequency, false);
    
    
    highLabel.setText ("High Gain", juce::dontSendNotification);
    highLabel.setJustificationType(juce::Justification::centred);
    highLabel.attachToComponent (&highGain, false);
        
    lowLabel.setText ("Low Gain", juce::dontSendNotification);
    lowLabel.setJustificationType(juce::Justification::centred);
    lowLabel.attachToComponent (&lowGain, false);
    
    highQLabel.setText ("High Q", juce::dontSendNotification);
    highQLabel.setJustificationType(juce::Justification::centred);
    highQLabel.attachToComponent (&highQ, false);
        
    lowQLabel.setText ("Low Q", juce::dontSendNotification);
    lowQLabel.setJustificationType(juce::Justification::centred);
    lowQLabel.attachToComponent (&lowQ, false);
    
    highFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highFrequency", highFrequency);
    lowFrequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"lowFrequency", lowFrequency);
    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highGain", highGain);
    lowGainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowGain", lowGain);
    highQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters,"highQ", highQ);
    lowQattachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowQ", lowQ);
    //Load the file
    file.onClick = [this]()
    {
        chooser = std::make_unique<juce::FileChooser> ("Select an audio file...",juce::File{},"*.wav;*.mp3");

        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file.exists())
            {
                audioProcessor.loadFile (file);
            }
        });
    };
    setSize (400, 550);
}

FiltersLabAudioProcessorEditor::~FiltersLabAudioProcessorEditor()
{
}

//==============================================================================
void FiltersLabAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));

    auto headerArea = getLocalBounds().removeFromTop(50);
                
    g.drawFittedText ("EQ", headerArea, juce::Justification::centred, 1);
}

void FiltersLabAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40); // Title area
    auto buttonArea = area.removeFromTop(40);
    //BUTTONS
    int buttonWidth = buttonArea.getWidth() / 3;
    file.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5, 5));
    play.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5, 5));
    stop.setBounds(buttonArea.reduced(5, 5));
    
    auto leftArea = area.removeFromLeft(area.getWidth() / 2).reduced(10, 0);
    auto rightArea = area.reduced(10, 0);

    //Divide in 3. FOR THE SLIDERS
    const float rowHeight = leftArea.getHeight() / 3.0f;

    highFrequency.setBounds(leftArea.removeFromTop(rowHeight).reduced(0, 10));
    
    leftArea.removeFromTop(20); // Spacer for High Gain Label
    highGain.setBounds(leftArea.removeFromTop(rowHeight).reduced(0, 10));
    
    leftArea.removeFromTop(20); // Spacer for High Q Label
    highQ.setBounds(leftArea.reduced(0));

    lowFrequency.setBounds(rightArea.removeFromTop(rowHeight).reduced(0, 10));
    
    rightArea.removeFromTop(20); // Spacer for Low Gain Label
    lowGain.setBounds(rightArea.removeFromTop(rowHeight).reduced(0, 10));
    
    rightArea.removeFromTop(20); // Spacer for Low Q Label
    lowQ.setBounds(rightArea.reduced(0));
}
