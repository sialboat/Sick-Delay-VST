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
//    clipperButton.setEnabled(true);
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
    fxGroup.addAndMakeVisible(preFXButton);
    
    postFXButton.setButtonText("Post");
    postFXButton.setClickingTogglesState(true);
    postFXButton.setBounds(0, 0, 35, 27);
    postFXButton.getProperties().set("roundLeft", false);
    postFXButton.getProperties().set("roundRight", true);
    postFXButton.setLookAndFeel(HalfRoundedButtonLookAndFeel::get());
    fxGroup.addAndMakeVisible(postFXButton);
    
    preFXButton.setRadioGroupId(filterButtons);
    postFXButton.setRadioGroupId(filterButtons);
    
    
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
    setSize(820, 480);
    
    setLookAndFeel(&mainLF);
    
    updateDelayKnobs(audioProcessor.params.tempoSyncParam->get());
    
    audioProcessor.params.tempoSyncParam->addListener(this);
//    audioProcessor.params.delayModeParam->addListener(this);
}

ClipDelayAudioProcessorEditor::~ClipDelayAudioProcessorEditor()
{
    audioProcessor.params.tempoSyncParam->removeListener(this);
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
    outputGroup.setBounds(bounds.getWidth() - 180, y, 170, height);
    fxGroup.setBounds(outputGroup.getX() - 200, y, 190, height);
    feedbackGroup.setBounds(delayGroup.getRight() + 10, y, fxGroup.getX() - delayGroup.getRight() - 20, height);
    
//    feedbackGroup.setBounds(distortionGroup.getRight() + 10, y, distortionGroup.getX() - delayGroup.getRight() - 20, height);
    //376 width for fxgroup
    //fxGroup.getX() - delayGroup.getRight() - 20
    //position the knobs inside the groups
    
    //delay group
    delayTimeKnob.setTopLeftPosition(20, 20);
    tempoSyncButton.setTopLeftPosition(20, delayTimeKnob.getBottom() + 5);
    delayNoteKnob.setTopLeftPosition(delayTimeKnob.getX(), delayTimeKnob.getY());
    spreadKnob.setTopLeftPosition(20, tempoSyncButton.getBottom() + 10);
    delayModeButton.setTopLeftPosition(20, spreadKnob.getBottom() + 20);
    comboBox.setBounds(spreadKnob.getX(), spreadKnob.getBottom() + 5,
                       tempoSyncButton.getWidth(), tempoSyncButton.getHeight());
//    comboBox.setTopLeftPosition(spreadKnob.getX(), spreadKnob.getBottom() + 10, 100, 36);
//    comboBox.setBounds(spreadKnob.getX(), spreadKnob.getBottom() + 10, 100, 36);
    
    //output group
    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), tempoSyncButton.getBottom() + 10);
    meter.setBounds(outputGroup.getWidth() - 45, 30, 30, gainKnob.getBottom() - 30);
    clipperButton.setTopLeftPosition(gainKnob.getX(), mixKnob.getBottom() + 10);
    
    //feedback group
    feedbackKnob.setTopLeftPosition(30, 8);
    stereoKnob.setTopLeftPosition(feedbackKnob.getRight() + 30, 8);
    lowCutKnob.setTopLeftPosition(feedbackKnob.getX(), feedbackKnob.getBottom() - 10);
    highCutKnob.setTopLeftPosition(lowCutKnob.getRight() + 30, lowCutKnob.getY());
    
    //distortion group
    fxSelectKnob.setTopLeftPosition(20, 20);
//    fxAmountKnob.setTopLeftPosition(fxSelectKnob.getX(), highCutKnob.getY() + 10);
    preFXButton.setTopLeftPosition(fxSelectKnob.getX(), fxSelectKnob.getBottom() + 10);
    postFXButton.setTopLeftPosition(preFXButton.getRight(), preFXButton.getY());
//    distortionDriveKnob.setTopLeftPosition(30, 20);
}

void ClipDelayAudioProcessorEditor::parameterValueChanged(int, float value)
{
    DBG("param changed: " << value);
    if(juce::MessageManager::getInstance()->isThisTheMessageThread())
        updateDelayKnobs(value != 0.0f);
    else {
        juce::MessageManager::callAsync([this, value]
        {
            updateDelayKnobs(value != 0.0f);
        });
    }
    
}

void ClipDelayAudioProcessorEditor::updateDelayKnobs(bool tempoSyncActive)
{
    delayTimeKnob.setVisible(!tempoSyncActive);
    delayNoteKnob.setVisible(tempoSyncActive);
}
