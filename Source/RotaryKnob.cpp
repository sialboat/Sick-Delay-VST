/*
  ==============================================================================

    RotaryKnob.cpp
    Created: 30 Dec 2024 2:18:31pm
    Author:  Silas Wang

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RotaryKnob.h"
#include "LookAndFeel.h"

//==============================================================================
RotaryKnob::RotaryKnob(const juce::String& text, juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& parameterID, bool drawFromMiddle,
                       int w, int knobH, int textH) : attachment(apvts, parameterID.getParamID(), slider)
{
    knobHeight = knobH;
    textHeight = textH;
    width = w;
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
//    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, w, textH - 12);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, w, textH);
    slider.setBounds(0, 0, w, knobH);
    addAndMakeVisible(slider);
    
    label.setText(text, juce::NotificationType::dontSendNotification);
    
    label.setJustificationType(juce::Justification::horizontallyCentred);
    label.setBorderSize(juce::BorderSize<int>{ 0, 0, 2, 0 });
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);
    
    setSize(w, textH + knobH); //70 110 is default
    
    setLookAndFeel(RotaryKnobLookAndFeel::get());
    
    float pi = juce::MathConstants<float>::pi;
    slider.setRotaryParameters(1.25f * pi, 2.75f * pi, true);
    
    slider.getProperties().set("drawFromMiddle", drawFromMiddle);
    
    //225º and 495º for start and end == 1.25 * pi and 2.75 * pi. If we used 135º at the end, the angle would be drawn in the opposite way
    
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

RotaryKnob::~RotaryKnob()
{
}

void RotaryKnob::resized()
{
    slider.setTopLeftPosition(0, textHeight);
    // This method is where you should set the bounds of any child
    // components that your component contains..
}
