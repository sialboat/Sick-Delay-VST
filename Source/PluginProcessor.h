/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "Tempo.h"
#include "DelayLine.h"
#include "Measurement.h"
#include "Distortion.h"

//==============================================================================
/**
*/
class ClipDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ClipDelayAudioProcessor();
    ~ClipDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

//==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts {
        *this, nullptr, "Parameters", Parameters::createParameterLayout()
    };
    
    Parameters params;
    
    //using atomic variables to allow for both pluginEditor and pluginProcessor to safely read and write a variable
    Measurement levelL, levelR;
    
    juce::AudioProcessorParameter* getBypassParameter() const override;
    float processEffect(int fxIndex, float sample);
    
    void calculateAnalogDelayTime(float& currentDelayTime, float& targetDelayTime, float& sample);
    void calculateDigitalDelayTime();
    
private:
    
    DelayLine delayLineL, delayLineR;
    std::unique_ptr <Distortion> outputClipper;
    std::unique_ptr <Distortion> fxDistortion;
    
    float feedbackL = 0.0f;
    float feedbackR = 0.0f;
    
//    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    juce::dsp::StateVariableTPTFilter<float> lowCutFilter;
    juce::dsp::StateVariableTPTFilter<float> highCutFilter;
    
    float lastLowCut = -1.0f;
    float lastHighCut = -1.0f;
    
    Tempo tempo;
    
    //variables for delay time and crossfade delay mode:
    float delayInSamplesL = 0.0f;
    float delayInSamplesR = 0.0f;
    float targetDelayL = 0.0f;
    float targetDelayR = 0.0f;
    float xfadeL = 0.0f;
    float xfadeR = 0.0f;
    float xfadeInc = 0.0f;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipDelayAudioProcessor)
};
