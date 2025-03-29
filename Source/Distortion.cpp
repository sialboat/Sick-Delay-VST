/*
  ==============================================================================

    Distortion.cpp
    Created: 21 Jan 2025 11:17:38pm
    Author:  Silas Wang

  ==============================================================================
*/

#include "Distortion.h"

template <typename T> static int sgn(T val)
{
    return (T(0)) < val - (val < T(0));
}

Distortion::Distortion()
{
    controls.distMode = 0;
    controls.distDrive = 1.0f;
    controls.distCurve = 0.0f;
    controls.distBias = 0.0f;
}
Distortion::~Distortion() {}

float Distortion::processSample(float sample, float mix)
{
    input = sample;
    
    switch(controls.distMode) {
        case 1:
            output = softClip(sample);
            break;
        case 2:
            output = hardClip(sample);
            break;
        case 3:
            output = inflator(sample, controls.distCurve);
            break;
        case 4:
            output = tapeTube(sample, controls.distDrive, controls.distCurve, controls.distBias);
            break;
        case 5:
            output = swell(sample, controls.distDrive, controls.distCurve);
            break;
        case 6:
            output = oddEven(sample, controls.distDrive, controls.distCurve);
        default:
            output = input;
            break;
    }
    
    return ((1.0f - mix) * input) + (mix * output);
}

float Distortion::softClip(float sample)
{
    if(sample > 1.0f || sample < -1.0f) {
        return tanh(sample);
    }
    return sample;
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

float Distortion::tapeTube(float sample, float drive, float curve, float bias)
{
//    float tape = tanh(curve + drive * sample);
    //bias for tape can only be from -1 to 1, should be around -5 to 5 for tube, double check.
    float tape = (float)(2 / M_PI) * atan(drive * sample + bias);
    float tube = 1 - exp((-1 * abs(sample)) * (drive + (3.7f * bias) * sgn(sample)));
    
    return (1 - curve) * tape + curve * tube;
}

float Distortion::swell(float sample, float drive, float curve)
{
    return ((1 + drive) * sample) / (1 + pow((abs(drive * sample)), curve) );
}

float Distortion::oddEven(float sample, float drive, float curve)
{
    float odd = tanh(sample * drive);
    float even = tanh(sample * drive) * tanh(sample * drive);
    return (1 - curve) * odd + curve * even;
}
