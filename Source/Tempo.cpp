/*
  ==============================================================================

    Tempo.cpp
    Created: 2 Jan 2025 6:42:09pm
    Author:  Silas Wang

  ==============================================================================
*/

#include "Tempo.h"

static std::array<double, 16> noteLengthMultipliers =
{
    0.125,      //0 = 1/32
    0.5/3.0,    //1 = 1/16 triplet
    0.1875,     //2 = 1/32 dotted
    0.25,       //3 = 1/16
    1.0/3.0,    //4 = 1/8 triplet
    0.375,      //5 = 1/116 dotted
    0.5,        //6 = 1/8
    2.0/3.0,    //7 = 1/4 triplet
    0.75,       //8 = 1/8 dotted
    1.0,        //9 = 1/4
    4.0/3.0,    //10 = 1.2 triplet
    1.5,        //11 = 1/4 dotted
    2.0,        //12 = 1/2
    8.0/3.0,    //13 = 1/1 triplet
    3.0,        //14 = 1/2 dotted
    4.0,        //15 = 1/1
};

//sets the tempo to a default value
void Tempo::reset() noexcept
{
    bpm = 120.0;
}

void Tempo::update(const juce::AudioPlayHead* playhead) noexcept
{
    reset(); //resets to 120 if any steps fail, just to be sure
    
    if(playhead == nullptr) return; //null check for pointers, stopping the function if so
    
    const auto opt = playhead->getPosition(); //store important playhead information
    
    if(!opt.hasValue()) return; //if there's no value in opt, stop the function if so
    
    const auto& pos = *opt; //dereferences opt so we can obtain PositionInfo object from a pointer
    
    if(pos.getBpm().hasValue()) { //if there's a bpm value, set bpm equal to said value.
        bpm = *pos.getBpm();
    }
}

//60 000 (ms in one minute) * desired note length / bpm
double Tempo::getMillisecondsForNoteLength(int index) const noexcept
{
    //size_t casts an int to an unsigned number, making the compiler happy
    return 60000.0 * noteLengthMultipliers[size_t(index)] / bpm;
}
