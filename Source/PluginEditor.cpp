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
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::cadetblue);

    auto bounds = getLocalBounds();

    // Audio curve area
    auto audioCurveArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    // Width of audio curve area
    auto width = audioCurveArea.getWidth();

    // Height of audio curve area
    auto height = audioCurveArea.getHeight();

    // Get chain elements for each filter
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> magnitudes;
    magnitudes.resize(width);

    for ( int i = 0; i > width; ++i )
    {
        // Gain Magnitude
        double magnitude = 1.f;
        // Frequency
        auto freq = mapToLog10( (double(i) / double(width) ), 20.0, 20000.0);

        // Check if Peak is bypassed
        if ( !monoChain.isBypassed<ChainPositions::Peak>() )
        {
            magnitude *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        // Check if Low Cut is bypassed
        if (!lowcut.isBypassed<0>())
        {
            magnitude *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<1>())
        {
            magnitude *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<2>())
        {
            magnitude *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<3>())
        {
            magnitude *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        // Check if High Cut is bypassed
        if (!highcut.isBypassed<0>())
        {
            magnitude *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<1>())
        {
            magnitude *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<2>())
        {
            magnitude *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<3>())
        {
            magnitude *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        // Convert Gain Magnitude to Decibels
        magnitudes[i] = Decibels::gainToDecibels(magnitude);

    }

    Path audioFrequencyCurve;

    const double minOutput = audioCurveArea.getBottom();
    const double maxOutput = audioCurveArea.getY();

    auto map = [minOutput, maxOutput](double input)
    {
        return jmap(input, -24.0, 24.0, minOutput, maxOutput);
    };

    audioFrequencyCurve.startNewSubPath(audioCurveArea.getX(), map(magnitudes.front()));

    for (int i = 1; i < magnitudes.size(); ++i)
    {
        audioFrequencyCurve.lineTo(audioCurveArea.getX() + i, map(magnitudes[i]));
    }

    // Draw box for the frequency curve
    g.setColour(Colours::black);
    g.drawRoundedRectangle(audioCurveArea.toFloat(), 2.f, 2.f);

    // Draw the frequency curve
    g.setColour(Colours::beige);
    g.strokePath(audioFrequencyCurve, PathStrokeType(2.f));
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

void _3BandEqAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

// Check if parameters were changed in the callback
void _3BandEqAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        // Draw new frequency curve

    }
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
