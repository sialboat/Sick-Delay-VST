/*
  ==============================================================================

    BypassGradient.cpp
    Created: 6 Jan 2025 11:41:30pm
    Author:  Silas Wang

  ==============================================================================
*/

#include "BypassGradient.h"

BypassGradient::BypassGradient(){
    isBypassed = true;
}

void BypassGradient::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    if(isBypassed) {
//        
        g.setColour(Colors::bypassedSum);
        g.fillRect(bounds.withTop(getY()));
        
        auto gradient = juce::ColourGradient(Colors::bypassedCenter, bounds.getCentreX(), bounds.getCentreY(),
                                             Colors::bypassedFarthest, bounds.getCentreX(), bounds.getHeight(), true);
        //        g.fillAll(Colors::bypassedFarthest);
        //        gradient.addColour(0.5, juce::Colours::grey);
        g.setGradientFill(gradient);
        g.fillRect(bounds);
        
        g.setColour(juce::Colours::white);
        g.setFont(Fonts::getFont(45.0f));
        //        g.setFont(45.0f);
        g.drawFittedText("Plugin is bypassed", getLocalBounds(), juce::Justification::centred, 1);
    } else {
        g.fillAll(juce::Colours::transparentBlack);
    }
}

void BypassGradient::setBypass(bool b)
{
    isBypassed = b;
    repaint();
}
