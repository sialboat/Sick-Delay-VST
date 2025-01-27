/*
  ==============================================================================

    RotaryKnob.h
    Created: 30 Dec 2024 2:18:31pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class RotaryKnob  : public juce::Component
{
public:
    RotaryKnob(const juce::String& text, juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& parameterID, bool drawFromMiddle = false,
               int width = 70, int knobHeight = 86, int textHeight = 24);
    ~RotaryKnob() override;

    void resized() override;
    
    juce::Slider slider;
    juce::Label label;
    
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    

private:
    int knobHeight = 0;
    int textHeight = 0;
    int width = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotaryKnob)
};
