/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// Represent all parameter values
struct ChainParameters
{
    float peakFreq{ 0 }, peakGain{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{ Slope::Slope_12 }, highCutSlope{ Slope::Slope_12 };
};

// Get parameter values
ChainParameters getChainParameters(juce::AudioProcessorValueTreeState& apvts);

// Peak Filter
using Filter = juce::dsp::IIR::Filter<float>;

// Cut Filter (A Filter for each db/Oct value)
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

// Mono signal path (Low Cut, Peak, High Cut)
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

// Declare positions of links in processing chain
enum ChainPositions
{
    LowCut,
    Peak,
    HighCut
};

using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

// Create Peak Filter
Coefficients createPeakFilter(const ChainParameters& chainParameters, double sampleRate);

template<int Idx, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& lowCutCoefficients)
{
    updateCoefficients(chain.template get<Idx>().coefficients, lowCutCoefficients[Idx]);
    chain.template setBypassed<Idx>(false);
}

// Cut Coefficients
template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain, const CoefficientType& coefficients, const Slope& slope)
{
    // LowCut Coefficients ---------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------

    // Bypass slope values (0, 1, 2, 3)
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);

    // Activate slope
    switch (slope)
    {
        case Slope_48:
        {
            update<3>(chain, coefficients);
        }
        case Slope_36:
        {
            update<2>(chain, coefficients);
        }
        case Slope_24:
        {
            update<1>(chain, coefficients);
        }
        case Slope_12:
        {
            update<0>(chain, coefficients);
        }
    }
}

// Create Low Cut Filter
inline auto createLowCutFilter(const ChainParameters& chainParameters, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainParameters.lowCutFreq,
        sampleRate,
        (chainParameters.lowCutSlope + 1) * 2
    );
}

// Create High Cut Filter
inline auto createHighCutFilter(const ChainParameters& chainParameters, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainParameters.highCutFreq,
        sampleRate,
        (chainParameters.highCutSlope + 1) * 2
    );
}








//==============================================================================
/**
*/
class _3BandEqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    _3BandEqAudioProcessor();
    ~_3BandEqAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout()};

private:

    // Create a left and right MonoChain instance to do Stereo Processing
    MonoChain leftChain, rightChain;

    void updatePeakFilter(const ChainParameters& chainParameters);

    


    void updateLowCutFilter(const ChainParameters& chainParameters);
    void updateHighCutFilter(const ChainParameters& chainParameters);

    void updateFilters();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_3BandEqAudioProcessor)
};
