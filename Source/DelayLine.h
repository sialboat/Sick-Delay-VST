/*
  ==============================================================================

    DelayLine.h
    Created: 4 Jan 2025 5:23:31pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

#include <memory>

class DelayLine
{
public:
    void setMaximumDelayInSamples(int maxLengthInSamples);
    void reset() noexcept;
    
    void write(float input) noexcept;
    float read(float delayInSamples) const noexcept;
    
    int getBufferLength() const noexcept
    {
        return bufferLength;
    }
    
private:
    std::unique_ptr<float[]> buffer; //smart pointer that automatically manages lifetime of allocated memory (no need to de-allocate)
    int bufferLength = 0;
    int writeIndex = 0; //the location of the most recent value and where it was written.
};
