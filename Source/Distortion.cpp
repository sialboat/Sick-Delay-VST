/*
  ==============================================================================

    Distortion.cpp
    Created: 21 Jan 2025 11:17:38pm
    Author:  Silas Wang

  ==============================================================================
*/

#include "Distortion.h"

Distortion::Distortion()
{
    controls.distMode = 0;
    controls.distDrive = 1.0f;
    controls.distMix = 0.0f;
    controls.distCurve = 0.0f;
}
Distortion::~Distortion() {}

float Distortion::processSample(float sample)
{
    input = sample;
    output = input * controls.distDrive;
    
    switch(controls.distMode) {
        case 1:
            output = softClip(sample, controls.distDrive);
            break;
        case 2:
            output = hardClip(sample);
            break;
        case 3:
            output = inflator(sample, controls.distCurve);
            break;
        default:
            output = input;
            break;
    }
    
    return (1.0f - controls.distMix) * input + controls.distMix * output;
}

float Distortion::softClip(float sample, float drive)
{
    return tanh(sample * drive);
}

float Distortion::hardClip(float sample)
{
    if(sample < -1.0f) {
        return -1.0f;
    } else if(sample > 1.0f) {
        return 1.0f;
    } else {
        return sample;
    }
}

float Distortion::inflator(float sample, float curve)
{
    float A = 1.0f + (curve + 50.0f) / 100.0f;
    float B = -curve / 50.0f;
    float C = (curve - 50.0f) / 100.0f;
    float D = (1/16.0f) - (curve / 400.0f) + pow(curve, 2.0f)/(4.0f * pow(10.0f, 4.0f));
    
    return (A * sample) + (B * pow(sample, 2.0f)) + (C * pow(sample, 3.0f)) -
    ((D * (pow(sample, 2.0f)) - (2 * pow(sample, 3.0f)) + pow(sample, 4.0f)));
}
