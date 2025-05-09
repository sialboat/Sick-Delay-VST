# Sick Delay
Sick Delay is a work-in-progress Delay Plugin that offers impeccable flexibility of sonic manipulation. The plugin features three distinct sections, a dual-delay line, a multi-effect, and an output section. The delay section features two delay lines with flexible delay tap controls, allowing users to switch delay algorithms (from a varispeed emulation to a crossfade delay), offset delay taps, and sync to a DAW's internal clock. The delay's feedback allows the user to crossfade between a LR ping-pong, a normal delay, and a RL ping pong, with the addition of filters to further shape the sound. The user can insert an array of effects ranging from distortions to pitch shifters within the feedback line, solely the dry signal, or both to further color the sound. The output section features a meter, and options for the user to change the delay line's interpolation algorithm, adjust the mix of the plugin, and the option to add a soft or hard clipper to the very end of the signal chain.

This Delay Plugin has been based off of the delay plugin from the tutorial book ["The Complete Beginner's Guide to Audio Plugin Development"](https://www.theaudioprogrammer.com/books/beginners-plugin-book). I reccommend everybody pick up that book or follow along the online JUCE tutorials for anybody interested in figuring out how to do this on your own.

For more information or diagrams, see the attached documents in the "Media & Diagrams" folder.

## Building
Because this project is currently in development, a lot of planned features are currently missing. However if you would like to build a VST yourself, follow the given instructions (which may or may not work):

1) Download Projucer from [the Juce website](https://juce.com/)
2) Fork the content of this GitHub
3) Load the projucer project in Visual Studio (Windows) or XCode (Mac)
4) Build a VST / AU from the IDE.

### Effects
#### Distortion
**Analog/Digital:** crossfades between a $\tanh(t)$ transfer curve and a scrappier nonlinear "digital" curve, with the option to bias the signal.
**Odd/Even:** a specially designed soft-clipping transfer curve that allows the user to crossfade between odd and even harmonics.
**Swell:** a unique nonlinear transfer curve.
**Deci/Crush:** A crossfade between a decimator and a bitcrusher, allowing for thorough signal destruction!

#### Modulation
**AM/RM/FM:**
**Pitch Shifter**
**Freq. Shifter (CWO)**
**Allpass / Disperser**

#### Time
**Chorus**
**Phaser**
**Flanger**
**Marbles**
**Hybrid Reverb**

#### Ext. Out
(Long-term feature) allows the user to send the signal out to a custom signal chain of their own.
## Feature Roadmap
Because this project is currently in development, a lot of planned features are currently missing. Here are a list of the currently implemented features (i.e. core delay vst functionality):
- Crossfade / Varispeed delay algorithm
- Delay Interpolation selector
- Soft clip / Hard clip enable for the output signal
- Delay tap spread for individual delay taps (adds and subtracts an amount to both delay taps)
- Filter processing in the feedback section
- Ping-Pong implementation
- Tempo Sync with DAW

All of the features below are the features planned for the plugin to be fully finished. It is unlikely that there will be any other features that will be added on top of these in future updates.
### General
- Windows and non-intel versions of the plugin
- Proper feedback line interaction
- Auto-Gain button for effects
- Finish all the effects
- reformat audio parameters to audioParameterGroups for each section (for AU plugins)
- VST, AU, CLAP support (AAX is a long shot; mostly because I don't know how) 
### Effects
- Different Reverb Algorithms
- jitter feedback (randomise delay duration per tap)
- inertia
- Ext. Out
- Phaser, Flanger (Low prio Time Effects)
- Sidechain input for the AM/RM/FM signal
- Marbles (a nested delay) (Time effect)
- Multi-Filter (Time effect)
- OTT (maybe)
### Misc.
- User Manual
- Settings Tab (For delay interpolation, oversampling, light/dark mode, user manual link)
- UI Overhaul
- Light and Dark themes
- Name Change
