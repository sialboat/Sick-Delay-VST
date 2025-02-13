/*
  ==============================================================================

    LevelMeter.cpp
    Created: 5 Jan 2025 1:35:44pm
    Author:  Silas Wang

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LevelMeter.h"
#include "LookAndFeel.h"

LevelMeter::LevelMeter(Measurement& measurementL_, Measurement& measurementR_)
    : measurementL(measurementL_), measurementR(measurementR_), dbLevelL(clampdB), dbLevelR(clampdB)
{
    setOpaque(true);
    decay = 1.0f - std::exp(-1.0f / (float(refreshRate) * 0.2f));
    startTimerHz(refreshRate);
}

LevelMeter::~LevelMeter()
{
}

void LevelMeter::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    
    g.fillAll(Colors::LevelMeter::background);
    
    
    drawLevel(g, dbLevelL, 0, 7);
    drawLevel(g, dbLevelR, 9, 7);
    
    g.setFont(Fonts::getFont(10.0f));
    for(float db = maxdB; db >= mindB; db -= stepdB) { //meter tickline
        int y = positionForLevel(db);
        
        g.setColour(Colors::LevelMeter::tickLine);
        g.fillRect(0, y, 25, 1);
        
        g.setColour(Colors::LevelMeter::tickLabel);
        g.drawSingleLineText(juce::String(int(db)), bounds.getWidth(), y + 3, juce::Justification::right);
    }
    
}

void LevelMeter::resized()
{
    maxPos = 4.0f; //maxPos is 4 pixels from the top
    minPos = float(getHeight()) - 4.0f; //minPos is 4 pixels from the bottom
}

void LevelMeter::timerCallback()
{
    //    DBG("left: " << measurementL.load() << ", right: " << measurementR.load());
    updateLevel(measurementL.readAndReset(), levelL, dbLevelL);
    updateLevel(measurementR.readAndReset(), levelR, dbLevelR);
    
    repaint();
}

void LevelMeter::drawLevel(juce::Graphics& g, float level, int x, int width)
{
    int y = positionForLevel(level);
    if(level > 0.0f) {
        int y0 = positionForLevel(0.0f);
        g.setColour(Colors::LevelMeter::tooLoud);
        g.fillRect(x, y, width, y0 - y);
        g.setColour(Colors::LevelMeter::levelOK);
        g.fillRect(x, y0, width, getHeight() - y0);
    } else if (y < getHeight()) {
        g.setColour(Colors::LevelMeter::levelOK);
        g.fillRect(x, y, width, getHeight() - y);
    }
}

void LevelMeter::updateLevel(float newLevel, float& smoothedLevel, float& leveldB) const
{
    if(newLevel > smoothedLevel) {
        smoothedLevel = newLevel;
    } else {
        smoothedLevel += (newLevel - smoothedLevel) * decay;
    }
    
    if(smoothedLevel > clampLevel) {
        leveldB = juce::Decibels::gainToDecibels(smoothedLevel);
    } else {
        leveldB = clampdB;
    }
}
