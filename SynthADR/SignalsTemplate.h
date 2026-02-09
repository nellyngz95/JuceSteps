/*
  ==============================================================================

    Signals.h
    Created: 8 Feb 2026 8:18:19pm
    Author:  Nelly Victoria Alexandra Garcia Sihuay

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>

//Write your Synthesiser Sound class



class VoiceBase : public juce::SynthesiserVoice
{
public:
    VoiceBase() {}
   //create a variable for your selection of signals
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return dynamic_cast<MySignal*> (sound) != nullptr;
    }
    
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override {
       //How would you start the note?
        adsr.noteOn();
    }

    void stopNote(float, bool allowTailOff) override {
        adsr.noteOff();
        if (!allowTailOff || !adsr.isActive()) clearCurrentNote();
    }

    virtual float getNextSample() = 0;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        while (--numSamples >= 0) {
            auto rawSample = getNextSample();
            auto env = adsr.getNextSample();
            auto finalSample = rawSample * env * level;

            for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                outputBuffer.addSample(i, startSample, finalSample);

            startSample++;
            
            if (!adsr.isActive()) clearCurrentNote();
        }
    }
    
    void setSampleRate(double newSampleRate) {
        adsr.setSampleRate(newSampleRate);
    }

    void updateAdsr(float a, float d, float s, float r,double sr,int oscChoice) {
       //ask for the parameters..
        adsr.setSampleRate(sr);
        adsr.setParameters(adsrParams);
    }

protected:
    float phase = 0.0f, phaseDelta = 0.0f, level = 0.0f;
   //adsr object
     //parameter
};



class SineVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
            return oscSelection == 0 && dynamic_cast<MySignal*>(sound) != nullptr;
        }
    float getNextSample() override {
       //create your own sine signal.
        return sample;
    }
   
};

class SawVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
            return oscSelection == 1 && dynamic_cast<MySignal*>(sound) != nullptr;
        }
    float getNextSample() override {
       //create your sawtooth signal
        return sample;
    }
};

class SquareVoice : public VoiceBase {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
            return oscSelection == 2 && dynamic_cast<MySignal*>(sound) != nullptr;
        }
    float getNextSample() override {
        //Create your square signal
        return sample;
    }
};
