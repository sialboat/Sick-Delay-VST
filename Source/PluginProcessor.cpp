/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtectYourEars.h" //for developing the plugin, eliminates any signal above or below ±2.0f

//==============================================================================
ClipDelayAudioProcessor::ClipDelayAudioProcessor() : AudioProcessor (BusesProperties()
     .withInput("Input", juce::AudioChannelSet::stereo(), true)
     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
     ), 
    params(apvts)
{
    lowCutFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    highCutFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    outputClipper = std::make_unique<Distortion>(); //way to use unique_ptr
    fxDistortion = std::make_unique<Distortion>();
}

ClipDelayAudioProcessor::~ClipDelayAudioProcessor()
{
}

//==============================================================================
const juce::String ClipDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClipDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ClipDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ClipDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ClipDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClipDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ClipDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClipDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ClipDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void ClipDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ClipDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
   //reset variables here
    params.prepareToPlay(sampleRate);
    params.reset();
    
   
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;
    
//    delayLine.prepare(spec);
    
    double numSamples = Parameters::maxDelayTime / 1000.0 * sampleRate;
    int maxDelayInSamples = int(std::ceil(numSamples));
    
    delayLineL.setMaximumDelayInSamples(maxDelayInSamples);
    delayLineR.setMaximumDelayInSamples(maxDelayInSamples);
    delayLineL.reset();
    delayLineR.reset();
    
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    
    delayInSamplesL = 0.0f;
    delayInSamplesR = 0.0f;
    targetDelayL = 0.0f;
    targetDelayR = 0.0f;
    xfadeL = 0.0f;
    xfadeR = 0.0f;
    xfadeInc = static_cast<float>(1.0 / (0.05 * sampleRate)); //50ms based on the incoming sample size
    
//    spread = 0.0f;
    
    lowCutFilter.prepare(spec);
    lowCutFilter.reset();
    
    highCutFilter.prepare(spec);
    highCutFilter.reset();
    
    lastLowCut = -1.0f;
    lastHighCut = -1.0f;
    
    tempo.reset();
    
    levelL.reset();
    levelR.reset();
    //    DBG(maxDelayInSamples);
    
}

void ClipDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    
    if(mainIn == mono && mainOut == mono) return true;
    if(mainIn == mono && mainOut == stereo) return true;
    if(mainIn == stereo && mainOut == stereo) return true;
    
    return false;
}

float ClipDelayAudioProcessor::processEffect(int fxIndex, float sample)
{
//    DBG("fx index: " << fxIndex);
//    DBG(params.fxSelect);
//    DBG(params.tapeTubeDrive);
//    DBG("Mix: " << params.oddEvenMix << ", Drive: " << params.oddEvenDrive << ", Bias " << params.oddEvenBias << ", Curve: " << params.oddEvenCurve);
    //each other combobox parameter for time, modulation, etc. must have different indexes. distortion has 0-3!
    float output = sample;
    switch(fxIndex) {
        case 0: //tapeTube
            fxDistortion->setMode(4);
            fxDistortion->setDriveCurveBias(params.tapeTubeDrive,
                                            params.tapeTubeCurve,
                                            params.tapeTubeBias);
            output = fxDistortion->processSample(sample, params.tapeTubeMix);
        break;
        case 1: //oddEven
            fxDistortion->setMode(6);
            fxDistortion->setDriveCurveBias(params.oddEvenDrive,
                                            params.oddEvenCurve,
                                            params.oddEvenBias);
            output = fxDistortion->processSample(sample, params.oddEvenMix);
            break;
        case 2: //soar
            fxDistortion->setMode(5);
//            fxDistortion->setDriveCurveBias()
//            output = fxDistortion->processSample(sample, )
//            fxDistortion->setMode();
            break;
        case 3:
            break;
        case 4:
            break;
    }
    return output;
}
//#endif

void ClipDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    params.update(); //adjusts parameter values
    tempo.update(getPlayHead()); //gathers tempo-based value information
    outputClipper->setValues(params.clipperMode, 1.0f, 0.0f); //clipper mode
    
    //limits delay time to the maximum delay duration
    float syncedTime = float(tempo.getMillisecondsForNoteLength(params.delayNote));
    if(syncedTime > Parameters::maxDelayTime) syncedTime = Parameters::maxDelayTime;

    float sampleRate = float(getSampleRate());

    auto mainInput = getBusBuffer(buffer, true, 0);
    auto mainInputChannels = mainInput.getNumChannels();
    auto isMainInputStereo = mainInputChannels > 1;
    const float* inputDataL = mainInput.getReadPointer(0);
    const float* inputDataR = mainInput.getReadPointer(isMainInputStereo ? 1 : 0);
    
    auto mainOutput = getBusBuffer(buffer, false, 0);
    auto mainOutputChannels = mainOutput.getNumChannels();
    auto isMainOutputStereo = mainOutputChannels > 1;
    float* outputDataL = mainOutput.getWritePointer(0);
    float* outputDataR = mainOutput.getWritePointer(isMainOutputStereo ? 1 : 0);
    
    float maxL = 0.0f;
    float maxR = 0.0f;
    //    DBG("Spread: " << params.spread);
    //use the juce::dsp::WaveShaper class to to add a clipper / drive
    
//    auto choice = apvts.getRawParameterValue(delayModeParamID.getParamID())->load();
//    DBG("choice: " << choice << " param: " << params.delayMode);
//    DBG(apvts.getRawParameterValue(distortionSelectParamID.getParamID())->load() << " " << apvts.getRawParameterValue(fxSelectParamID.getParamID())->load());
    
    //1 OR TRUE is crossfading, 0 OR FALSE is varispeed
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        params.smoothen(); //smoothens changes to plugin parameters so there's no unwanted clicks or zipper noise
        
        float delayTime = params.tempoSync ? syncedTime : params.delayTime;
        
        float delayTimeL = delayTime + params.spread;
        float delayTimeR = delayTime - params.spread;
        delayTimeL = juce::jlimit(1.0f, 5000.0f, delayTimeL);
        delayTimeR = juce::jlimit(1.0f, 5000.0f, delayTimeR);
        
        delayInSamplesL = delayTimeL / 1000.0f * sampleRate;
        delayInSamplesR = delayTimeR / 1000.0f * sampleRate;
//            float delayTime = params.tempoSync ? syncedTime : params.delayTime;
        
        if(params.delayMode) { // == 1
            if(xfadeL == 0.0f || xfadeR == 0.0f) {
                targetDelayL = delayTimeL / 1000.0f * sampleRate;
                targetDelayR = delayTimeR / 1000.0f * sampleRate;
                
                if(delayInSamplesL == 0.0f) {
                    delayInSamplesL = targetDelayL;
                }
                if(delayInSamplesR == 0.0f) {
                    delayInSamplesR = targetDelayR;
                }
                if (std::abs(targetDelayL - delayInSamplesL) > 0.01f) {
//                    if (targetDelayL != delayInSamplesL) {
                    xfadeL = xfadeInc;
                }
                if (std::abs(targetDelayR - delayInSamplesR) > 0.01f) {
                    xfadeR = xfadeInc;
                }
            }
        }

        if(params.lowCut != lastLowCut) {
            lowCutFilter.setCutoffFrequency(params.lowCut);
            lastLowCut = params.lowCut;
        }
        if(params.highCut != lastHighCut) {
            highCutFilter.setCutoffFrequency(params.highCut);
            lastHighCut = params.highCut;
        }
        
        float dryL = inputDataL[sample]; //reads the next audio sample into LR channels
        float dryR = inputDataR[sample];
        
        float mono = (dryL + dryR) * 0.5f;

        //we process the effect somewhere here i think
        
        if(mainInputChannels == 1) { //for mono compatability
            delayLineL.write(mono*params.panL + feedbackR); //push these samples into the delay line
            delayLineR.write(mono*params.panR + feedbackL); //alternating L and R for ping-pong delay
        } else if(mainInputChannels >= 2) {
            delayLineL.write(dryL*params.panL + feedbackR);
            delayLineR.write(dryR*params.panR + feedbackL);
        }
        
        
        float wetL = delayLineL.read(delayInSamplesL, params.delayQuality); //reading delayed samples with stereo spread
        float wetR = delayLineR.read(delayInSamplesR, params.delayQuality);
//
        if(params.delayMode) {
            if(xfadeL > 0.0f) {
                //read new sample values at the new delay length
                float newL = delayLineL.read(targetDelayL, params.delayQuality);
//                    float newR = delayLineR.read(targetDelayR);
                
                //perform a crossfade
                wetL = (1.0f - xfadeL) * wetL + xfadeL * newL;
//                    wetR = (1.0f - xfadeR) * wetR + xfadeR * newR;
                
                //increment xfade, capping it at 1.0f
                xfadeL += xfadeInc;
                if(xfadeL >= 1.0f) {
                    delayInSamplesL = targetDelayL;
                    xfadeL = 0.0f;
                }
            }
            if(xfadeR > 0.0f) {
                float newR = delayLineR.read(targetDelayR, params.delayQuality);
                
                wetR = (1.0f - xfadeR) * wetR + xfadeR * newR;
                
                xfadeR += xfadeInc;
                
                if(xfadeR >= 1.0f) {
                    delayInSamplesR = targetDelayR;
                    xfadeR = 0.0f;
                }
            }
        }
        
        // PRE
//        if(!params.filterButton) {
//            feedbackL = processEffect(params.fxSelect / 100, sample);
//            feedbackR = processEffect(params.fxSelect / 100, sample);
//        }
//        DBG("fx index: " << params.getProperFxIndex(params.fxSelect));
        wetL = processEffect(params.getProperFxIndex(params.fxSelect), wetL);
        wetR = processEffect(params.getProperFxIndex(params.fxSelect), wetR);
//        DBG(wetL << " " << wetR);
        
        feedbackL = wetL * params.feedback;
        feedbackR = wetR * params.feedback;
                    
        //FILTER PROCESSING
        feedbackL = lowCutFilter.processSample(0, feedbackL);
        feedbackL = highCutFilter.processSample(0, feedbackL);
        feedbackR = lowCutFilter.processSample(1, feedbackR);
        feedbackR = highCutFilter.processSample(1, feedbackR);
        
        // POST
//        if(params.filterButton) {
//            feedbackL = processEffect(params.fxSelect / 100, sample);
//            feedbackR = processEffect(params.fxSelect / 100, sample);
//        }

        //turns up wet mix based on the dry/wet.
//            float mixL = dryL + wetL * params.mix; //calculating a dry/wet
//            float mixR = dryR + wetR * params.mix;
        
        //linear interpolation of dry/wet mix. when mix is 100%, you will only hear the wet signal.
        float mixL = dryL * (1.0f - params.mix) + wetL * params.mix;
        float mixR = dryR * (1.0f - params.mix) + wetR * params.mix;
        
        mixL = outputClipper->processSample(mixL, 1.0f);
        mixR = outputClipper->processSample(mixR, 1.0f);
        
//        float outL = mixL * params.gain;
//        float outR = mixR * params.gain;
        float outL = wetL * params.gain;
        float outR = wetR * params.gain;
        
        //bypass button
        if(params.bypassed) {
            outL = dryL;
            outR = dryR;
        } else {
            //put the clipper processing code here
        }
        
        outputDataL[sample] = outL;
        outputDataR[sample] = outR;
        
        maxL = std::max(maxL, std::abs(outL)); //to send over to pluginEditor for metering
        maxR = std::max(maxR, std::abs(outR));
        
//            outputDataL[sample] = mixL * params.gain; //writing output samples back into juce::audioBuffer and applying final gain.
//            outputDataR[sample] = mixR * params.gain;
        
    }

    //protecting ears during development. add this as the first thing when you build a plugin!
    #if JUCE_DEBUG
    protectYourEars(buffer);
    #endif
    
    levelL.updateIfGreater(maxL);
    levelR.updateIfGreater(maxR);
}

//==============================================================================
bool ClipDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ClipDelayAudioProcessor::createEditor()
{
    return new ClipDelayAudioProcessorEditor (*this);
}

//==============================================================================
void ClipDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
    
//    DBG(apvts.copyState().toXmlString());
}

void ClipDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
//    DBG(apvts.copyState().toXmlString());
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorParameter* ClipDelayAudioProcessor::getBypassParameter() const
{
    return params.bypassParam;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipDelayAudioProcessor();
}
