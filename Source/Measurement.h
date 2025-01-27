/*
  ==============================================================================

    Measurement.h
    Created: 6 Jan 2025 6:12:15pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <atomic>

struct Measurement //use struct if you want all of the functions and variables to be public
{
    void reset() noexcept
    {
        value.store(0.0f);
    }
    
    void updateIfGreater(float newValue) noexcept
    {
        auto oldValue = value.load();
        while(newValue > oldValue && !value.compare_exchange_weak(oldValue, newValue));
    }
    
    float readAndReset() noexcept
    {
        return value.exchange(0.0f);
    }
    
    std::atomic<float> value;
};
