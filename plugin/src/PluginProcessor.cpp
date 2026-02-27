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
        driveParam = apvts.getRawParameterValue("DRIVE");
        inputGainParam = apvts.getRawParameterValue("INPUT_GAIN");
        outputGainParam = apvts.getRawParameterValue("OUTPUT_GAIN");
        colorParam = apvts.getRawParameterValue("COLOR");
        bitCrushParam = apvts.getRawParameterValue("BITCRUSH");
        dacNoiseParam = apvts.getRawParameterValue("DAC_NOISE");
        delayTimeParam = apvts.getRawParameterValue("DELAY_TIME");
        delayMixParam = apvts.getRawParameterValue("DELAY_MIX");
        delayFeedbackParam = apvts.getRawParameterValue("DELAY_FEEDBACK");
        chorusMixParam = apvts.getRawParameterValue("CHORUS_MIX");
        eqBypassParam = apvts.getRawParameterValue("EQ_BYPASS");
        volumeCompParam = apvts.getRawParameterValue("VOLUME_COMPENSATION");

        lowCutFreqParam = apvts.getRawParameterValue("LOW_CUT_FREQ");
        lowCutQParam = apvts.getRawParameterValue("LOW_CUT_Q");
        lowMidFreqParam = apvts.getRawParameterValue("LOW_MID_FREQ");
        lowMidGainParam = apvts.getRawParameterValue("LOW_MID_GAIN");
        lowMidQParam = apvts.getRawParameterValue("LOW_MID_Q");
        highMidFreqParam = apvts.getRawParameterValue("HIGH_MID_FREQ");
        highMidGainParam = apvts.getRawParameterValue("HIGH_MID_GAIN");
        highMidQParam = apvts.getRawParameterValue("HIGH_MID_Q");
        highCutFreqParam = apvts.getRawParameterValue("HIGH_CUT_FREQ");
        highCutQParam = apvts.getRawParameterValue("HIGH_CUT_Q");
    }

    AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
    {
        spectrumAnalyzerCallback = nullptr;
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
        juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels()};

        warmDistortion.prepare(spec);
        bitCrusher.prepare(spec);
        parametricEQ.prepare(spec);
        chorus.prepare(spec);
        pingPongDelay.prepare(spec);

        // Prepare global dry/wet mixer with larger buffer for latency compensation
        juce::dsp::ProcessSpec globalMixerSpec = spec;
        globalMixerSpec.maximumBlockSize = spec.maximumBlockSize * kLatencyBufferMargin;
        globalDryWet.prepare(globalMixerSpec);
        globalDryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        globalDryWet.reset();

        // Prepare dry buffer for latency-matched signal
        delayMatchedDryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
        delayMatchedDryBuffer.clear();

        driveSmoother.reset(sampleRate, kSmootherRampTime);
        driveSmoother.setCurrentAndTargetValue(driveParam->load());
        
        inputGainSmoother.reset(sampleRate, kSmootherRampTime);
        inputGainSmoother.setCurrentAndTargetValue(inputGainParam->load());
        
        outputGainSmoother.reset(sampleRate, kSmootherRampTime);
        outputGainSmoother.setCurrentAndTargetValue(outputGainParam->load());

        const int distortionLatency = juce::jmax(1, warmDistortion.getLatencySamples());

        // Calculate initial latency
        const int eqLatency = parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
        currentTotalLatency.store(totalLatency);
        setLatencySamples(totalLatency);

        updateAllDSPParameters();
    }

    void AudioPluginAudioProcessor::releaseResources()
    {
    }

    bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
    {
#if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
#else
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

#if !JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
#endif

        return true;
#endif
    }

    float AudioPluginAudioProcessor::calculateRMS(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
            return 0.0f;

        float rmsSum = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            rmsSum += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        
        return rmsSum / static_cast<float>(buffer.getNumChannels());
    }

    void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
        [[maybe_unused]] juce::MidiBuffer &midiMessages)
    {
        juce::ignoreUnused(midiMessages);

        if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
            return;

        juce::ScopedNoDenormals noDenormals;

        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        const float color = colorParam->load(std::memory_order_relaxed);
        const float driveBase = driveParam->load(std::memory_order_relaxed);
        const float inputGain = inputGainParam->load(std::memory_order_relaxed);
        const float outputGain = outputGainParam->load(std::memory_order_relaxed);
        const float bitCrush = bitCrushParam->load(std::memory_order_relaxed);
        const float dacNoise = dacNoiseParam->load(std::memory_order_relaxed);
        const float delayTime = delayTimeParam->load(std::memory_order_relaxed);
        const float delayMix = delayMixParam->load(std::memory_order_relaxed);
        const float delayFeedback = delayFeedbackParam->load(std::memory_order_relaxed);
        const float chorusMix = chorusMixParam->load(std::memory_order_relaxed);
        const bool volumeComp = volumeCompParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool eqBypass = eqBypassParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        
        driveSmoother.setTargetValue(driveBase);
        inputGainSmoother.setTargetValue(inputGain);
        outputGainSmoother.setTargetValue(outputGain);

        const float drive = driveSmoother.getNextValue() + (color * kColorDriveFactor);
        const float inputGainLinear = juce::Decibels::decibelsToGain(inputGainSmoother.getNextValue());
        const float outputGainLinear = juce::Decibels::decibelsToGain(outputGainSmoother.getNextValue());

        buffer.applyGain(inputGainLinear);

        const float inputRms = calculateRMS(buffer);
        const float currentInputDb = inputRmsLevel.load(std::memory_order_relaxed);
        const float targetInputDb = juce::Decibels::gainToDecibels(inputRms, kRmsMinDb);
        inputRmsLevel.store(currentInputDb * kRmsSmoothingCoeff + targetInputDb * (1.0f - kRmsSmoothingCoeff), 
                           std::memory_order_relaxed);

        delayMatchedDryBuffer.makeCopyOf(buffer);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        
        warmDistortion.setDrive(drive);
        warmDistortion.setVolumeCompensation(volumeComp);
        warmDistortion.process(context);

        bitCrusher.setBitCrushMix(bitCrush * color);
        bitCrusher.setDACNoise(dacNoise);
        bitCrusher.process(buffer);

        // Update EQ parameters every frame
        const float lowCutFreq = lowCutFreqParam->load(std::memory_order_relaxed);
        parametricEQ.setLowCut(lowCutFreq, lowCutQParam->load(std::memory_order_relaxed), lowCutFreq > 0.0f);
        parametricEQ.setLowMid(lowMidFreqParam->load(std::memory_order_relaxed), 
                               lowMidGainParam->load(std::memory_order_relaxed), 
                               lowMidQParam->load(std::memory_order_relaxed));
        parametricEQ.setHighMid(highMidFreqParam->load(std::memory_order_relaxed), 
                                highMidGainParam->load(std::memory_order_relaxed), 
                                highMidQParam->load(std::memory_order_relaxed));
        const float highCutFreq = highCutFreqParam->load(std::memory_order_relaxed);
        parametricEQ.setHighCut(highCutFreq, highCutQParam->load(std::memory_order_relaxed), highCutFreq < 20000.0f);

        if (!eqBypass)
            parametricEQ.process(context);

        chorus.setChorusMix(chorusMix * color);
        chorus.process(context);

        pingPongDelay.setDelayTime(delayTime);
        pingPongDelay.setDelayMix(delayMix * color);
        pingPongDelay.setFeedback(delayFeedback);
        pingPongDelay.process(buffer);

        buffer.applyGain(outputGainLinear);

        const float outputRms = calculateRMS(buffer);
        const float currentOutputDb = outputRmsLevel.load(std::memory_order_relaxed);
        const float targetOutputDb = juce::Decibels::gainToDecibels(outputRms, kRmsMinDb);
        outputRmsLevel.store(currentOutputDb * kRmsSmoothingCoeff + targetOutputDb * (1.0f - kRmsSmoothingCoeff), 
                            std::memory_order_relaxed);

        const int distortionLatency = warmDistortion.getLatencySamples();
        const int eqLatency = eqBypass ? 0 : parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
        
        if (totalLatency != currentTotalLatency.load())
        {
            currentTotalLatency.store(totalLatency);
            setLatencySamples(totalLatency);
        }
        
        globalDryWet.setWetLatency(static_cast<float>(totalLatency));
        globalDryWet.setWetMixProportion(color);
        globalDryWet.pushDrySamples(juce::dsp::AudioBlock<float>(delayMatchedDryBuffer));
        globalDryWet.mixWetSamples(block);

        auto callback = spectrumAnalyzerCallback;
        if (callback && buffer.getNumChannels() > 0)
            callback(buffer.getReadPointer(0), buffer.getNumSamples());
    }

    //==============================================================================
    bool AudioPluginAudioProcessor::hasEditor() const
    {
        return true;
    }

    juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor()
    {
        return new AudioPluginAudioProcessorEditor(*this);
    }

    void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
    {
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());

        if (xml != nullptr)
        {
            juce::MemoryOutputStream stream(destData, false);
            xml->writeTo(stream);
        }
    }

    void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
    {
        const juce::String xmlString = juce::String::createStringFromData(data, sizeInBytes);

        if (xmlString.isEmpty())
            return;

        auto xmlState = juce::XmlDocument::parse(xmlString);
        if (xmlState == nullptr)
            return;

        auto valueTree = juce::ValueTree::fromXml(*xmlState);
        if (!valueTree.isValid())
            return;

        apvts.replaceState(valueTree);
        updateAllDSPParameters();
    }

    void AudioPluginAudioProcessor::updateAllDSPParameters()
    {
        const float color = colorParam->load();
        const float driveBase = driveParam->load();
        const float drive = driveBase + (color * kColorDriveFactor);
        
        warmDistortion.setDrive(drive);
        warmDistortion.setVolumeCompensation(volumeCompParam->load() > kBooleanThreshold);

        bitCrusher.setBitCrushMix(bitCrushParam->load() * color);
        bitCrusher.setDACNoise(dacNoiseParam->load());

        pingPongDelay.setDelayTime(delayTimeParam->load());
        pingPongDelay.setDelayMix(delayMixParam->load() * color);
        pingPongDelay.setFeedback(delayFeedbackParam->load());

        chorus.setChorusMix(chorusMixParam->load() * color);

        const float lowCutFreq = lowCutFreqParam->load();
        parametricEQ.setLowCut(lowCutFreq, lowCutQParam->load(), lowCutFreq > 0.0f);
        
        parametricEQ.setLowMid(lowMidFreqParam->load(), lowMidGainParam->load(), lowMidQParam->load());
        parametricEQ.setHighMid(highMidFreqParam->load(), highMidGainParam->load(), highMidQParam->load());
        
        const float highCutFreq = highCutFreqParam->load();
        parametricEQ.setHighCut(highCutFreq, highCutQParam->load(), highCutFreq < 20000.0f);
    }

    juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("INPUT_GAIN", "Input Gain", -24.0f, 24.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("COLOR", "Color", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("BITCRUSH", "BitCrush Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DAC_NOISE", "DAC Noise", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_TIME", "Delay Time", juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f), 250.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_MIX", "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_FEEDBACK", "Delay Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.4f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("CHORUS_MIX", "Chorus Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_CUT_FREQ", "Low Cut Freq", 0.0f, 1000.0f, 0.0f));

        auto qRange = juce::NormalisableRange<float>(0.025f, 40.0f, 0.001f);
        qRange.setSkewForCentre(1.0f);

        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_CUT_Q", "Low Cut Q", qRange, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_FREQ", "Low Mid Freq", 100.0f, 2000.0f, 350.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_GAIN", "Low Mid Gain", -15.0f, 15.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_MID_Q", "Low Mid Q", qRange, 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_FREQ", "High Mid Freq", 1000.0f, 8000.0f, 3800.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_GAIN", "High Mid Gain", -15.0f, 15.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_MID_Q", "High Mid Q", qRange, 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_CUT_FREQ", "High Cut Freq", 5000.0f, 20000.0f, 20000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("HIGH_CUT_Q", "High Cut Q", qRange, 1.0f));

        layout.add(std::make_unique<juce::AudioParameterBool>("VOLUME_COMPENSATION", "Volume Compensation", true));
        layout.add(std::make_unique<juce::AudioParameterBool>("EQ_BYPASS", "EQ Bypass", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>("OUTPUT_GAIN", "Output Gain", -24.0f, 24.0f, 0.0f));
        return layout;
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}