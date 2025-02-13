/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 30 Dec 2024 3:09:07pm
    Author:  Silas Wang

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LookAndFeel.h"
#include "PluginProcessor.h"

const juce::Typeface::Ptr Fonts::typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::LatoMedium_ttf, BinaryData::LatoMedium_ttfSize);

juce::Font Fonts::getFont(float height)
{
    return juce::FontOptions(typeface)
    .withMetricsKind(juce::TypefaceMetricsKind::legacy)
    .withHeight(height);
}

juce::Font RotaryKnobLookAndFeel::getLabelFont([[maybe_unused]] juce::Label& label)
{
    return Fonts::getFont();
}

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel()
{
    setColour(juce::Label::textColourId, Colors::Knob::label);
    setColour(juce::Slider::textBoxTextColourId, Colors::Knob::label);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderFillColourId, Colors::Knob::trackActive);
    setColour(juce::CaretComponent::caretColourId, Colors::Knob::caret);
}

void RotaryKnobLookAndFeel::fillTextEditorBackground(
juce::Graphics& g, [[maybe_unused]] int width, [[maybe_unused]] int height, juce::TextEditor& textEditor)
{
    g.setColour(Colors::Knob::textBoxBackground);
    g.fillRoundedRectangle(textEditor.getLocalBounds().reduced(4, 0).toFloat(), 4.0f);
}

void RotaryKnobLookAndFeel::drawRotarySlider( juce::Graphics& g,
    int x, int y, int width, [[maybe_unused]] int height,
    float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, width).toFloat(); //bounds of the knob
    auto knobRect = bounds.reduced(10.0f, 10.0f);
    
    auto path = juce::Path(); //drawing the actual circle
    path.addEllipse(knobRect);
    dropShadow.drawForPath(g, path);
    
    g.setColour(Colors::Knob::outline); //set a color value and fill the circle
    g.fillEllipse(knobRect);
    
    auto innerRect = knobRect.reduced(2.0f, 2.0f);
    auto gradient = juce::ColourGradient(Colors::Knob::gradientTop,  0.0f, innerRect.getY(),
                                         Colors::Knob::gradientBottom, 0.0f, innerRect.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillEllipse(innerRect);
    
    auto center = bounds.getCentre();
    auto radius = bounds.getWidth() / 2.0f;
    auto lineWidth = 3.0f;
    auto arcRadius = radius - lineWidth/2.0f;
    
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    auto strokeType = juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.setColour(Colors::Knob::trackBackground);
    g.strokePath(backgroundArc, strokeType);
    
    //given an angle of rotation and a radius, the x-position is radius * cos(angle), the y-position is radius * sin(angle)
    //sin and cos are swapped here because juce thinks 0º is at the top of the circle, whereas math people believe it is on the right.
    
    auto dialRadius = innerRect.getHeight()/2.0f - lineWidth;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Point<float> dialStart(center.x + 10.0f * std::sin(toAngle),
                                 center.y - 10.0f * std::cos(toAngle));
    juce::Point<float> dialEnd(center.x + dialRadius * std::sin(toAngle),
                               center.y - dialRadius * std::cos(toAngle));

    juce::Path dialPath;
    dialPath.startNewSubPath(dialStart);
    dialPath.lineTo(dialEnd);
    g.setColour(Colors::Knob::dial);
    g.strokePath(dialPath, strokeType);
    
    if(slider.isEnabled()) {
        
        float fromAngle = rotaryStartAngle;
        if(slider.getProperties()["drawFromMiddle"]) {
            fromAngle += (rotaryEndAngle - rotaryStartAngle) / 2.0f;
        }
        
        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, fromAngle, toAngle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, strokeType);
    }
    
}

class RotaryKnobLabel : public juce::Label
{
public:
    RotaryKnobLabel(): juce::Label() {}
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override {}
    
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler(*this);
    }
    
    juce::TextEditor* createEditorComponent() override
    {
        auto* ed = new juce::TextEditor(getName());
        ed->applyFontToAllText(getLookAndFeel().getLabelFont(*this));
        copyAllExplicitColoursTo(*ed);
        ed->setBorder(juce::BorderSize<int>());
        ed->setIndents(2, 1);
        ed->setJustification(juce::Justification::centredTop);
        ed->setPopupMenuEnabled(false);
        ed->setInputRestrictions(8);
        
        return ed;
    }
};

juce::Label* RotaryKnobLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto l = new RotaryKnobLabel();
    l->setJustificationType(juce::Justification::centred);
    l->setKeyboardType(juce::TextInputTarget::decimalKeyboard);
    l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::TextEditor::textColourId, Colors::Knob::value);
    l->setColour(juce::TextEditor::highlightedTextColourId, Colors::Knob::value);
    l->setColour(juce::TextEditor::highlightColourId,
                 slider.findColour(juce::Slider::rotarySliderFillColourId));
    l->setColour(juce::TextEditor::backgroundColourId,
                 Colors::Knob::textBoxBackground);
    
    return l;
}

MainLookAndFeel::MainLookAndFeel()
{
    setColour(juce::GroupComponent::textColourId, Colors::Group::label);
    setColour(juce::GroupComponent::outlineColourId, Colors::Group::outline);
}

juce::Font MainLookAndFeel::getLabelFont([[maybe_unused]] juce::Label& label)
{
    return Fonts::getFont();
}

ButtonLookAndFeel::ButtonLookAndFeel()
{
    setColour(juce::TextButton::textColourOffId, Colors::Button::text);
    setColour(juce::TextButton::textColourOnId, Colors::Button::textToggled);
    setColour(juce::TextButton::buttonColourId, Colors::Button::background);
    setColour(juce::TextButton::buttonOnColourId, Colors::Button::backgroundToggled);
}

void ButtonLookAndFeel::drawButtonBackground(
                                             juce::Graphics& g, juce::Button& button, const juce::Colour&backgroundColour,
                                             [[maybe_unused]] bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = bounds.getHeight() * 0.25f;
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
    //    auto buttonRect = bounds.reduced(1.0f, 1.0f);
    
    if(shouldDrawButtonAsDown) {
        buttonRect.translate(0.0f, 1.0f);
    }
    
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(buttonRect, cornerSize);
    
    g.setColour(Colors::Button::outline);
    g.drawRoundedRectangle(buttonRect, cornerSize, 2.0f);
}

void ButtonLookAndFeel::drawButtonText(
                                       juce::Graphics& g, juce::TextButton& button,
                                       [[maybe_unused]] bool shouldDrawButtonAsHighlighted,
                                       bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
//    auto buttonRect = bounds.reduced(1.0f, 1.0f);
    
    if(shouldDrawButtonAsDown) {
        buttonRect.translate(0.0f, 1.0f);
    }
    
    if(button.getToggleState()) {
        g.setColour(button.findColour(juce::TextButton::textColourOnId));
    } else {
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
    }
    
    g.setFont(Fonts::getFont());
    g.drawText(button.getButtonText(), buttonRect, juce::Justification::centred);
    //draw a gradient behind the button when you get the chance you bloke
    //lmao not doing that
}


HalfRoundedButtonLookAndFeel::HalfRoundedButtonLookAndFeel()
{
    setColour(juce::TextButton::textColourOffId, Colors::Button::text);
    setColour(juce::TextButton::textColourOnId, Colors::Button::textToggled);
    setColour(juce::TextButton::buttonColourId, Colors::Button::background);
    setColour(juce::TextButton::buttonOnColourId, Colors::Button::backgroundToggled);
}

void HalfRoundedButtonLookAndFeel::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                                                  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
//    auto buttonRect = bounds.reduced(1.0f, 1.0f);
    
    if(shouldDrawButtonAsDown) {
        buttonRect.translate(0.0f, 1.0f);
    }
    
    if(button.getToggleState()) {
        g.setColour(button.findColour(juce::TextButton::textColourOnId));
    } else {
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
    }
    
    g.setFont(Fonts::getFont().withHeight(g.getCurrentFont().getHeight() - 1.5f));
    g.drawText(button.getButtonText(), buttonRect, juce::Justification::centred);
}

void HalfRoundedButtonLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                        const juce::Colour &backgroundColour,
                                                        bool shouldDrawButtonAsHighlighted,
                                                        bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = bounds.getHeight() * 0.25f;
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
    
    bool roundLeft = button.getProperties().getWithDefault("roundLeft", true);
    bool roundRight = button.getProperties().getWithDefault("roundRight", false);
//    if(shouldDrawButtonAsDown)
//        buttonRect.translate(1.0f, 1.0f);
    
    juce::Path path;
    
    //top left, top right, bottom left, bottom right (boolean order for curved rectangles)
    if(roundLeft && !roundRight) {
        path.addRoundedRectangle(
                                0, 0, bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize,
                                true, false, true, false);
//        path.closeSubPath();
    } else if (!roundLeft && roundRight) {
        path.addRoundedRectangle(0, 0, bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize,
                                 false, true, false, true);
    }
    
    if(shouldDrawButtonAsDown) {
        path.applyTransform(juce::AffineTransform::translation(0.0f, 1.0f));
    }
    
//  background
    g.setColour(backgroundColour);
    g.fillPath(path);
    //outline
    g.setColour(Colors::Button::outline);
    juce::PathStrokeType strokeThickness(3.0f);
//    strokeThickness.setStrokeThickness(juce::PathStrokeType::inside);
    g.strokePath(path, strokeThickness);
}

EnabledButtonLookAndFeel::EnabledButtonLookAndFeel()
{
    setColour(juce::TextButton::textColourOffId, Colors::Button::textToggled);
    setColour(juce::TextButton::textColourOnId, Colors::Button::textToggled);
    setColour(juce::TextButton::buttonColourId, Colors::Button::backgroundToggled);
    setColour(juce::TextButton::buttonOnColourId, Colors::Button::backgroundToggled);
}

void EnabledButtonLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                                    const juce::Colour &backgroundColour,
                                                    bool shouldDrawButtonAsHighlighted,
                                                    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = bounds.getHeight() * 0.25f;
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
    //    auto buttonRect = bounds.reduced(1.0f, 1.0f);
    
    if(shouldDrawButtonAsDown) {
        buttonRect.translate(0.0f, 1.0f);
    }
    
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(buttonRect, cornerSize);
    
    g.setColour(Colors::Button::outline);
    g.drawRoundedRectangle(buttonRect, cornerSize, 2.0f);
}

void EnabledButtonLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                                 bool shouldDrawButtonAsHighlighted,
                                                 bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
//    auto buttonRect = bounds.reduced(1.0f, 1.0f);
    
    if(shouldDrawButtonAsDown) {
        buttonRect.translate(0.0f, 1.0f);
    }
    
    if(button.getToggleState()) {
        g.setColour(button.findColour(juce::TextButton::textColourOnId));
    } else {
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
    }
    
    g.setFont(Fonts::getFont());
    g.drawText(button.getButtonText(), buttonRect, juce::Justification::centred);
}

ComboBoxLookAndFeel::ComboBoxLookAndFeel()
{
    setColour(juce::ComboBox::textColourId, Colors::ComboBox::text);
    setColour(juce::PopupMenu::textColourId, Colors::ComboBox::text);
    setColour(juce::PopupMenu::highlightedTextColourId, Colors::ComboBox::text);
    
    setColour(juce::ComboBox::backgroundColourId, Colors::ComboBox::background);
    setColour(juce::PopupMenu::backgroundColourId, Colors::ComboBox::background);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::ComboBox::background);
    
    setColour(juce::ComboBox::outlineColourId, Colors::ComboBox::outline);
    setColour(juce::ComboBox::focusedOutlineColourId, Colors::ComboBox::outline);
    setColour(juce::ComboBox::focusedOutlineColourId, Colors::ComboBox::outline);
    
    
    setColour(juce::ComboBox::arrowColourId, Colors::ComboBox::arrow);
    
}
/*
 PopupMenu::backgroundColourId,              currentColourScheme.getUIColour (ColourScheme::UIColour::menuBackground).getARGB(),
 PopupMenu::textColourId,                    currentColourScheme.getUIColour (ColourScheme::UIColour::menuText).getARGB(),
 PopupMenu::headerTextColourId,              currentColourScheme.getUIColour (ColourScheme::UIColour::menuText).getARGB(),
 PopupMenu::highlightedTextColourId,         currentColourScheme.getUIColour (ColourScheme::UIColour::highlightedText).getARGB(),
 PopupMenu::highlightedBackgroundColourId,   currentColourScheme.getUIColour (ColourScheme::UIColour::highlightedFill).getARGB(),
*/

void ComboBoxLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                   int, int, int, int, juce::ComboBox& box)

{
    auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 6.0f;
    juce::Rectangle<int> boxBounds (0, 0, width, height);

//    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
//    g.setColour(Colors::ComboBox::background);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

    g.setColour (Colors::ComboBox::outline);
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 2.0f);

    //arrow
//    juce::Rectangle<int> arrowZone (width - 30, 0, 20, height);
//    juce::Path path;
//    path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
//    path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
//    path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);
//
//    g.setColour (Colors::ComboBox::arrow .withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
//    g.strokePath (path, juce::PathStrokeType (2.0f));
}

juce::Font ComboBoxLookAndFeel::getComboBoxFont([[maybe_unused]] juce::ComboBox&)
{
    return Fonts::getFont();
}

void ComboBoxLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    //x, y, w, h
    auto bounds = box.getLocalBounds().toFloat();
    label.setBounds(1, 0, (int)bounds.getWidth(), (int)bounds.getHeight());
    label.setJustificationType(juce::Justification::centred);
    auto font = getComboBoxFont(box);
    label.setFont(font);
}

void ComboBoxLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                            const bool isSeparator, const bool isActive,
                                            const bool isHighlighted, const bool isTicked,
                                            const bool hasSubMenu, const juce::String& text,
                                            const juce::String& shortcutKeyText,
                                            const juce::Drawable* icon, const juce::Colour* const textColourToUse)
{
    
    if (isSeparator)
    {
//        auto r = area.withWidth(r.getWidth())
        auto r  = area.reduced (5, 0);
        r.removeFromTop (juce::roundToInt (((float) r.getHeight() * 0.5f) - 0.5f));
        
        g.setColour (findColour (juce::PopupMenu::textColourId).withAlpha (0.3f));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        auto textColour = (textColourToUse == nullptr ? findColour (juce::PopupMenu::textColourId)
                           : *textColourToUse);
        
        auto r  = area.reduced (1);
        
        if (isHighlighted && isActive)
        {
            g.setColour (findColour (juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRoundedRectangle(0, 0, (float)r.getWidth(), (float)r.getHeight(), 4.5f);
//            g.fillRect (r);
            
            g.setColour (findColour (juce::PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour (textColour.withMultipliedAlpha (isActive ? 1.0f : 0.5f));
        }
        
        //source code
//        r.reduce (juce::jmin (5, area.getWidth() / 20), 0);
//        g.drawText(text, textArea, juce::Justification::centredLeft);
//        auto textArea = r.withTrimmedRight(20);
//        g.drawText(text, just::Justification::centredLeft);
//        g.drawFittedText (text, textArea, juce::Justification::centredLeft, 1);
//
//
        //drawing menu item text
        auto font = getPopupMenuFont();
        auto maxFontHeight = (float) r.getHeight() / 1.8f;
        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);
        g.setColour(findColour(juce::PopupMenu::textColourId));
        g.setFont (font);
        auto iconArea = r.removeFromLeft (juce::roundToInt (maxFontHeight)).toFloat();
//        auto textArea = r.withTrimmedLeft(5);
        g.drawFittedText(text, area, juce::Justification::centred, 1);
        
        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft (juce::roundToInt (maxFontHeight * 0.5f));
        }
        else if (isTicked)
        {
            auto tick = getTickShape (1.0f);
            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (iconArea.getWidth() / 5, 0).toFloat(), true));
        }
        
        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();
            
            auto x = static_cast<float> (r.removeFromRight ((int) arrowH).getX());
            auto halfH = static_cast<float> (r.getCentreY());
            
            juce::Path path;
            path.startNewSubPath (x, halfH - arrowH * 0.5f);
            path.lineTo (x + arrowH * 0.6f, halfH);
            path.lineTo (x, halfH + arrowH * 0.5f);
            
//            g.strokePath (path, juce::PathStrokeType (2.0f));
        }
        
//        r.removeFromRight (3);
        
        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setColour(findColour(juce::PopupMenu::textColourId));
            g.setFont (f2);
            
            g.drawText (shortcutKeyText, r, juce::Justification::centred, true);
        }
    }
}

void ComboBoxLookAndFeel::drawMenuBarBackground(juce::Graphics &g, int width,
                                                int height, bool, juce::MenuBarComponent &menuBar)
{
//    auto colour = menuBar.findColour (juce::PopupMenu::highlightedBackgroundColourId).withAlpha (0.4f);
//    auto colour = menuBar.findColour (juce::TextButton::buttonColourId).withAlpha (0.4f);
//
//    juce::Rectangle<int> r (width, height);
//
//    g.setColour (colour.contrasting (0.15f));
//    g.fillRect  (r.removeFromTop (1));
//    g.fillRect  (r.removeFromBottom (1));
//
//    g.setGradientFill (juce::ColourGradient::vertical (colour, 0, colour.darker (0.2f), (float) height));
//    g.fillRect (r);
}

void ComboBoxLookAndFeel::getIdealPopupMenuItemSize(const juce::String &text, const bool isSeparator, 
                                                    int standardMenuItemHeight, int &idealWidth, int &idealHeight)
{
    auto font = getPopupMenuFont();
    idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : juce::roundToInt (font.getHeight() * 1.2f);
//    idealWidth = juce::GlyphArrangement::getStringWidthInt(font, text) + idealHeight * 2;
//    if (isSeparator)
//    {
//        idealWidth = 50;
//        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 10 : 10;
//    }
//    else
//    {
//        auto font = getPopupMenuFont();
//
//        if (standardMenuItemHeight > 0 && font.getHeight() > (float) standardMenuItemHeight / 1.3f)
//            font.setHeight ((float) standardMenuItemHeight / 1.3f);
//
//        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : juce::roundToInt (font.getHeight() * 1.3f);
//        idealWidth = juce::GlyphArrangement::getStringWidthInt (font, text) + idealHeight * 2;
//    }
}

void ComboBoxLookAndFeel::drawMenuBarItem(juce::Graphics &g, int width, int height, int itemIndex, const juce::String &itemText, bool isMouseOverItem, bool isMenuOpen, bool, juce::MenuBarComponent &menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColour (menuBar.findColour (juce::TextButton::textColourOffId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColour (juce::TextButton::buttonOnColourId));
        g.setColour (menuBar.findColour (juce::TextButton::textColourOnId));
    }
    else
    {
        g.setColour (menuBar.findColour (juce::TextButton::textColourOffId));
    }

    g.setColour(menuBar.findColour(juce::ComboBox::textColourId));
//    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, juce::Justification::centredLeft, 1);
}
void ComboBoxLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll(findColour(juce::ComboBox::outlineColourId));
    
    const float cornerSize = 5.0f; // Adjust corner radius as needed

    juce::Path backgroundPath;
    backgroundPath.addRoundedRectangle(0, 0, width, height, cornerSize);
    
    // Draw a border with rounded corners
    g.setColour(findColour(juce::ComboBox::outlineColourId)); // Replace with desired border color
    g.strokePath(backgroundPath, juce::PathStrokeType(2.0f)); // Adjust thickness as needed
}
