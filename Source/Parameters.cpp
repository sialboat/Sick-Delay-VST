/*
  ==============================================================================

    Parameters.cpp
    Created: 28 Dec 2024 3:43:10pm
    Author:  Silas Wang

  ==============================================================================
*/

#include "Parameters.h"
#include "PluginEditor.h"
#include "DSP.h"

template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
                          const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);  // parameter does not exist or wrong type
}

//converts a parameter time value into text, storing it in a juce::String object and returning it
//to the caller.
static juce::String stringFromMilliseconds(float value, int)
{
    if (value < 10.0f) {
        return juce::String(value, 2) + " ms";
    } else if (value < 100.0f) {
        return juce::String(value, 1) + " ms";
    } else if (value < 1000.0f) {
        return juce::String(int(value)) + " ms";
    } else {
        return juce::String(value * 0.001f, 2) + " s";
    }
}

static juce::String stringFromDecibels(float value, int)
{
    return juce::String(value, 1) + " dB";
}

static juce::String stringFromSemitones(float value, int)
{
    return juce::String(value, 1) + " st";
}

static juce::String stringFromPercent(float value, int)
{
    return juce::String(int(value)) + " %";
}

//converts milliseconds float value to a juce::String value
static float millisecondsFromString(const juce::String& text)
{
    float value = text.getFloatValue();
    if(!text.endsWithIgnoreCase("ms")) {
        if(text.endsWithIgnoreCase("s") || value < Parameters::minDelayTime) {
            return value * 1000.0f;
        }
    }
    return value;
}

static juce::String stringFromHz(float value, int)
{
    if(value < 1000.0f) {
        return juce::String(int(value)) + "Hz";
    } else if (value < 10000.0f) {
        return juce::String(value / 1000.0f, 2) + " kHz";
    } else {
        return juce::String(value / 1000.0f, 1) + " kHz";
    }
}
static float hzFromString(const juce::String& str)
{
    float value = str.getFloatValue();
    if(value < 20.0f) {
        return value * 1000.0f;
    }
    return value;
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, delayTimeParamID, delayTimeParam);
    castParameter(apvts, mixParamID, mixParam);
    castParameter(apvts, feedbackParamID, feedbackParam);
    castParameter(apvts, spreadParamID, spreadParam);
    castParameter(apvts, stereoParamID, stereoParam);
    castParameter(apvts, lowCutParamID, lowCutParam);
    castParameter(apvts, highCutParamID, highCutParam);
    castParameter(apvts, tempoSyncParamID, tempoSyncParam);
    castParameter(apvts, delayNoteParamID, delayNoteParam);
    castParameter(apvts, bypassParamID, bypassParam);
//    castParameter(apvts, distortionDriveParamID, distortionDriveParam);
    castParameter(apvts, delayModeParamID, delayModeParam);
    castParameter(apvts, filterButtonParamID, filterButtonParam);
    castParameter(apvts, clipperButtonParamID, clipperButtonParam);
    
    castParameter(apvts, autoGainParamID, autoGainParam);
    castParameter(apvts, softClipDriveParamID, softClipDriveParam);
    castParameter(apvts, softClipMixParamID, softClipMixParam);
    
//    castParameter(apvts, fxLocationButtonParamID, fxLocationButtonParam);
    castParameter(apvts, delayQualityButtonParamID, delayQualityButtonParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       gainParamID,
       "Output Gain",
       juce::NormalisableRange<float> {-12.0f, 12.0f},
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
       ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        delayTimeParamID,
        "Delay Time",
        juce::NormalisableRange<float> { minDelayTime, maxDelayTime, 0.001f, 0.25f },
       100.0f,
            juce::AudioParameterFloatAttributes()
               .withStringFromValueFunction(stringFromMilliseconds)
               .withValueFromStringFunction(millisecondsFromString)
        ));
        
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       mixParamID,
       "Mix",
       juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f),
       40.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
       ));
    
    //negative feedback will invert its polarity alongside inserting feedback
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       feedbackParamID,
       "Feedback",
       juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
       ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       spreadParamID,
       "Spread",
       juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
       ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       stereoParamID,
       "Stereo",
       juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
   ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       lowCutParamID,
       "Low Cut",
       juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
       20.0f,
       juce::AudioParameterFloatAttributes()
           .withStringFromValueFunction(stringFromHz)
           .withValueFromStringFunction(hzFromString)
   ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       highCutParamID,
       "High Cut",
       juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
       20000.0f,
       juce::AudioParameterFloatAttributes()
           .withStringFromValueFunction(stringFromHz)
           .withValueFromStringFunction(hzFromString)
   ));
    
    juce::StringArray noteLengths = {
        "1/32", "1/16 trip", "1/32 dot", "1/16", "1/8 trip", "1/16 dot", "1/8", "1/4 trip",
        "1/8 dot", "1/4", "1/2 trip", "1/4 dot", "1/2", "1/1 trip", "1/2 dot", "1/1"
    };
    
    layout.add(std::make_unique<juce::AudioParameterBool>(tempoSyncParamID, "Tempo Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(delayNoteParamID, "Delay Note", noteLengths, 9));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(bypassParamID, "Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(delayModeParamID, "Delay Mode", false));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(clipperButtonParamID, "Clipper", 0, 2, 0));
    
    
    juce::StringArray fxTypes = {
      "Off", "Soft Clip", "Hard Clip", "Inflator", "Bitcrusher", "CWO", "Pitch Shifter", "Chorus", "Ext. Out"
    };
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(fxSelectParamID, "FX", fxTypes, 0));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(filterButtonParamID, "Filter Pre/Post", false));
    
    //=========
    //soft clip
    //=========
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       softClipMixParamID,
       "Mix",
       juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
       40.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       softClipDriveParamID,
       "Drive",
       juce::NormalisableRange<float>{0.0f, 24.0f},
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
       ));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(autoGainParamID, "Auto Gain", false));
    
    
    //output buttons
    layout.add(std::make_unique<juce::AudioParameterInt>(delayQualityButtonParamID, "Interpolation", 0, 3, 1));
    
    
    return layout;
}


void Parameters::prepareToPlay(double sampleRate) noexcept
{
    double duration = 0.05;
    gainSmoother.reset(sampleRate, duration);
    
    //to adjust how fast the delay time reacts, change the number multiplied by the float
    //to a value that is smaller (to make it faster) and visa versa.
    
    coeff = 1.0f - std::exp(-1.0f / (0.2f * float(sampleRate)));
    
    mixSmoother.reset(sampleRate, duration);
    
    feedbackSmoother.reset(sampleRate, duration);
    
    spreadSmoother.reset(sampleRate, duration);
    
    stereoSmoother.reset(sampleRate, duration);
    
    lowCutSmoother.reset(sampleRate, duration);
    highCutSmoother.reset(sampleRate, duration);
    
    softClipMixSmoother.reset(sampleRate, duration);
    softClipDriveSmoother.reset(sampleRate, duration);
//    distortionDriveSmoother.reset(sampleRate, duration);
}

void Parameters::reset() noexcept
{
    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    
    delayTime = 0.0f;
    
    mix = 0.4f;
    mixSmoother.setCurrentAndTargetValue(mixParam->get() * 0.01f);
    
    feedback = 0.0f;
    feedbackSmoother.setCurrentAndTargetValue(feedbackParam->get() * 0.01f);
    
    spread = 0.0f;
    spreadSmoother.setCurrentAndTargetValue(spreadParam->get() * 0.01f);
    
    panL = 0.0f;
    panR = 1.0f;
    
    stereoSmoother.setCurrentAndTargetValue(stereoParam->get() * 0.01f);
    
    lowCut = 20.0f;
    lowCutSmoother.setCurrentAndTargetValue(lowCutParam->get());
    
    highCut = 20000.0f;
    highCutSmoother.setCurrentAndTargetValue(highCutParam->get());
    
    softClipDrive = 0.0f;
    softClipDriveSmoother.setCurrentAndTargetValue(softClipDriveParam->get());
    softClipMix = 0.0f;
    softClipMixSmoother.setCurrentAndTargetValue(softClipMixParam->get());
//    distortionDriveSmoother.setCurrentAndTargetValue(distortionDriveParam->get());
    
    autoGain = false;
    filterButton = false;
    delayMode = false;
}

void Parameters::update() noexcept
{
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    
    targetDelayTime = delayTimeParam->get();
    if(delayTime == 0.0f) {
        delayTime = targetDelayTime;
    }
    
    mixSmoother.setTargetValue(mixParam->get() * 0.01f);
    
    feedbackSmoother.setTargetValue(feedbackParam->get() * 0.01f);
    
    stereoSmoother.setTargetValue(stereoParam->get() * 0.01f);
    
    lowCutSmoother.setTargetValue(lowCutParam->get());
    highCutSmoother.setTargetValue(highCutParam->get());
    
    delayNote = delayNoteParam->getIndex();
    tempoSync = tempoSyncParam->get();
    
    bypassed = bypassParam->get();
    
//    distortionDriveSmoother.setTargetValue(distortionDriveParam->get());
    
    spreadSmoother.setTargetValue(spreadParam->get());
    
    delayMode = delayModeParam->get();
    filterButton = filterButtonParam->get();
    
    clipperMode = clipperButtonParam->get();
    
    softClipDriveSmoother.setTargetValue(juce::Decibels::decibelsToGain(softClipDriveParam->get()));
    softClipMixSmoother.setTargetValue(softClipMixParam->get() * 0.01f);
    autoGain = autoGainParam->get();
//    delayMode = apvts.getRawParameterValue(delayModeParamID)->load();
}

void Parameters::smoothen() noexcept
{
    gain = gainSmoother.getNextValue();
    
    delayTime += (targetDelayTime - delayTime) * coeff;
    
    mix = mixSmoother.getNextValue();
    
    feedback = feedbackSmoother.getNextValue();
    
    spread = spreadSmoother.getNextValue();
    
    panningEqualPower(stereoSmoother.getNextValue(), panL, panR);
    
    lowCut = lowCutSmoother.getNextValue();
    highCut = highCutSmoother.getNextValue();
    
    softClipDrive = softClipDriveSmoother.getNextValue();
    softClipMix = softClipMixSmoother.getNextValue();
//    distortionDrive = distortionDriveSmoother.getNextValue();
}
