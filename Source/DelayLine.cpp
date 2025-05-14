/*
  ==============================================================================

    DelayLine.cpp
    Created: 4 Jan 2025 5:23:31pm
    Author:  Silas Wang

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DelayLine.h"

void DelayLine::setMaximumDelayInSamples(int maxLengthInSamples)
{
    //verifies that the delay length is positive (so we don't set it as a negative number)
    //this notifies if something is wrong if so
    jassert(maxLengthInSamples > 0);
    
    //allocates memory for the number of samples (we add 2 so the 1-pole smoothing can reach 5000 faster instead of plateauing)
    int paddedLength = maxLengthInSamples + 2;
    
    //don't allocate memory if the current buffer is the same or larger than needed.
    if(bufferLength < paddedLength) {
        
        //allocate memory
        bufferLength = paddedLength;
        buffer.reset(new float[size_t(bufferLength)]);
    }
}

void DelayLine::reset() noexcept
{
    writeIndex = bufferLength - 1;
    for(size_t i = 0; i < size_t(bufferLength); ++i)
    {
        buffer[i] = 0.0f;
    }
}

void DelayLine::write(float input) noexcept
{
    //verifies that delay length is positive
    jassert(bufferLength > 0);
    
    //increment writeIndex, wrap back around if it exceeds bufferLength
    writeIndex++;
    if(writeIndex >= bufferLength)
        writeIndex = 0;
    
    //shift writeIndex
//    if(writeIndex >= bufferLength)
//        writeIndex = 0;
    
    //put the new sample into the memory buffer
    buffer[size_t(writeIndex)] = input;
}

float DelayLine::read(float delayInSamples, int mode) const noexcept
{
    switch (mode)
    {
        case 0: //linear
            return linear(delayInSamples);
        case 1: //lagrange
            return lagrange(delayInSamples);
        case 2: //cubic
            return cubic(delayInSamples);
        case 3: //hermite
            return hermite(delayInSamples);
        default:
            return lagrange(delayInSamples);
    }
}

float DelayLine::lagrange(float delayInSamples) const noexcept
{
    jassert(delayInSamples >= 2.0f);
    jassert(delayInSamples <= bufferLength - 3.0f);

    int integerDelay = static_cast<int>(delayInSamples);
    float fraction = delayInSamples - static_cast<float>(integerDelay);

    int readIndex[4];
    for (int i = 0; i < 4; ++i)
    {
        readIndex[i] = writeIndex - integerDelay + i - 1;
        if (readIndex[i] < 0)
            readIndex[i] += bufferLength;
    }

    float sample[4];
    for (int i = 0; i < 4; ++i)
    {
        sample[i] = buffer[static_cast<size_t>(readIndex[i])];
    }

    float a = sample[1] - sample[0] - sample[2] + sample[3];
    float b = sample[2] - sample[0] - a;
    float c = sample[1] - sample[0];

    return sample[0] + fraction * (c + fraction * (b + fraction * a));
}

float DelayLine::hermite(float delayInSamples) const noexcept
{
//    DBG("Now Playing: Hermite Interpolation");
    jassert(delayInSamples >= 1.0f);
    jassert(delayInSamples <= bufferLength - 2.0f);
    
    //Hermite interpolation based off of Matt Tytel's explanation at ADC21
    int integerDelay = int(delayInSamples);
    
    int readIndexA = writeIndex - integerDelay + 1;
    int readIndexB = readIndexA - 1;
    int readIndexC = readIndexA - 2;
    int readIndexD = readIndexA - 3;
    
    if(readIndexD < 0) {
        readIndexD += bufferLength;
        if(readIndexC < 0) {
            readIndexC += bufferLength;
            if(readIndexB < 0) {
                readIndexB += bufferLength;
                if(readIndexA < 0) {
                    readIndexA += bufferLength;
                }
            }
        }
    }
    
    float sampleA = buffer[size_t(readIndexA)];
    float sampleB = buffer[size_t(readIndexB)];
    float sampleC = buffer[size_t(readIndexC)];
    float sampleD = buffer[size_t(readIndexD)];
    
    float fraction = delayInSamples - float(integerDelay);
    float slope0 = (sampleC - sampleA) * 0.5f;
    float slope1 = (sampleD - sampleB) * 0.5f;
    float v = sampleB - sampleC;
    float w = slope0 + v;
    float a = w + v + slope1;
    float b = w + a;
    float stage1 =  a * fraction - b;
    float stage2 = stage1 * fraction + slope0;
    return stage2 * fraction + sampleB;
}

float DelayLine::cubic(float delayInSamples) const noexcept
{
//    DBG("Now Playing: Cubic Interpolation");
    jassert(delayInSamples >= 1.0f);
    jassert(delayInSamples <= bufferLength - 3.0f);

    int integerDelay = static_cast<int>(delayInSamples);
    float fraction = delayInSamples - static_cast<float>(integerDelay);

    int readIndex[4];
    for (int i = 0; i < 4; ++i)
    {
        readIndex[i] = writeIndex - integerDelay + i - 1;
        if (readIndex[i] < 0)
            readIndex[i] += bufferLength;
    }

    float sample[4];
    for (int i = 0; i < 4; ++i)
    {
        sample[i] = buffer[static_cast<size_t>(readIndex[i])];
    }

    float a0 = sample[3] - sample[2] - sample[0] + sample[1];
    float a1 = sample[0] - sample[1] - a0;
    float a2 = sample[2] - sample[0];
    float a3 = sample[1];

    return a0 * fraction * fraction * fraction + a1 * fraction * fraction + a2 * fraction + a3;
}

float DelayLine::linear(float delayInSamples) const noexcept
{
//    DBG("Now Playing: Linear Interpolation");
    jassert(delayInSamples >= 0.0f);
    jassert(delayInSamples <= bufferLength - 1.0f);

    int integerDelay = int(delayInSamples);

    int readIndexA = writeIndex - integerDelay;
    if (readIndexA < 0) {
        readIndexA += bufferLength;
    }

    int readIndexB = readIndexA - 1;
    if (readIndexB < 0) {
        readIndexB += bufferLength;
    }

    float sampleA = buffer[size_t(readIndexA)];
    float sampleB = buffer[size_t(readIndexB)];

    float fraction = delayInSamples - float(integerDelay);
    return sampleA + fraction * (sampleB - sampleA);
}
/*
 Liner Interpolation:
 
 jassert(delayInSamples >= 0.0f);
 jassert(delayInSamples <= bufferLength - 1.0f);

 int integerDelay = int(delayInSamples);

 int readIndexA = writeIndex - integerDelay;
 if (readIndexA < 0) {
     readIndexA += bufferLength;
 }

 int readIndexB = readIndexA - 1;
 if (readIndexB < 0) {
     readIndexB += bufferLength;
 }

 float sampleA = buffer[size_t(readIndexA)];
 float sampleB = buffer[size_t(readIndexB)];

 float fraction = delayInSamples - float(integerDelay);
 return sampleA + fraction * (sampleB - sampleA);
 
 ===
 Nearest Neighbors:
 ===
 
 jassert(delayInSamples >= 0.0f);
 jassert(delayInSamples <= bufferLength - 1.0f);

 int readIndex = int(std::round(writeIndex - delayInSamples));
 if (readIndex < 0) {
     readIndex += bufferLength;
 }

 return buffer[size_t(readIndex)];
 
 ===
 Hermite Interpolation:
 ===
 //catch potential mistakes
 jassert(delayInSamples >= 1.0f);
 jassert(delayInSamples <= bufferLength - 2.0f);
 
 //Hermite interpolation based off of Matt Tytel's explanation at ADC21
 int integerDelay = int(delayInSamples);
 
 int readIndexA = writeIndex - integerDelay + 1;
 int readIndexB = readIndexA - 1;
 int readIndexC = readIndexA - 2;
 int readIndexD = readIndexA - 3;
 
 if(readIndexD < 0) {
     readIndexD += bufferLength;
     if(readIndexC < 0) {
         readIndexC += bufferLength;
         if(readIndexB < 0) {
             readIndexB += bufferLength;
             if(readIndexA < 0) {
                 readIndexA += bufferLength;
             }
         }
     }
 }
 
 float sampleA = buffer[size_t(readIndexA)];
 float sampleB = buffer[size_t(readIndexB)];
 float sampleC = buffer[size_t(readIndexC)];
 float sampleD = buffer[size_t(readIndexD)];
 
 float fraction = delayInSamples - float(integerDelay);
 float slope0 = (sampleC - sampleA) * 0.5f;
 float slope1 = (sampleD - sampleB) * 0.5f;
 float v = sampleB - sampleC;
 float w = slope0 + v;
 float a = w + v + slope1;
 float b = w + a;
 float stage1 =  a * fraction - b;
 float stage2 = stage1 * fraction + slope0;
 return stage2 * fraction + sampleB;
 
 ===
 Lagrange
 ===
 jassert(delayInSamples >= 1.0f);
 jassert(delayInSamples <= bufferLength - 3.0f);

 int integerDelay = static_cast<int>(delayInSamples);
 float fraction = delayInSamples - static_cast<float>(integerDelay);

 int readIndex[4];
 for (int i = 0; i < 4; ++i)
 {
     readIndex[i] = writeIndex - integerDelay + i - 1;
     if (readIndex[i] < 0)
         readIndex[i] += bufferLength;
 }

 float sample[4];
 for (int i = 0; i < 4; ++i)
 {
     sample[i] = buffer[static_cast<size_t>(readIndex[i])];
 }

 float a = sample[1] - sample[0] - sample[2] + sample[3];
 float b = sample[2] - sample[0] - a;
 float c = sample[1] - sample[0];

 return sample[0] + fraction * (c + fraction * (b + fraction * a));
 
 ===
 Cubic interpolation
 ===
 jassert(delayInSamples >= 1.0f);
 jassert(delayInSamples <= bufferLength - 3.0f);

 int integerDelay = static_cast<int>(delayInSamples);
 float fraction = delayInSamples - static_cast<float>(integerDelay);

 int readIndex[4];
 for (int i = 0; i < 4; ++i)
 {
     readIndex[i] = writeIndex - integerDelay + i - 1;
     if (readIndex[i] < 0)
         readIndex[i] += bufferLength;
 }

 float sample[4];
 for (int i = 0; i < 4; ++i)
 {
     sample[i] = buffer[static_cast<size_t>(readIndex[i])];
 }

 float a0 = sample[3] - sample[2] - sample[0] + sample[1];
 float a1 = sample[0] - sample[1] - a0;
 float a2 = sample[2] - sample[0];
 float a3 = sample[1];

 return a0 * fraction * fraction * fraction + a1 * fraction * fraction + a2 * fraction + a3;
 
 */
