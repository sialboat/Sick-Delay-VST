/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

//==============================================================================
ClipDelayAudioProcessorEditor::ClipDelayAudioProcessorEditor (ClipDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), meter(p.levelL, p.levelR)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
        
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    delayGroup.addAndMakeVisible(delayTimeKnob);
    delayGroup.addChildComponent(delayNoteKnob);
    delayGroup.addAndMakeVisible(spreadKnob);
    addAndMakeVisible(delayGroup);
    
    feedbackGroup.setText("Feedback");
    feedbackGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    feedbackGroup.addAndMakeVisible(feedbackKnob);
    feedbackGroup.addAndMakeVisible(stereoKnob);
    feedbackGroup.addAndMakeVisible(lowCutKnob);
    feedbackGroup.addAndMakeVisible(highCutKnob);
    addAndMakeVisible(feedbackGroup);
    
    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    outputGroup.addAndMakeVisible(meter);
    addAndMakeVisible(outputGroup);
    
    tempoSyncButton.setButtonText("Sync");
    tempoSyncButton.setClickingTogglesState(true);
    tempoSyncButton.setBounds(0, 0, 70, 27);
    tempoSyncButton.setLookAndFeel(ButtonLookAndFeel::get());
    delayGroup.addAndMakeVisible(tempoSyncButton);
    
    clipperButton.setButtonText(clipperText[clipperButtonIndex]);
    clipperButton.setClickingTogglesState(true);
    clipperButton.setBounds(0, 0, 70, 27);
    clipperButton.setLookAndFeel(EnabledButtonLookAndFeel::get());
    outputGroup.addAndMakeVisible(clipperButton);
    
    clipperButton.onClick = [this]()
    {
        clipperButtonIndex = (clipperButtonIndex + 1) % clipperText.size();
        clipperButton.setButtonText(clipperText[clipperButtonIndex]);
    };
    
    
//    delayModeButton.setButtonText("Mode");
//    delayModeButton.setClickingTogglesState(true);
//    delayModeButton.setBounds(0, 0, 70, 27);
//    delayModeButton.setLookAndFeel(ButtonLookAndFeel::get());
    
//    comboBox.setTextWhenNothingSelected("Delay Modes");
    comboBox.setEnabled(true);
    comboBox.setLookAndFeel(ComboBoxLookAndFeel::get()); //forgot this line lmao
    comboBox.addItem("Analog", 1);
    comboBox.addItem("Digital", 2);
    comboBox.setSelectedId(1);
    
    delayGroup.addAndMakeVisible(comboBox);
    
    delayQualityBox.setEnabled(true);
    delayQualityBox.setLookAndFeel(ComboBoxLookAndFeel::get());
    delayQualityBox.addItem("Linear", 1);
    delayQualityBox.addItem("Lagrange", 2);
    delayQualityBox.addItem("Cubic", 3);
    delayQualityBox.addItem("Hermite", 4);
    delayQualityBox.setSelectedId(2);
    
    outputGroup.addAndMakeVisible(delayQualityBox);
    
    //for the clipper button
    
    fxGroup.setText("FX");
    fxGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
//    fxGroup.addAndMakeVisible(fxAmountKnob);
    fxGroup.addAndMakeVisible(fxSelectKnob);
    preFXButton.setButtonText("Pre");
    preFXButton.setClickingTogglesState(true);
    preFXButton.setBounds(0, 0, 35, 27);
    preFXButton.getProperties().set("roundLeft", true);
    preFXButton.getProperties().set("roundRight", false);
    preFXButton.setLookAndFeel(HalfRoundedButtonLookAndFeel::get());
    preFXButton.setToggleState(true, juce::dontSendNotification); //sets pre filter button to be on by default
    feedbackGroup.addAndMakeVisible(preFXButton);
    
    postFXButton.setButtonText("Post");
    postFXButton.setClickingTogglesState(true);
    postFXButton.setBounds(0, 0, 35, 27);
    postFXButton.getProperties().set("roundLeft", false);
    postFXButton.getProperties().set("roundRight", true);
    postFXButton.setLookAndFeel(HalfRoundedButtonLookAndFeel::get());
    feedbackGroup.addAndMakeVisible(postFXButton);
    
    preFXButton.setRadioGroupId(filterButtons);
    postFXButton.setRadioGroupId(filterButtons);
    
    //softclip
    
    autoGainButton.setClickingTogglesState(true);
    autoGainButton.setBounds(0, 0, 70, 27);
    autoGainButton.setLookAndFeel(ButtonLookAndFeel::get());
    fxGroup.addChildComponent(autoGainButton);
    fxGroup.addChildComponent(tapeTubeDriveKnob);
    fxGroup.addChildComponent(tapeTubeMixKnob);
    //    fxGroup.addChildComponent(softClipMixKnob);
    //    fxGroup.addChildComponent(softClipDriveKnob);
//    fxGroup.addAndMakeVisible(softClipDriveKnob);
//    fxGroup.addAndMakeVisible(softClipMixKnob);
//    fxGroup.addAndMakeVisible(autoGainButton);
//    fxGroup.addChildComponent(autoGainButton);
    
    addAndMakeVisible(fxGroup);
    
//    distortionGroup.addAndMakeVisible()
    
    addAndMakeVisible(bypassGradient);
    
    auto bypassIcon = juce::ImageCache::getFromMemory(BinaryData::Bypass_png, BinaryData::Bypass_pngSize);
    
    bypassButton.setClickingTogglesState(true);
    bypassButton.setBounds(0, 0, 20, 20);
    bypassButton.setImages(false, true, true,
                           bypassIcon, 1.0f, juce::Colours::white,
                           bypassIcon, 1.0f, juce::Colours::white,
                           bypassIcon, 1.0f, juce::Colours::grey,
                           0.0f);
    addAndMakeVisible(bypassButton);

    //for the rotary knob to appear in the desired coordinates, setSize() needs to be at the bottom apparantly.
    setSize(760, 400);
    
    setLookAndFeel(&mainLF);
    
    updateDelayKnobs(audioProcessor.params.tempoSyncParam->get());
    updateFxKnobs(audioProcessor.params.fxSelectParam->getIndex(), true);
    
    audioProcessor.params.tempoSyncParam->addListener(this);
    audioProcessor.params.tapeTubeDriveParam->addListener(this);
    audioProcessor.params.tapeTubeMixParam->addListener(this);
    
    audioProcessor.params.fxSelectParam->addListener(this);
    
//    juce::MessageManager::callAsync([this]()
//    {
//        audioProcessor.params.tempoSyncParam->addListener(this);
//        audioProcessor.params.distDriveParam->addListener(this);
//        audioProcessor.params.distMixParam->addListener(this);
//        
//    });
    
    
    
//    audioProcessor.params.delayModeParam->addListener(this);
}

ClipDelayAudioProcessorEditor::~ClipDelayAudioProcessorEditor()
{
    audioProcessor.params.tempoSyncParam->removeListener(this);
    
    audioProcessor.params.tapeTubeDriveParam->removeListener(this);
    audioProcessor.params.tapeTubeMixParam->removeListener(this);
    
    audioProcessor.params.fxSelectParam->removeListener(this);
    
    
    setLookAndFeel(nullptr);
}

//==============================================================================
void ClipDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    bypassGradient.setBypass(audioProcessor.params.bypassed);
    //drawing noise onto the screen
    auto noise = juce::ImageCache::getFromMemory(BinaryData::Noise_png, BinaryData::Noise_pngSize);
    auto fillType = juce::FillType(noise, juce::AffineTransform::scale(0.5f));
    g.setFillType(fillType);
    g.fillRect(getLocalBounds());
    
    //drawing the PNG Logo
    auto rect = getLocalBounds().withHeight(40);
    g.setColour(Colors::header);
    g.fillRect(rect);
    
    //imageCache loads the image once and puts it in memory so it doesn't have to be done multiple times.
    auto image = juce::ImageCache::getFromMemory(BinaryData::sickDelay_logo_png, BinaryData::sickDelay_logo_pngSize);
    
    int destWidth = image.getWidth() / 2;
    int destHeight = image.getHeight() / 2;
    g.drawImage(image, getWidth() / 2 - destWidth / 2, 2, destWidth, destHeight, 0, 0, image.getWidth(), image.getHeight());
    
//    bypassGradient.paint(g);
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll(Colors::background);
    //consult the source code for understanding how lookAndFeel works! The documentation for look and feel isn't that good
}

void ClipDelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    int y = 50;
    int height = bounds.getHeight() - 60;
    
    bypassButton.setTopLeftPosition(bounds.getRight() - bypassButton.getWidth() - 10, 10);
    bypassGradient.setBounds(0, 0, bounds.getWidth(), bounds.getHeight());
    
    //positioning bounds
    delayGroup.setBounds(10, y, 110, height);
    outputGroup.setBounds(bounds.getWidth() - 190, y, 180, height);
    feedbackGroup.setBounds(delayGroup.getRight() + 10, y, 210, height);
    fxGroup.setBounds(feedbackGroup.getRight() + 10, y, 210, height);
    
//    feedbackGroup.setBounds(distortionGroup.getRight() + 10, y, distortionGroup.getX() - delayGroup.getRight() - 20, height);
    //376 width for fxgroup
    //fxGroup.getX() - delayGroup.getRight() - 20
    //position the knobs inside the groups
    
    //delay group
    delayTimeKnob.setTopLeftPosition(20, 20);
    tempoSyncButton.setTopLeftPosition(20, delayTimeKnob.getBottom() + 5);
    delayNoteKnob.setTopLeftPosition(delayTimeKnob.getX(), delayTimeKnob.getY());
    spreadKnob.setTopLeftPosition(20, tempoSyncButton.getBottom() + 20);
    delayModeButton.setTopLeftPosition(20, spreadKnob.getBottom() + 20);
    comboBox.setBounds(spreadKnob.getX(), spreadKnob.getBottom() + 5,
                       tempoSyncButton.getWidth(), tempoSyncButton.getHeight());
//    comboBox.setTopLeftPosition(spreadKnob.getX(), spreadKnob.getBottom() + 10, 100, 36);
//    comboBox.setBounds(spreadKnob.getX(), spreadKnob.getBottom() + 10, 100, 36);
    
    //output group
    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), tempoSyncButton.getBottom() + 10);
    meter.setBounds(outputGroup.getWidth() - 60, 30, 45, gainKnob.getBottom() - 30);
    clipperButton.setTopLeftPosition(gainKnob.getX(), mixKnob.getBottom() + 10);
    delayQualityBox.setBounds(gainKnob.getX(), gainKnob.getBottom() + 10,
                              tempoSyncButton.getWidth(), tempoSyncButton.getHeight());
    
    //feedback group
    feedbackKnob.setTopLeftPosition(20, 20);
    stereoKnob.setTopLeftPosition(feedbackKnob.getRight() + 30, feedbackKnob.getY());
    lowCutKnob.setTopLeftPosition(feedbackKnob.getX(), feedbackKnob.getBottom() + 20);
    highCutKnob.setTopLeftPosition(lowCutKnob.getRight() + 30, lowCutKnob.getY());
    preFXButton.setTopLeftPosition(lowCutKnob.getX(), lowCutKnob.getBottom() + 20);
    postFXButton.setTopLeftPosition(preFXButton.getRight(), preFXButton.getY());
    
    //fx group
//    fxAmountKnob.setTopLeftPosition(fxSelectKnob.getX(), highCutKnob.getY() + 10);
//    preFXButton.setTopLeftPosition(fxSelectKnob.getX(), fxSelectKnob.getBottom() + 10);
//    postFXButton.setTopLeftPosition(preFXButton.getRight(), preFXButton.getY());
    fxSelectKnob.setTopLeftPosition(60, 8);
    tapeTubeDriveKnob.setTopLeftPosition(fxSelectKnob.getX() - 40, highCutKnob.getY());
    tapeTubeMixKnob.setTopLeftPosition(tapeTubeDriveKnob.getRight() + 30, tapeTubeDriveKnob.getY());
    autoGainButton.setButtonText("Auto Gain");
    autoGainButton.setTopLeftPosition(fxSelectKnob.getX() + 10, fxSelectKnob.getBottom() + 120); //doing this in consideration for
//    softClipMixKnob.setTopLeftPosition
//    distortionDriveKnob.setTopLeftPosition(30, 20);
}

//if(audioProcessor.params.tempoSync) {
//    updateDelayKnobs(value != 0.0f);
//}

void ClipDelayAudioProcessorEditor::parameterValueChanged(int paramIndex, float value)
{
//    auto* param = audioProcessor.params
    DBG("parameter index: " << paramIndex);
    DBG("param changed: " << value);
//    auto* tempoParam = dynamic_cast<juce::AudioParameterBool*>(params);
//    auto* fxParam = dynamic_cast<juce::AudioParameterChoice*>(params);
//    auto* param = audioProcessor.getParameter(paramIndex);

    if(juce::MessageManager::getInstance()->isThisTheMessageThread()) { //running on message thread
        if(paramIndex == 8) //DELAY
            updateDelayKnobs(value != 0.0f);
        if(paramIndex == 13) { //FX SELECTOR
            updateFxKnobs((int)(value * 10), value != 0.0f);
        }
//        if(tempoParam->getParameterID().equalsIgnoreCase("tempoSyncParamID")) {
//            updateDelayKnobs(value != 0.0f);
//        }
//        if(param->getName(0).equalsIgnoreCase("Tempo Sync"))
//            updateDelayKnobs(value != 0.0f);
//        "fxType"
    }
    else { //not running on message thread
        
        //schedules the task so that it doesn't run on the messanger thread
        juce::MessageManager::callAsync([this, paramIndex, value]
        {
            if(paramIndex == 8)
                updateDelayKnobs(value != 0.0f);
            if(paramIndex == 13)
                updateFxKnobs((int)(value * 10), value != 0.0f);
            
        });
    }
}

void ClipDelayAudioProcessorEditor::updateDelayKnobs(bool tempoSyncActive)
{
    delayTimeKnob.setVisible(!tempoSyncActive);
    delayNoteKnob.setVisible(tempoSyncActive);
}

void ClipDelayAudioProcessorEditor::updateFxKnobs(int fxIndex, bool flag)
{
    DBG("index value: " << fxIndex);
    switch(fxIndex) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            tapeTubeDriveKnob.setVisible(flag);
            tapeTubeMixKnob.setVisible(flag);
            autoGainButton.setVisible(flag);
            break;
        default:
            tapeTubeDriveKnob.setVisible(!flag);
            tapeTubeMixKnob.setVisible(!flag);
            autoGainButton.setVisible(!flag);
            break;
            
    }
}
