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
class ClipDelayAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::AudioProcessorParameter::Listener
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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    void parameterValueChanged(int, float) override;
    void parameterGestureChanged(int, bool) override { }
    void updateDelayKnobs(bool tempoSyncActive);
    void updateFxKnobs(int fxIndex, float val, bool flag);
    void updateDistortionFx(int distIndex, bool flag);
    void updateModulationFx(int modIndex, bool flag);
    void updateTimeFx(int timeIndex, bool flag);
    
    void setTapeTubeVisibility(bool flag)
    {
        tapeTubeDriveKnob.setVisible(flag);
        tapeTubeMixKnob.setVisible(flag);
        tapeTubeBiasKnob.setVisible(flag);
        tapeTubeCurveKnob.setVisible(flag);
    };
    
    void setOddEvenVisibility(bool flag)
    {
        oddEvenDriveKnob.setVisible(flag);
        oddEvenMixKnob.setVisible(flag);
        oddEvenBiasKnob.setVisible(flag);
        oddEvenCurveKnob.setVisible(flag);
    };
    
    void setupTextButton(juce::TextButton& button, const juce::String& name, const std::vector<int>& textButtonBounds,
                         bool canClick, juce::LookAndFeel* lookAndFeel) {
        button.setButtonText(name);
        button.setBounds(textButtonBounds.at(0), textButtonBounds.at(1), textButtonBounds.at(2), textButtonBounds.at(3));
        button.setClickingTogglesState(canClick);
        button.setLookAndFeel(lookAndFeel);
//        group.addAndMakeVisible(button);
    }
    
    void setupGroups(juce::GroupComponent& group, const juce::String& name, juce::Justification justification, const std::vector<juce::Component*>& items, const std::vector<juce::Component*>& children = {})
    {
        group.setName(name);
        group.setTextLabelPosition(justification);
        for(juce::Component* c : items)
            group.addAndMakeVisible(c);
        for(juce::Component* c : children)
            group.addChildComponent(c);
    }
    
    void setupComboBox(juce::ComboBox& comboBox, const juce::String& name, const juce::StringArray& items, const std::vector<int>& itemIds,
                       bool enabled, int selectedId, juce::LookAndFeel* lookAndFeel,
                       const std::vector<int>& comboButtonBounds = {0, 0, 70, 27})
    {
        comboBox.setName(name);
        comboBox.setLookAndFeel(lookAndFeel);
        comboBox.setEnabled(enabled);
        
        int itemCount = 0;
            comboBox.setBounds(comboButtonBounds.at(0), comboButtonBounds.at(1),
                               comboButtonBounds.at(2), comboButtonBounds.at(3));
        for(int i = 0; i < items.size(); i++) {
            if(items[i].substring(0, 3) == "SEP") {
                comboBox.addSeparator();
                comboBox.addSectionHeading(items[i].substring(3));
            } else {
                comboBox.addItem(items[i], itemIds[(int)itemCount]);
                itemCount++;
            }
        }
        
        comboBox.setSelectedId(selectedId);
//        group.addAndMakeVisible(comboBox);
    }
    
    
    ClipDelayAudioProcessor& audioProcessor;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipDelayAudioProcessorEditor)
    
    //70 86 24 is default, 105 129 36 is 1.5x higher
    //88 108 30 is 1.25x higher,
    //    RotaryKnob feedbackKnob {"Feedback", audioProcessor.apvts, feedbackParamID, true, 105, 129, 36};
    //    RotaryKnob stereoKnob {"Stereo", audioProcessor.apvts, stereoParamID, true, 105, 129, 36};
    //    RotaryKnob lowCutKnob {"Low Cut", audioProcessor.apvts, lowCutParamID, false, 105, 129, 36}; //
    //    RotaryKnob highCutKnob {"High Cut", audioProcessor.apvts, highCutParamID, false, 105, 129, 36};
    
    RotaryKnob gainKnob {"Gain", audioProcessor.apvts, gainParamID, true};
    RotaryKnob mixKnob {"Mix", audioProcessor.apvts, mixParamID};
    RotaryKnob delayTimeKnob {"Delay Time", audioProcessor.apvts, delayTimeParamID};
    RotaryKnob spreadKnob {"Spread", audioProcessor.apvts, spreadParamID, true};
    RotaryKnob feedbackKnob {"Feedback", audioProcessor.apvts, feedbackParamID, true};
    RotaryKnob stereoKnob {"Stereo", audioProcessor.apvts, stereoParamID, true};
    RotaryKnob lowCutKnob {"Low Cut", audioProcessor.apvts, lowCutParamID, false}; //
    RotaryKnob highCutKnob {"High Cut", audioProcessor.apvts, highCutParamID, false};
    
    RotaryKnob delayNoteKnob {"Note", audioProcessor.apvts, delayNoteParamID};
    
    RotaryKnob tapeTubeDriveKnob {"Drive", audioProcessor.apvts, tapeTubeDriveParamID};
    RotaryKnob tapeTubeMixKnob {"Mix", audioProcessor.apvts, tapeTubeMixParamID};
    RotaryKnob tapeTubeCurveKnob {"Tape/Tube", audioProcessor.apvts, tapeTubeCurveParamID, true};
    RotaryKnob tapeTubeBiasKnob {"Bias", audioProcessor.apvts, tapeTubeBiasParamID, true};
    
    RotaryKnob oddEvenDriveKnob {"Drive", audioProcessor.apvts, oddEvenDriveParamID};
    RotaryKnob oddEvenMixKnob {"Mix", audioProcessor.apvts, oddEvenMixParamID};
    RotaryKnob oddEvenCurveKnob {"Odd/Even", audioProcessor.apvts, oddEvenCurveParamID, true};
    RotaryKnob oddEvenBiasKnob {"Bias", audioProcessor.apvts, oddEvenBiasParamID, true};
    
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
    juce::ComboBox fxSelectBox;
    int fxSelectIndex = 0;
    juce::TextButton fxLocationButton;
    int fxLocationIndex = 0;
    
    juce::ComboBox distortionSelectBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment distortionSelectAttachment {
        audioProcessor.apvts, distortionSelectParamID.getParamID(), distortionSelectBox
    };
    
    juce::ComboBox modulationSelectBox;
    
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
    
    juce::ComboBox delayMode;
    
    juce::AudioProcessorValueTreeState::ComboBoxAttachment delayModeAttachment {
        audioProcessor.apvts, delayModeParamID.getParamID(), delayMode
    };
    
    juce::AudioProcessorValueTreeState::ButtonAttachment filterButtonAttachment {
        audioProcessor.apvts, filterButtonParamID.getParamID(), preFXButton
    };
    
    MainLookAndFeel mainLF;
    
    LevelMeter meter;
};
