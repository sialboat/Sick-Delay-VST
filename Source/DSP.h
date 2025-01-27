/*
  ==============================================================================

    DSP.h
    Created: 2 Jan 2025 4:53:14pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <cmath>

inline void panningEqualPower(float panning, float& left, float& right)
{
    float x = 0.7853981633974483f * (panning + 1.0f); //this is π/4
    left = std::cos(x);
    right = std::sin(x);
}
