/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
_3BandEqAudioProcessorEditor::_3BandEqAudioProcessorEditor(_3BandEqAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
peakFreqKnobAtt(audioProcessor.apvts, "Peak Freq", peakFreqKnob),
peakGainKnobAtt(audioProcessor.apvts, "Peak Gain", peakGainKnob),
peakQualityKnobAtt(audioProcessor.apvts, "Peak Quality", peakQualityKnob),
lowCutFreqKnobAtt(audioProcessor.apvts, "LowCut Freq", lowCutFreqKnob),
lowCutSlopeKnobAtt(audioProcessor.apvts, "LowCut Slope", lowCutSlopeKnob),
highCutFreqKnobAtt(audioProcessor.apvts, "HighCut Freq", highCutFreqKnob),
highCutSlopeKnobAtt(audioProcessor.apvts, "HighCut Slope", highCutSlopeKnob)



{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComponents()) 
    {
        addAndMakeVisible(comp);
    }

    setSize (600, 500);
}

_3BandEqAudioProcessorEditor::~_3BandEqAudioProcessorEditor()
{
}

//==============================================================================
void _3BandEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void _3BandEqAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..


    // Get Window Bounds
    auto bounds = getLocalBounds();
    
    auto audioCurveArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    // Place Low Cut Knobs
    lowCutFreqKnob.setBounds( lowCutArea.removeFromTop(bounds.getHeight() * 0.5) );
    lowCutSlopeKnob.setBounds(lowCutArea);


    // Place High Cut Knobs
    highCutFreqKnob.setBounds(highCutArea.removeFromTop(bounds.getHeight() * 0.5));
    highCutSlopeKnob.setBounds(highCutArea);

    // Place Peak Knobs
    peakFreqKnob.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainKnob.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualityKnob.setBounds(bounds);

}

std::vector<juce::Component*> _3BandEqAudioProcessorEditor::getComponents()
{
    return
    {
        &peakFreqKnob,
        &peakGainKnob,
        &peakQualityKnob,
        &lowCutFreqKnob,
        &highCutFreqKnob,
        &lowCutSlopeKnob,
        &highCutSlopeKnob
    };
}
