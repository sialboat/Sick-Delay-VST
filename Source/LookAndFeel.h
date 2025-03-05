/*
  ==============================================================================

    LookAndFeel.h
    Created: 30 Dec 2024 3:09:07pm
    Author:  Silas Wang

  ==============================================================================
*/

#pragma once

namespace Colors
{
    const juce::Colour background {47, 47, 47}; //Mauve
    const juce::Colour header {64, 64, 64};
    const juce::Colour bypassedFarthest {juce::uint8(31), juce::uint8(31), juce::uint8(31), juce::uint8(192)};
    const juce::Colour bypassedCenter {juce::uint8(120), juce::uint8(120), juce::uint8(120), juce::uint8(198)};
    const juce::Colour bypassedSum {juce::uint8(112), juce::uint8(112), juce::uint8(112), juce::uint8(128)};

    namespace Knob
    {
        const juce::Colour trackBackground {205, 200, 195};
        const juce::Colour trackActive { 177, 101, 135 }; //change this to Mauve
        const juce::Colour outline { 255, 250, 245 };
        const juce::Colour gradientTop { 250, 245, 240 };
        const juce::Colour gradientBottom { 240, 235, 230 };
        const juce::Colour dial { 100, 100, 100 };
        const juce::Colour dropShadow { 195, 190, 185 };
        const juce::Colour label { 80, 80, 80 };
        const juce::Colour textBoxBackground { 80, 80, 80 };
        const juce::Colour value { 240, 240, 240 };
        const juce::Colour caret { 255, 255, 255 };
    }
    namespace Group
    {
        const juce::Colour label {160, 155, 150};
        const juce::Colour outline {235, 230, 225};
    }
    namespace Button
    {
        const juce::Colour text {80, 80, 80};
        const juce::Colour textToggled {40, 40, 40};
        const juce::Colour background {245, 240, 235};
        const juce::Colour backgroundToggled {255, 250, 245};
        const juce::Colour outline {235, 230, 225};
    }
    namespace LevelMeter
    {
        const juce::Colour background {245, 240, 235};
        const juce::Colour tickLine {200, 200, 200};
        const juce::Colour tickLabel {80, 80, 80};
        const juce::Colour tooLoud {226, 74, 81};
        const juce::Colour levelOK {65, 206, 88};
    }
    namespace ComboBox
    {
//        const juce::Colour background {245, 240, 235};
        const juce::Colour background {255, 250, 245};
        const juce::Colour arrow {225, 220, 215};
        const juce::Colour outline {235, 230, 225};
        const juce::Colour text {80, 80, 80};
    }
}

//rotary knob look and feel
class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RotaryKnobLookAndFeel();
    
    static RotaryKnobLookAndFeel* get()
    {
        static RotaryKnobLookAndFeel instance;
        return &instance;
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
    
    juce::Font getLabelFont(juce::Label&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
    
    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}
    
    void fillTextEditorBackground(juce::Graphics&, int width, int height, juce::TextEditor&) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobLookAndFeel)
    
    juce::DropShadow dropShadow { Colors::Knob::dropShadow, 6, { 0, 3 } };
};


//font class
class Fonts
{
public:
    Fonts() = delete;
    static juce::Font getFont(float height = 16.0f);
private:
    static const juce::Typeface::Ptr typeface;
};

//main look and feel class
class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MainLookAndFeel();
    
    juce::Font getLabelFont(juce::Label&) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};


//button look and feel class
class ButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ButtonLookAndFeel();
    static ButtonLookAndFeel* get()
    {
        static ButtonLookAndFeel instance;
        return &instance;
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};

class HalfRoundedButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HalfRoundedButtonLookAndFeel();
    static HalfRoundedButtonLookAndFeel* get()
    {
        static HalfRoundedButtonLookAndFeel instance;
        return &instance;
    }
    
    void setRoundedSide(bool b)
    {
        roundedSide = b;
    }
//    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
//                     bool shouldDrawButtonAsDown) override;
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
    
private:
    bool roundedSide = false; //left == FALSE, right == TRUE. By default this is false
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HalfRoundedButtonLookAndFeel)
};

class EnabledButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    EnabledButtonLookAndFeel();
    static EnabledButtonLookAndFeel* get()
    {
        static EnabledButtonLookAndFeel instance;
        return &instance;
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnabledButtonLookAndFeel)
};


//we need a dropdown button look and feel class here too
class ComboBoxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ComboBoxLookAndFeel();
    static ComboBoxLookAndFeel* get()
    {
        static ComboBoxLookAndFeel instance;
        return &instance;
    }
    
//    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& c) override;
    
    //definitely not me trying to figure out how to change each aspect of the comboBox
    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override;
    
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    
    int getPopupMenuBorderSize() override
    {
        return 0;
    }
    
//    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            const bool isSeparator, const bool isActive,
                            const bool isHighlighted, const bool isTicked,
                            const bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* const textColourToUse) override;
//    juce::Font getPopupMenuFont() override;
    
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    
    void drawMenuBarBackground (juce::Graphics& g, int width, int height,
                                bool, juce::MenuBarComponent& menuBar) override;
    
    void getIdealPopupMenuItemSize(const juce::String& text, const bool isSeparator,
                                   int standardMenuItemHeight, int& idealWidth, int& idealHeight) override;
    
    void drawMenuBarItem(juce::Graphics& g, int width, int height,
                         int itemIndex, const juce::String& itemText,
                         bool isMouseOverItem, bool isMenuOpen,
                         bool /*isMouseOverBar*/, juce::MenuBarComponent& menuBar) override;
    
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    
//    void draw
//    juce::Font getComboFontBox (juce::ComboBox& c, juce::Label& l);
//    void drawComboBoxBackground(juce::Graphics g, juce::ComboBox comboBox, const juce::Colour& backgroundColour,) override;
};
/*
 drawComboBox from lookAndFeel v4
 auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
 Rectangle<int> boxBounds (0, 0, width, height);

 g.setColour (box.findColour (ComboBox::backgroundColourId));
 g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

 g.setColour (box.findColour (ComboBox::outlineColourId));
 g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

 Rectangle<int> arrowZone (width - 30, 0, 20, height);
 Path path;
 path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
 path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
 path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

 g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
 g.strokePath (path, PathStrokeType (2.0f));
 */
