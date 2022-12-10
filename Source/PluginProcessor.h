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


//==============================================================================
/**
*/
class _3BandEqAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
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

    void updatePeakFilter(const ChainParameters& chainParameters);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    template<int Idx, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& lowCutCoefficients)
    {
        updateCoefficients(chain.template get<Idx>().coefficients, lowCutCoefficients[Idx]);
        chain.template setBypassed<Idx>(false);
    }

    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut, const CoefficientType& lowCutCoefficients, const Slope& lowCutSlope)
    {
        // LowCut Coefficients ---------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------

        // Bypass slope values (0, 1, 2, 3)
        leftLowCut.setBypassed<0>(true);
        leftLowCut.setBypassed<1>(true);
        leftLowCut.setBypassed<2>(true);
        leftLowCut.setBypassed<3>(true);

        // Activate slope
        switch (lowCutSlope)
        {
            case Slope_48:
            {
                update<3>(leftLowCut, lowCutCoefficients);
            }
            case Slope_36:
            {
                update<2>(leftLowCut, lowCutCoefficients);
            }
            case Slope_24:
            {
                update<1>(leftLowCut, lowCutCoefficients);
            }
            case Slope_12:
            {
                update<0>(leftLowCut, lowCutCoefficients);
            }
        }
    }

    void updateLowCutFilter(const ChainParameters& chainParameters);
    void updateHighCutFilter(const ChainParameters& chainParameters);
    void updateFilters();

    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout()};

private:

    // Create a left and right MonoChain instance to do Stereo Processing
    MonoChain leftChain, rightChain;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_3BandEqAudioProcessor)
};
