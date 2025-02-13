/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"
#include "LevelMeter.h"
#include "BypassGradient.h"

//==============================================================================
/**
*/

class MultiStateButtonAttachment : private juce::Button::Listener
{
public:
//    ThreeStateButtonAttachment(juce::AudioProcessorValueTreeState& apvts,
//                               const juce::String& parameterID, juce::TextButton button_)
//    : state(apvts), paramID(parameterID), button(button_)
//    {
//
//    }
    
    MultiStateButtonAttachment(juce::AudioProcessorValueTreeState& apvts_,
                               const juce::String& parameterID, juce::Button& buttonToUse)
                                : apvts(apvts_), param(*apvts.getParameter(parameterID)), button(buttonToUse)
    {
        button.addListener(this);
        updateButtonState();
        
        auto* intParam = dynamic_cast<juce::AudioParameterInt*>(&param);
        if (intParam && intParam->get() != 0)
        {
            // Force the parameter to start at 0 if it isn't already
            intParam->setValueNotifyingHost(0);
        }
    }
    
    ~MultiStateButtonAttachment() override {
        button.removeListener(this);
    }
    
    void buttonClicked(juce::Button*) override
    {
        auto intParam = dynamic_cast<juce::AudioParameterInt*>(&param);
        if(intParam)
        {
//            auto currentValue = param.getValue();
            int currentValue = intParam->get();
            int newValue = (currentValue + 1) % size;
            intParam->setValueNotifyingHost(newValue / 2.0f);
        }
    }
    
    void buttonStateChanged(juce::Button*) override {
    }
    
    void parameterValueChanged(int, float newValue)
    {
        juce::MessageManager::callAsync([this, newValue]()
        {
            updateButtonState();
        });
    }
    
    void setSize(int s) {size = s; }
                                
//    : apvts(apvts_), paramID(parameterID), button(buttonToUse)
private:
    void updateButtonState()
    {
        auto* intParam = dynamic_cast<juce::AudioParameterInt*>(&param);
        if (intParam)
        {
            auto currentValue = static_cast<int>(param.getValue() * 2.0f);
            button.setToggleState(currentValue > 0, juce::dontSendNotification);
//            button.setButtonText(juce::String(currentValue));
        }
    }
    
    juce::AudioProcessorValueTreeState& apvts;
    juce::RangedAudioParameter& param; //this helps manage a parameter that is gridlocked by a range of values
    juce::Button& button;
    
    int size = 3; //default value is 3
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiStateButtonAttachment)
};

//==============================================================================

// private thing is to let the processor editor know when to make things visible / invisible.
class ClipDelayAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::AudioProcessorParameter::Listener
{
public:
    ClipDelayAudioProcessorEditor (ClipDelayAudioProcessor&);
    ~ClipDelayAudioProcessorEditor() override;
    
    enum RadioButtonIds
    {
        filterButtons = 1001
    };
    
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    void parameterValueChanged(int, float) override;
    void parameterGestureChanged(int, bool) override { }
    void updateDelayKnobs(bool tempoSyncActive);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ClipDelayAudioProcessor& audioProcessor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipDelayAudioProcessorEditor)
    
    //70 86 24 is default, 105 129 36 is 1.5x higher
    //88 108 30 is 1.25x higher,
    RotaryKnob gainKnob {"Gain", audioProcessor.apvts, gainParamID, true};
    RotaryKnob mixKnob {"Mix", audioProcessor.apvts, mixParamID};
    RotaryKnob delayTimeKnob {"Delay Time", audioProcessor.apvts, delayTimeParamID};
    RotaryKnob spreadKnob {"Spread", audioProcessor.apvts, spreadParamID, true};
//    RotaryKnob feedbackKnob {"Feedback", audioProcessor.apvts, feedbackParamID, true, 105, 129, 36};
//    RotaryKnob stereoKnob {"Stereo", audioProcessor.apvts, stereoParamID, true, 105, 129, 36};
//    RotaryKnob lowCutKnob {"Low Cut", audioProcessor.apvts, lowCutParamID, false, 105, 129, 36}; //
//    RotaryKnob highCutKnob {"High Cut", audioProcessor.apvts, highCutParamID, false, 105, 129, 36};
    RotaryKnob feedbackKnob {"Feedback", audioProcessor.apvts, feedbackParamID, true};
    RotaryKnob stereoKnob {"Stereo", audioProcessor.apvts, stereoParamID, true};
    RotaryKnob lowCutKnob {"Low Cut", audioProcessor.apvts, lowCutParamID, false}; //
    RotaryKnob highCutKnob {"High Cut", audioProcessor.apvts, highCutParamID, false};
    
    RotaryKnob delayNoteKnob {"Note", audioProcessor.apvts, delayNoteParamID};
    RotaryKnob fxSelectKnob {"Mode", audioProcessor.apvts, fxSelectParamID, false, 88, 108, 36};
    
    RotaryKnob softClipDriveKnob {"Drive", audioProcessor.apvts, softClipDriveParamID};
    RotaryKnob softClipMixKnob {"Mix", audioProcessor.apvts, softClipMixParamID};
//    RotaryKnob softClipKnob {"Drive (soft clip)", audioProcessor.apvts, softClipParamID};
//    RotaryKnob softClipMixKnob {"Mix (soft clip)", audioProcessor.apvts, }
//    RotaryKnob fxSelectKnob {"Mode", audioProcessor.apvts, fxSelectParamID}; //false, 88, 108, 36
    
//    RotaryKnob fxAmountKnob {"Amount", audioProcessor.apvts, fxDriveParamID}; //false, 88, 108, 36
    
    juce::TextButton tempoSyncButton;
    juce::TextButton delayModeButton;
    juce::TextButton autoGainButton;
    
    juce::TextButton preFXButton;
    juce::TextButton postFXButton;
    
    juce::TextButton clipperButton;
    int clipperButtonIndex = 0;
    juce::StringArray clipperText {"N/A", "Soft Clip", "Hard Clip"};
    MultiStateButtonAttachment clipperAttachment {
        audioProcessor.apvts, clipperButtonParamID.getParamID(), clipperButton
    };
    
    juce::ComboBox delayQualityBox;
    juce::TextButton fxLocationButton;
    int fxLocationIndex = 0;
    
//    juce::StringArray fxLocation = {"Dry", "Wet", "Dry+Wet"};
//    MultiStateButtonAttachment locationAttachment {
//        audioProcessor.apvts, fxLocationButtonParamID.getParamID(), fxLocationButton
//    };
    
    
//    juce::TextButton delayQualityButton;
//    int delayQualityButtonIndex = 1; //default is lagrange
//    juce::StringArray quality {"Linear", "Lagrange", "Hermite", "Cubic"};
    
    BypassGradient bypassGradient;
    
    juce::ImageButton bypassButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttachment {
        audioProcessor.apvts, bypassParamID.getParamID(), bypassButton
    };
    
    juce::AudioProcessorValueTreeState::ButtonAttachment tempoSyncAttachment {
        audioProcessor.apvts, tempoSyncParamID.getParamID(), tempoSyncButton
    };
    
    juce::GroupComponent delayGroup, feedbackGroup, outputGroup, fxGroup;
    
    juce::ComboBox comboBox;
    
    juce::AudioProcessorValueTreeState::ComboBoxAttachment comboBoxAttachment {
        audioProcessor.apvts, delayModeParamID.getParamID(), comboBox
    };
    
    juce::AudioProcessorValueTreeState::ButtonAttachment filterButtonAttachment {
        audioProcessor.apvts, filterButtonParamID.getParamID(), preFXButton
    };
    
    MainLookAndFeel mainLF;
    
    LevelMeter meter;
};
