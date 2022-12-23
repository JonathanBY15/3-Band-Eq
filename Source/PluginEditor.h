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

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    _3BandEqAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged { false };

    MonoChain monoChain;

    // Declare Knobs
    CustomKnob peakFreqKnob, peakGainKnob, peakQualityKnob, lowCutFreqKnob, highCutFreqKnob, lowCutSlopeKnob, highCutSlopeKnob;

    // Alias Attachment
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    // Declare Attachments
    Attachment peakFreqKnobAtt, peakGainKnobAtt, peakQualityKnobAtt, lowCutFreqKnobAtt, lowCutSlopeKnobAtt, highCutFreqKnobAtt, highCutSlopeKnobAtt;



    // Create vector for knobs
    std::vector<juce::Component*> getComponents();

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_3BandEqAudioProcessorEditor)
};
