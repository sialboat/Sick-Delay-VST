/*
  ==============================================================================

    BypassGradient.h
    Created: 6 Jan 2025 11:41:30pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class BypassGradient : public juce::Component
{
public:
    BypassGradient();
    void paint(juce::Graphics& g) override;
    void setBypass(bool b);
    
    bool hitTest(int, int) override {return false; }
private:
    bool isBypassed;
//    bool paintChecker(bool b);
};
//
//adding text and a 50% opacity thing saying plugin is bypassed if plugin is bypassed
//if(audioProcessor.params.bypassed) {
//    auto bounds = getLocalBounds().toFloat();
//    
//    g.setColour(Colors::bypassedSum);
//    g.fillRect(bounds.withTop(bounds.getY() + 40));
//    
//    auto gradient = juce::ColourGradient(Colors::bypassedCenter, bounds.getCentreX(), bounds.getCentreY(),
//                                         Colors::bypassedFarthest, bounds.getCentreX(), bounds.getHeight(), true);
//        g.fillAll(Colors::bypassedFarthest);
//        gradient.addColour(0.5, juce::Colours::grey);
//    g.setGradientFill(gradient);
//    g.fillRect(bounds.withTop(bounds.getY() + 40));
//    
//    g.setColour(juce::Colours::white);
//    g.setFont(Fonts::getFont(45.0f));
//    //        g.setFont(45.0f);
//    g.drawFittedText("Plugin is bypassed", getLocalBounds(), juce::Justification::centred, 1);
//    repaint();
//} else {
//    repaint();
//}
