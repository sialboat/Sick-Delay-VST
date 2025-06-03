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
#include "Distortion.h"

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
static juce::String stringFromPercentOddEven(float value, int)
{
    //at 0 (12:00), 50% odd, 50% even
    //at -100, we have 100% odd, 0% even
    //at 100, we have 0% odd, 100% even
    //value > 0, more even
    float odd = 100.0f - value;
    float even = value;
    
    return juce::String((int) odd) + " / " + juce::String((int) even) + " %";
    
}
static juce::String stringFromPercentTapeTube(float value, int)
{
    //at value == 100, we have 0/100
    //at value == 0, we have 100/0
    float tape = 100.0f - value;
    float tube = value;
    
    return juce::String((int) tape) + " / " + juce::String((int) tube) + " %";
}

static juce::String stringFromPercentFloat(float value, int)
{
    return juce::String(value, 2) + " %";
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

    castParameter(apvts, delayModeParamID, delayModeParam);
    castParameter(apvts, filterButtonParamID, filterButtonParam);
    castParameter(apvts, clipperButtonParamID, clipperButtonParam);
    
    castParameter(apvts, autoGainParamID, autoGainParam);
    castParameter(apvts, fxSelectParamID, fxSelectParam);
    
    castParameter(apvts, tapeTubeDriveParamID, tapeTubeDriveParam);
    castParameter(apvts, tapeTubeMixParamID, tapeTubeMixParam);
    castParameter(apvts, tapeTubeCurveParamID, tapeTubeCurveParam);
    castParameter(apvts, tapeTubeBiasParamID, tapeTubeBiasParam);
    
    castParameter(apvts, oddEvenDriveParamID, oddEvenDriveParam);
    castParameter(apvts, oddEvenMixParamID, oddEvenMixParam);
    castParameter(apvts, oddEvenCurveParamID, oddEvenCurveParam);
    castParameter(apvts, oddEvenBiasParamID, oddEvenBiasParam);
    
}

//only do this for automatable parameters
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
    
//    juce::StringArray fxTypes = {
//        "Tape/Tube", "Odd/Even", "Swell", "Deci/Crush", "CWO", "Pitch", "Chorus/Flanger"
//    };
    
    //Distortion (0-*): Tape/Tube (0-0), Odd/Even (0-1), Swell (0-2), Deci/Crush (0-3)
    //Modulation (1-*): FM/AM/RM (1-0), Pitch (1-1), CWO (1-2)
    //Time (2-*): Reverb (2-0), Chorus (2-1), Flanger(2-2), Phaser(2-3) Disperser (2-4)
    //Ext. Out (3-0)
    juce::StringArray fxTypes = {
        "Bypass", "Distortion", "Modulation", "Time", "Ext. Out"
    };
//    juce::StringArray fxTypes = {
//        "Tape/Tube", "Odd/Even", "Swell", "Deci/Crush", "Convolution", "FM/AM/RM", "Reverb", "CWO", "Pitch", "Chorus/Flanger", "Ext. Out"
//    };
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(fxSelectParamID, "FX", fxTypes, 0));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(filterButtonParamID, "Filter Pre/Post", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(autoGainParamID, "Auto Gain", false));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(distortionSelectParamID, "Distortion", 0, 3, 0));
    
    
    //=========
    // tapeTube
    //=========
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       tapeTubeMixParamID,
       "Mix",
       juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
       100.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       tapeTubeDriveParamID,
       "Drive",
       juce::NormalisableRange<float>{0.0f, 24.0f},
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
       ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       tapeTubeCurveParamID,
       "Tape/Tube",
       juce::NormalisableRange<float>{0.0f, 100.0f},
       50.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercentTapeTube)
       ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       tapeTubeBiasParamID,
       "Bias",
       juce::NormalisableRange<float>{-100.0f, 100.0f},
       0.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
       ));
    
    // ========
    // oddEven
    // ========
    layout.add(std::make_unique<juce::AudioParameterFloat> (
        oddEvenDriveParamID,
        "Drive",
        juce::NormalisableRange<float> {0.0f, 24.0f},
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)));
    layout.add(std::make_unique<juce::AudioParameterFloat>  (
         oddEvenMixParamID,
         "Mix",
         juce::NormalisableRange<float> {0.0f, 100.0f, 1.0f},
         100.0f,
         juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       oddEvenCurveParamID,
       "Odd/Even",
       juce::NormalisableRange<float> {0.0f, 100.0f, 1.0f},
       50.0f,
       juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercentOddEven)));
    layout.add(std::make_unique<juce::AudioParameterFloat> (
        oddEvenBiasParamID,
        "Bias",
        juce::NormalisableRange<float> {-100.0f, 100.0f},
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)));
    
    
//    layout.add(std::make_unique<juce::AudioParameterFloat>(
//                                                           tapeTubeCurveParamID, ""))
    
    
    //output buttons
//    layout.add(std::make_unique<juce::AudioParameterInt>(delayQualityButtonParamID, "Interpolation", 0, 3, 1));
    
    
    return layout;
}


void Parameters::prepareToPlay(double sampleRate) noexcept
{
    double duration = 0.05;
    fxSelect = 0;
    gainSmoother.reset(sampleRate, duration);
    
    //to adjust how fast the delay time reacts, change the number multiplied by the float
    //to a value that is smaller (to make it faster) and visa versa.
    
    coeff = 1.0f - std::exp(-1.0f / (0.2f * float(sampleRate)));
    
    mixSmoother.reset(sampleRate, duration);
//    fxSelectParam.reset(sampleRate, duration);
    
    feedbackSmoother.reset(sampleRate, duration);
    
    spreadSmoother.reset(sampleRate, duration);
    
    stereoSmoother.reset(sampleRate, duration);
    
    lowCutSmoother.reset(sampleRate, duration);
    highCutSmoother.reset(sampleRate, duration);
    
    tapeTubeMixSmoother.reset(sampleRate, duration);
    tapeTubeDriveSmoother.reset(sampleRate, duration);
    tapeTubeBiasSmoother.reset(sampleRate, duration);
    tapeTubeCurveSmoother.reset(sampleRate, duration);
    
    oddEvenDriveSmoother.reset(sampleRate, duration);
    oddEvenMixSmoother.reset(sampleRate, duration);
    oddEvenBiasSmoother.reset(sampleRate, duration);
    oddEvenCurveSmoother.reset(sampleRate, duration);
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
    
    fxSelect = 0;
    
    lowCut = 20.0f;
    lowCutSmoother.setCurrentAndTargetValue(lowCutParam->get());
    
    highCut = 20000.0f;
    highCutSmoother.setCurrentAndTargetValue(highCutParam->get());
    
    //based on what mode we are in, we should have a different set of parameters set the distortion controls
    //where does that belong? 
    
    tapeTubeDrive = 0.0f;
    tapeTubeDriveSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(tapeTubeDriveParam->get()));
    tapeTubeMix = 1.0f;
    tapeTubeMixSmoother.setCurrentAndTargetValue(tapeTubeMixParam->get());
    tapeTubeBias = 0.0f;
    tapeTubeCurve = 0.0f;
    tapeTubeBiasSmoother.setCurrentAndTargetValue(tapeTubeBiasParam->get());
    tapeTubeCurveSmoother.setCurrentAndTargetValue(tapeTubeCurveParam->get());
    
    oddEvenDrive = 0.0f;
    oddEvenDriveSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(oddEvenDriveParam->get()));
    oddEvenMix = 1.0f;
    oddEvenMixSmoother.setCurrentAndTargetValue(oddEvenMixParam->get());
    oddEvenBias = 0.0f;
    oddEvenBiasSmoother.setCurrentAndTargetValue(oddEvenBiasParam->get());
    oddEvenCurve = 0.0f;
    oddEvenCurveSmoother.setCurrentAndTargetValue(oddEvenCurveParam->get());
//    distortionDriveSmoother.setCurrentAndTargetValue(distortionDriveParam->get());
    
    autoGain = false;
    filterButton = false;
    delayMode = 0;
    distortionSelect = 0;
}

void Parameters::update() noexcept
{
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    
    targetDelayTime = delayTimeParam->get();
    if(delayTime == 0.0f) {
        delayTime = targetDelayTime;
    }
    
    fxSelect = fxSelectParam->getIndex();
    
    mixSmoother.setTargetValue(mixParam->get() * 0.01f);
    
    feedbackSmoother.setTargetValue(feedbackParam->get() * 0.01f);
    
    stereoSmoother.setTargetValue(stereoParam->get() * 0.01f);
    
    lowCutSmoother.setTargetValue(lowCutParam->get());
    highCutSmoother.setTargetValue(highCutParam->get());
    
    delayNote = delayNoteParam->getIndex();
    tempoSync = tempoSyncParam->get();
    
    bypassed = bypassParam->get();
    
//    distortionDriveSmoother.setTargetValue(distortionDriveParam->get());
//    spreadSmoother.setTargetValue(spreadParam->get());
    spreadSmoother.setTargetValue(spreadParam->get());
//    spread = spreadSmoother.getNextValue();
    
    delayMode = delayModeParam->get();
    filterButton = filterButtonParam->get();
    
    clipperMode = clipperButtonParam->get();
    
    
    autoGain = autoGainParam->get();

    tapeTubeDriveSmoother.setTargetValue(juce::Decibels::decibelsToGain(tapeTubeDriveParam->get()));
    tapeTubeMixSmoother.setTargetValue(tapeTubeMixParam->get() * 0.01f);
    tapeTubeCurveSmoother.setTargetValue(tapeTubeCurveParam->get() * 0.01f);
    tapeTubeBiasSmoother.setTargetValue(tapeTubeBiasParam->get() * 0.01f);
    
    oddEvenDriveSmoother.setTargetValue(juce::Decibels::decibelsToGain(oddEvenDriveParam->get()));
    oddEvenMixSmoother.setTargetValue(oddEvenMixParam->get() * 0.01f);
    oddEvenCurveSmoother.setTargetValue(oddEvenCurveParam->get() * 0.01f);
    oddEvenBiasSmoother.setTargetValue(oddEvenBiasParam->get() * 0.01f);
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
    
    tapeTubeMix = tapeTubeMixSmoother.getNextValue();
    tapeTubeBias = tapeTubeBiasSmoother.getNextValue();
    tapeTubeCurve = tapeTubeCurveSmoother.getNextValue();
    tapeTubeDrive = tapeTubeDriveSmoother.getNextValue();
    
    oddEvenMix = oddEvenMixSmoother.getNextValue();
    oddEvenBias = oddEvenBiasSmoother.getNextValue();
    oddEvenCurve = oddEvenCurveSmoother.getNextValue();
    oddEvenDrive = oddEvenDriveSmoother.getNextValue();
//    distortionDrive = distortionDriveSmoother.getNextValue();
}
