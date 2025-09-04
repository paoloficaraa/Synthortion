#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    AudioPluginAudioProcessor::AudioPluginAudioProcessor()
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                             ),
          apvts(*this, nullptr, "Parameters", createParameterLayout())
    {
    }

    AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
    {
    }

    //==============================================================================
    const juce::String AudioPluginAudioProcessor::getName() const
    {
        return "Synthortion";
    }

    bool AudioPluginAudioProcessor::acceptsMidi() const
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool AudioPluginAudioProcessor::producesMidi() const
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    bool AudioPluginAudioProcessor::isMidiEffect() const
    {
#if JucePlugin_IsMidiEffect
        return true;
#else
        return false;
#endif
    }

    double AudioPluginAudioProcessor::getTailLengthSeconds() const
    {
        return 0.0;
    }

    int AudioPluginAudioProcessor::getNumPrograms()
    {
        return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
                  // so this should be at least 1, even if you're not really implementing programs.
    }

    int AudioPluginAudioProcessor::getCurrentProgram()
    {
        return 0;
    }

    void AudioPluginAudioProcessor::setCurrentProgram(int index)
    {
        juce::ignoreUnused(index);
    }

    const juce::String AudioPluginAudioProcessor::getProgramName(int index)
    {
        juce::ignoreUnused(index);
        return {};
    }

    void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName)
    {
        juce::ignoreUnused(index, newName);
    }

    //==============================================================================
    void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumInputChannels()};
        warmDistortion.prepare(spec);
        parametricEQ.prepare(spec);

        // Initialize RMS level smoothing
        inputRmsLevel.reset(sampleRate, 0.1); // 100ms smoothing
        outputRmsLevel.reset(sampleRate, 0.1);
        inputRmsLevel.setCurrentAndTargetValue(-60.0f);
        outputRmsLevel.setCurrentAndTargetValue(-60.0f);
    }

    void AudioPluginAudioProcessor::releaseResources()
    {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
    }

    bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
    {
#if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
#else
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        // Some plugin hosts, such as certain GarageBand versions, will only
        // load plugins that support stereo bus layouts.
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
#endif

        return true;
#endif
    }

    void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                                 juce::MidiBuffer &midiMessages)
    {
        juce::ignoreUnused(midiMessages);

        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // Skip RMS processing ahead
        inputRmsLevel.skip(buffer.getNumSamples());
        outputRmsLevel.skip(buffer.getNumSamples());

        // Update distortion parameters from APVTS
        auto driveValue = apvts.getRawParameterValue("DRIVE")->load();
        auto mixValue = apvts.getRawParameterValue("MIX")->load();
        auto saturationTypeValue = apvts.getRawParameterValue("SATURATION_TYPE")->load();

        // Calculate input RMS level
        float inputRms = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            inputRms += buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
        }
        inputRms /= static_cast<float>(buffer.getNumChannels());
        inputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(inputRms, -60.0f));

        warmDistortion.setDrive(driveValue);
        warmDistortion.setMix(mixValue);
        warmDistortion.setSaturationType(static_cast<WarmDistortion::SaturationType>(static_cast<int>(saturationTypeValue))); // Process the audio through the distortion effect
        warmDistortion.process(buffer);

        // Update EQ parameters from APVTS
        auto lowCutFreq = apvts.getRawParameterValue("LOW_CUT_FREQ")->load();
        auto lowCutQ = apvts.getRawParameterValue("LOW_CUT_Q")->load();
        auto lowMidFreq = apvts.getRawParameterValue("LOW_MID_FREQ")->load();
        auto lowMidGain = apvts.getRawParameterValue("LOW_MID_GAIN")->load();
        auto lowMidQ = apvts.getRawParameterValue("LOW_MID_Q")->load();
        auto highMidFreq = apvts.getRawParameterValue("HIGH_MID_FREQ")->load();
        auto highMidGain = apvts.getRawParameterValue("HIGH_MID_GAIN")->load();
        auto highMidQ = apvts.getRawParameterValue("HIGH_MID_Q")->load();
        auto highCutFreq = apvts.getRawParameterValue("HIGH_CUT_FREQ")->load();
        auto highCutQ = apvts.getRawParameterValue("HIGH_CUT_Q")->load();

        parametricEQ.setLowCut(lowCutFreq, lowCutQ);
        parametricEQ.setLowMid(lowMidFreq, lowMidGain, lowMidQ);
        parametricEQ.setHighMid(highMidFreq, highMidGain, highMidQ);
        parametricEQ.setHighCut(highCutFreq, highCutQ);

        // Apply EQ post-distortion
        parametricEQ.process(buffer);

        // Calculate output RMS level
        float outputRms = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            outputRms += buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
        }
        outputRms /= static_cast<float>(buffer.getNumChannels());
        outputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(outputRms, -60.0f));

        // Send audio data to spectrum analyzer (post-EQ)
        if (spectrumAnalyzerCallback && buffer.getNumChannels() > 0)
        {
            auto *channelData = buffer.getReadPointer(0);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                spectrumAnalyzerCallback(channelData[sample]);
            }
        }
    }

    //==============================================================================
    bool AudioPluginAudioProcessor::hasEditor() const
    {
        return true; // (change this to false if you choose to not supply an editor)
    }

    juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor()
    {
        return new AudioPluginAudioProcessorEditor(*this);
    }

    //==============================================================================
    void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
    {
        // You should use this method to store your parameters in the memory block.
        // You could do that either as raw data, or use the XML or ValueTree classes
        // as intermediaries to make it easy to save and load complex data.
        juce::ignoreUnused(destData);
    }

    void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
    {
        // You should use this method to restore your parameters from this memory block,
        // whose contents will have been created by the getStateInformation() call.
        juce::ignoreUnused(data, sizeInBytes);
    }

    juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 0.0f, 1.0f, 0.3f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f, 1.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("INPUT_GAIN", "Input Gain", -24.0f, 24.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("OUTPUT_GAIN", "Output Gain", -24.0f, 24.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY", "Delay", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("CHORUS", "Chorus", 0.0f, 1.0f, 0.0f));

        // Saturation type parameter (0 = SMOOTH, 1 = TUBE, 2 = TAPE)
        layout.add(std::make_unique<juce::AudioParameterChoice>("SATURATION_TYPE", "Saturation Type",
                                                                juce::StringArray{"Smooth", "Tube", "Tape"}, 0));

        // EQ Parameters
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_CUT_FREQ", "Low Cut Freq", 20.0f, 1000.0f, 20.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_CUT_Q", "Low Cut Q", 0.1f, 10.0f, 0.7f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_FREQ", "Low Mid Freq", 100.0f, 2000.0f, 350.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_GAIN", "Low Mid Gain", -15.0f, 15.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_Q", "Low Mid Q", 0.1f, 10.0f, 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_FREQ", "High Mid Freq", 1000.0f, 8000.0f, 3800.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_GAIN", "High Mid Gain", -15.0f, 15.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_Q", "High Mid Q", 0.1f, 10.0f, 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_CUT_FREQ", "High Cut Freq", 5000.0f, 20000.0f, 20000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_CUT_Q", "High Cut Q", 0.1f, 10.0f, 0.7f));

        return layout;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}