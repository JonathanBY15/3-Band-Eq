/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
_3BandEqAudioProcessor::_3BandEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

_3BandEqAudioProcessor::~_3BandEqAudioProcessor()
{
}

//==============================================================================
const juce::String _3BandEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool _3BandEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool _3BandEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool _3BandEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double _3BandEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int _3BandEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int _3BandEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void _3BandEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String _3BandEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void _3BandEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void _3BandEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Prepare processing chain
    juce::dsp::ProcessSpec spec;
    
    // Max number of samples that will be processed at once
    spec.maximumBlockSize = samplesPerBlock;

    // Number of channels
    spec.numChannels = 1;

    // Sample Rate
    spec.sampleRate = sampleRate;

    // Pass spec to each chain
    leftChain.prepare(spec);
    rightChain.prepare(spec);



    // Peak Coefficients-------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------
    auto chainParameters = getChainParameters(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, 
        chainParameters.peakFreq, 
        chainParameters.peakQuality, 
        juce::Decibels::decibelsToGain(chainParameters.peakGain)
    );

    // Access processing chain links
    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;



    // LowCut Coefficients ---------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainParameters.lowCutFreq, 
        sampleRate, 
        (chainParameters.lowCutSlope + 1) * 2
    );

    // Initialize left chain
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();

    // Bypass slope values (0, 1, 2, 3)
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.lowCutSlope) 
    {
        case Slope_12:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<0>(false);
            break;
        }
    }

    // Initialize right chain
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    // Bypass slope values (0, 1, 2, 3)
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.lowCutSlope)
    {
        case Slope_12:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<0>(false);
            break;
        }
    }

    // HighCut Coefficients ---------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainParameters.highCutFreq,
        getSampleRate(),
        (chainParameters.highCutSlope + 1) * 2
    );

    // Initialize left chain
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();

    // Bypass slope values (0, 1, 2, 3)
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.highCutSlope)
    {
        case Slope_12:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<0>(false);
            break;
        }
    }

    // Initialize right chain
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    // Bypass slope values (0, 1, 2, 3)
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.highCutSlope)
    {
        case Slope_12:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
            rightHighCut.setBypassed<0>(false);
            break;
        }
    }


}

void _3BandEqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _3BandEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void _3BandEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());




    // Peak Coefficients-------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------
    auto chainParameters = getChainParameters(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        getSampleRate(),
        chainParameters.peakFreq,
        chainParameters.peakQuality,
        juce::Decibels::decibelsToGain(chainParameters.peakGain)
    );

    // Access processing chain links
    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;



    // LowCut Coefficients ---------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainParameters.lowCutFreq,
        getSampleRate(),
        (chainParameters.lowCutSlope + 1) * 2
    );

    // Initialize left chain
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();

    // Bypass slope values (0, 1, 2, 3)
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.lowCutSlope)
    {
        case Slope_12:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<0>(false);
            break;
        }
    }

    // Initialize right chain
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    // Bypass slope values (0, 1, 2, 3)
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.lowCutSlope)
    {
        case Slope_12:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<0>(false);
            break;
        }
    }



    // HighCut Coefficients ---------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------------------------

    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainParameters.highCutFreq,
        getSampleRate(),
        (chainParameters.highCutSlope + 1) * 2
    );

    // Initialize left chain
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();

    // Bypass slope values (0, 1, 2, 3)
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.highCutSlope)
    {
        case Slope_12:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<0>(false);
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<0>(false);
            break;
        }
    }

    // Initialize right chain
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    // Bypass slope values (0, 1, 2, 3)
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    // Activate slope
    switch (chainParameters.highCutSlope)
    {
        case Slope_12:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            break;
        }case Slope_36:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<0>(false);
            break;
        }case Slope_48:
        {
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<0>(false);
            *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
            rightHighCut.setBypassed<0>(false);
            break;
        }
    }




    // Create audio block
    juce::dsp::AudioBlock<float> block(buffer);

    // Represent left and right channels with audio blocks
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    // Left and Right context
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    // Pass context to left and right filter chains
    leftChain.process(leftContext);
    rightChain.process(rightContext);


}

//==============================================================================
bool _3BandEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* _3BandEqAudioProcessor::createEditor()
{
    return new _3BandEqAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void _3BandEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void _3BandEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainParameters getChainParameters(juce::AudioProcessorValueTreeState& apvts)
{
    ChainParameters parameters;

    parameters.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    parameters.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    parameters.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    parameters.peakGain = apvts.getRawParameterValue("Peak Gain")->load();
    parameters.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    parameters.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    parameters.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    return parameters;
}


juce::AudioProcessorValueTreeState::ParameterLayout _3BandEqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Low Cut Frequency
    // (20 Hz to 20000 Hz), (1 interval Value) (Skew 0.25) (Start at 20Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq", 
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f, false),
                                                           20.f));

    // High Cut Frequency
    // (20 Hz to 20000 Hz), (1 interval Value) (Skew 0.25) (Start at 20000Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f, false),
                                                           20000.f));

    // Peak Frequency
    // (20 Hz to 20000 Hz), (1 interval Value) (Skew 0.25) (Start at 600Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f, false),
                                                           600.f));

    // Peak Gain
    // (-24 DB to 24 DB), (0.5 interval Value) (No Skew[1.f]) (Start at 0 DB)
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f, false),
                                                           0.0f));
    // Peak Quality
    // (0.1 Q to 10.0 Q), (0.05 interval Value) (No Skew[1.f]) (Start at 1 Q)
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.f, false),
                                                           1.f));

    // String Array that holds db/Oct Values
    juce::StringArray db_per_octave;
    for ( int i = 0; i < 4; ++i ) {
        juce::String str;
        str << (12 + 12*i);
        str << " db/Oct";
        db_per_octave.add(str);
    }

    // Low Cut and High Cut Steepness (12, 24, 36, 48 db/Oct)
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", db_per_octave, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", db_per_octave, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new _3BandEqAudioProcessor();
}
