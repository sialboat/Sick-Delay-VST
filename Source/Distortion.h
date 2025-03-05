/*
  ==============================================================================

    Distortion.h
    Created: 21 Jan 2025 11:17:38pm
    Author:  Silas Wang

  ==============================================================================
*/

#include <cmath>
#include <JuceHeader.h>

#pragma once

//mock distortion code based on this github repository i found: https://github.com/buosseph/juce-distortion
//inflator math found on this gearspace forum post: https://gearspace.com/board/showpost.php?p=15125058&postcount=118

class Distortion
{
public:
    struct Controls
    {
        //0 == off, 1 == soft clipping, 2 == hard clipping, 3 == inflator
        int distMode;
        float distDrive;
        float distCurve;
        float distMix;
    } controls;
    
    Distortion();
    ~Distortion();
    
    float processSample(float sample);
    
    void setValues(int distMode_, float distDrive_, float distCurve_, float distMix_)
    {
        controls.distMode = distMode_;
        controls.distDrive = distDrive_;
        controls.distCurve = distCurve_;
        controls.distMix = distMix_;
    }
    
private:
    float input, output = 0.0f;
    
    float softClip(float sample, float drive); //tanh(alphax)
    float hardClip(float sample);
    float inflator(float sample, float curve);
    float cubic(float sample);
    float tapeTube(float sample, float curve);
    
    //this won't be fully implemented for the release version as there are far too many effects to think about here
    float saturator(float sample); //we can try arctan(2xalpha/pi) x>=0 and arctan(alphax) x <= 0
};
