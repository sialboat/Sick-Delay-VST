/*
  ==============================================================================

    Parameters.h
    Created: 28 Dec 2024 3:43:10pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
const juce::ParameterID gainParamID { "gain", 1 };
const juce::ParameterID delayTimeParamID {"delayTime", 1};
const juce::ParameterID mixParamID {"mix", 1};
const juce::ParameterID feedbackParamID {"feedback", 1};
const juce::ParameterID stereoParamID {"stereo", 1};
const juce::ParameterID lowCutParamID {"lowCut", 1};
const juce::ParameterID highCutParamID {"highCut", 1};
const juce::ParameterID tempoSyncParamID {"tempoSync", 1};
const juce::ParameterID delayNoteParamID {"delayNote", 1};
const juce::ParameterID bypassParamID {"bypass", 1};

const juce::ParameterID delayModeParamID {"delayMode", 1};
const juce::ParameterID spreadParamID {"spread", 1};
//const juce::ParameterID distortionDriveParamID {"drive", 1};
const juce::ParameterID fxSelectParamID {"fxType", 1};

const juce::ParameterID filterButtonParamID {"filterButton", 1};
const juce::ParameterID clipperButtonParamID {"clipperButton", 1};
const juce::ParameterID fxLocationButtonParamID {"fxLocation", 1};
const juce::ParameterID delayQualityButtonParamID {"delayQuality", 1};

const juce::ParameterID distortionSelectParamID {"distortionSelect", 1};

const juce::ParameterID tapeTubeDriveParamID {"tapeTubeDrive", 1};
const juce::ParameterID tapeTubeMixParamID {"tapeTubeMix", 1};
const juce::ParameterID tapeTubeCurveParamID {"tapeTubeCurve", 1};
const juce::ParameterID tapeTubeBiasParamID {"tapeTubeBias", 1};
const juce::ParameterID autoGainParamID {"autoGain", 1};


class Parameters
{
public:
    Parameters(juce::AudioProcessorValueTreeState& apvts);
    
    static juce	::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    void prepareToPlay(double sampleRate) noexcept;
    void reset() noexcept;
    void update() noexcept;
    void smoothen() noexcept;
    
    //this is set to 0.0f because C++ can't guarantee every declared vairable to be initially set to 0.
    float gain = 0.0f;
    float delayTime = 0.0f;
    float mix = 0.4f;
    float feedback = 0.0f;
    float spread = 0.0f;
    
    float panL = 0.0f;
    float panR = 0.0f;
    float lowCut = 20.0f;
    float highCut = 20000.0f;
    
    int delayNote = 0;
    
    int clipperMode = 0; //for clipperButton, 0 == OFF, 1 == Soft Clipping, 2 == Hard Clipping

    int fxLocation = 1; //0 = fx processes dry signal only. 1 = fx is in feedback loop. 2 = fx processes dry and wet signal
    int delayQuality = 1; //0 = linear, 1 = lagrange, 2 = cubic, 3 = hermite. This is a ComboBox
    int fxSelect = 0; //0 = distortion, 1 = modulation, 2 = time, 3 = ext.out
    
    bool filterButton = false; //false == PRE filter, TRUE == POST filter
    
    bool bypassed = false;
    int delayMode = 0; //0 == varispeed, 1 == crossfade
    
    static constexpr float minDelayTime = 5.0f;
    static constexpr float maxDelayTime = 5000.0f;
    
    bool tempoSync = false;
    
    int distortionSelect = 0; //0 = tapeTube, 1 = Odd/Even, 2 = Swell, 3 = Deci/Crush
    
    bool autoGain = false;
    float tapeTubeDrive = 0.0f;
    float tapeTubeMix = 0.0f;
    float tapeTubeCurve = 0.0f;
    float tapeTubeBias = 0.0f;
    
    juce::AudioParameterBool* tempoSyncParam;
    juce::AudioParameterBool* bypassParam;
    juce::AudioParameterBool* delayModeParam;
    juce::AudioParameterInt* delayQualityButtonParam;
    juce::AudioParameterBool* autoGainParam;
    
    juce::AudioParameterChoice* fxSelectParam;
    juce::AudioParameterInt* distortionSelectParam;
    
    juce::AudioParameterFloat* tapeTubeDriveParam;
    juce::AudioParameterFloat* tapeTubeMixParam;
    juce::AudioParameterFloat* tapeTubeCurveParam;
    juce::AudioParameterFloat* tapeTubeBiasParam;
    
    juce::LinearSmoothedValue<float> tapeTubeDriveSmoother;
    juce::LinearSmoothedValue<float> tapeTubeMixSmoother;
    juce::LinearSmoothedValue<float> tapeTubeCurveSmoother;
    juce::LinearSmoothedValue<float> tapeTubeBiasSmoother;
    
private:
    
//    juce::AudioProcessorValueTreeState& apvts_;
    
    juce::AudioParameterFloat* gainParam;
    juce::LinearSmoothedValue<float> gainSmoother;
    
    juce::AudioParameterFloat* delayTimeParam;
    
    float targetDelayTime = 0.0f; //value the 1-pole filter is trying to reach
    float coeff = 0.0f; //determines how fast the exponential smoothing happens
    
    juce::AudioParameterFloat* mixParam;
    juce::LinearSmoothedValue<float> mixSmoother;
    
    juce::AudioParameterFloat* feedbackParam;
    juce::LinearSmoothedValue<float> feedbackSmoother;
  
    juce::AudioParameterFloat* spreadParam;
    juce::LinearSmoothedValue<float> spreadSmoother;
    
    juce::AudioParameterFloat* stereoParam;
    juce::LinearSmoothedValue<float> stereoSmoother;
    
    juce::AudioParameterFloat* lowCutParam;
    juce::LinearSmoothedValue<float> lowCutSmoother;
    
    juce::AudioParameterFloat* highCutParam;
    juce::LinearSmoothedValue<float> highCutSmoother;
    
    juce::AudioParameterChoice* delayNoteParam;
    
    juce::AudioParameterBool* filterButtonParam;
    
    juce::AudioParameterInt* clipperButtonParam;
    juce::AudioParameterInt* fxLocationButtonParam;
    
//    juce::AudioParameterFloat* fxAmountParam;
//    juce::LinearSmoothedValue<float> fxAmountSmoother;
    
    //need params: pre/post button
    //need params: soft hard no clip button
    //need params: pitch shifting stuff (so it's bipolar)
    //need params: listener and knobs for every distortion
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parameters)
};
