/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


// Custom knob class
struct CustomKnob : juce::Slider
{
    CustomKnob() : juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow)
    {
        juce::Slider::ColourIds::textBoxOutlineColourId;

        juce:Slider:ColourIds::rotarySliderFillColourId;

    };
};


//==============================================================================
/**
*/
class _3BandEqAudioProcessorEditor  : public juce::AudioProcessorEditor,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
public:
    _3BandEqAudioProcessorEditor (_3BandEqAudioProcessor&);
    ~_3BandEqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;



    void parameterValueChanged(int parameterIndex, float newValue) override;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called with gestureIsStarting
        being true when they first press the mouse button, and it will be called again with
        gestureIsStarting being false when they release it.

        IMPORTANT NOTE: This will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.
    */
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    void timerCallback() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    _3BandEqAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged { false };

    // Declare Knobs
    CustomKnob peakFreqKnob, peakGainKnob, peakQualityKnob, lowCutFreqKnob, highCutFreqKnob, lowCutSlopeKnob, highCutSlopeKnob;

    // Alias Attachment
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    // Declare Attachments
    Attachment peakFreqKnobAtt, peakGainKnobAtt, peakQualityKnobAtt, lowCutFreqKnobAtt, lowCutSlopeKnobAtt, highCutFreqKnobAtt, highCutSlopeKnobAtt;



    // Create vector for knobs
    std::vector<juce::Component*> getComponents();

    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_3BandEqAudioProcessorEditor)
};
