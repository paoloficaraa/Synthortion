#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        auto makeGainRange = []() {
            juce::NormalisableRange<float> r(-60.0f, 12.0f, 0.1f);
            r.setSkewForCentre(0.0f);
            return r;
        };

        auto makeQRange = []() {
            juce::NormalisableRange<float> r(0.1f, 10.0f, 0.1f);
            r.setSkewForCentre(1.0f);
            return r;
        };

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"INPUT_GAIN", 1},
            "Input Gain",
            makeGainRange(),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"OUTPUT_GAIN", 1},
            "Output Gain",
            makeGainRange(),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"COLOR", 1},
            "Color",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"BITCRUSH", 1},
            "Bitcrush",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"DELAY_TIME", 1},
            "Delay Time",
            juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f),
            250.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"DELAY_MIX", 1},
            "Delay Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"DELAY_FEEDBACK", 1},
            "Delay Feedback",
            juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
            0.4f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"CHORUS_MIX", 1},
            "Chorus Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"EQ_BYPASS", 1},
            "EQ Bypass",
            juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"VOLUME_COMPENSATION", 1},
            "Volume Compensation",
            juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f),
            1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_CUT_FREQ", 1},
            "Low Cut Freq",
            juce::NormalisableRange<float>(10.0f, 100.0f, 1.0f, 0.3f),
            10.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_CUT_Q", 1},
            "Low Cut Q",
            makeQRange(),
            1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_MID_FREQ", 1},
            "Low Mid Freq",
            juce::NormalisableRange<float>(100.0f, 1000.0f, 1.0f, 0.3f),
            500.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_MID_GAIN", 1},
            "Low Mid Gain",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_MID_Q", 1},
            "Low Mid Q",
            makeQRange(),
            1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_MID_FREQ", 1},
            "High Mid Freq",
            juce::NormalisableRange<float>(1000.0f, 10000.0f, 1.0f, 0.3f),
            5000.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_MID_GAIN", 1},
            "High Mid Gain",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
            0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_MID_Q", 1},
            "High Mid Q",
            makeQRange(),
            1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_CUT_FREQ", 1},
            "High Cut Freq",
            juce::NormalisableRange<float>(10000.0f, 30000.0f, 1.0f, 0.3f),
            30000.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_CUT_Q", 1},
            "High Cut Q",
            makeQRange(),
            1.0f));

        return layout;
    }

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
        inputGainParam = apvts.getRawParameterValue("INPUT_GAIN");
        jassert(inputGainParam != nullptr);
        outputGainParam = apvts.getRawParameterValue("OUTPUT_GAIN");
        jassert(outputGainParam != nullptr);
        colorParam = apvts.getRawParameterValue("COLOR");
        jassert(colorParam != nullptr);
        bitCrushParam = apvts.getRawParameterValue("BITCRUSH");
        jassert(bitCrushParam != nullptr);
        delayTimeParam = apvts.getRawParameterValue("DELAY_TIME");
        jassert(delayTimeParam != nullptr);
        delayMixParam = apvts.getRawParameterValue("DELAY_MIX");
        jassert(delayMixParam != nullptr);
        delayFeedbackParam = apvts.getRawParameterValue("DELAY_FEEDBACK");
        jassert(delayFeedbackParam != nullptr);
        chorusMixParam = apvts.getRawParameterValue("CHORUS_MIX");
        jassert(chorusMixParam != nullptr);
        eqBypassParam = apvts.getRawParameterValue("EQ_BYPASS");
        jassert(eqBypassParam != nullptr);
        volumeCompParam = apvts.getRawParameterValue("VOLUME_COMPENSATION");
        jassert(volumeCompParam != nullptr);

        lowCutFreqParam = apvts.getRawParameterValue("LOW_CUT_FREQ");
        jassert(lowCutFreqParam != nullptr);
        lowCutQParam = apvts.getRawParameterValue("LOW_CUT_Q");
        jassert(lowCutQParam != nullptr);
        lowMidFreqParam = apvts.getRawParameterValue("LOW_MID_FREQ");
        jassert(lowMidFreqParam != nullptr);
        lowMidGainParam = apvts.getRawParameterValue("LOW_MID_GAIN");
        jassert(lowMidGainParam != nullptr);
        lowMidQParam = apvts.getRawParameterValue("LOW_MID_Q");
        jassert(lowMidQParam != nullptr);
        highMidFreqParam = apvts.getRawParameterValue("HIGH_MID_FREQ");
        jassert(highMidFreqParam != nullptr);
        highMidGainParam = apvts.getRawParameterValue("HIGH_MID_GAIN");
        jassert(highMidGainParam != nullptr);
        highMidQParam = apvts.getRawParameterValue("HIGH_MID_Q");
        jassert(highMidQParam != nullptr);
        highCutFreqParam = apvts.getRawParameterValue("HIGH_CUT_FREQ");
        jassert(highCutFreqParam != nullptr);
        highCutQParam = apvts.getRawParameterValue("HIGH_CUT_Q");
        jassert(highCutQParam != nullptr);
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
        jassert(sampleRate > 0.0);
        jassert(samplesPerBlock > 0);

        juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels()};

        warmDistortion.prepare(spec);
        bitCrusher.prepare(spec);
        parametricEQ.prepare(spec);
        chorus.prepare(spec);
        pingPongDelay.prepare(spec);

        // Effects are now independent, no global dry/wet mixer needed
        // No initialization needed for removed components

        inputGainSmoother.reset(sampleRate, kSmootherRampTime);
        inputGainSmoother.setCurrentAndTargetValue(inputGainParam->load());
        
        outputGainSmoother.reset(sampleRate, kSmootherRampTime);
        outputGainSmoother.setCurrentAndTargetValue(outputGainParam->load());

        smoothedColorDrive.reset(sampleRate, 0.05); // 50ms smoothing

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

        jassert(buffer.getNumChannels() > 0);
        jassert(buffer.getNumSamples() > 0);

        if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
            return;

        juce::ScopedNoDenormals noDenormals;

        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        const float color = colorParam->load(std::memory_order_relaxed);
        const float inputGain = inputGainParam->load(std::memory_order_relaxed);
        const float outputGain = outputGainParam->load(std::memory_order_relaxed);
        const float bitCrush = bitCrushParam->load(std::memory_order_relaxed);
        const float delayTime = delayTimeParam->load(std::memory_order_relaxed);
        const float delayMix = delayMixParam->load(std::memory_order_relaxed);
        const float delayFeedback = delayFeedbackParam->load(std::memory_order_relaxed);
        const float chorusMix = chorusMixParam->load(std::memory_order_relaxed);
        const bool volumeComp = volumeCompParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool eqBypass = eqBypassParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        
        inputGainSmoother.setTargetValue(inputGain);
        outputGainSmoother.setTargetValue(outputGain);

        smoothedColorDrive.setTargetValue(color); // Color (0-1) directly maps to drive (0-1)

        // Apply input gain with proper per-sample smoothing
        if (inputGainSmoother.isSmoothing())
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const float gain = juce::Decibels::decibelsToGain(inputGainSmoother.getNextValue());
                    channelData[sample] *= gain;
                }
            }
        }
        else
        {
            const float inputGainLinear = juce::Decibels::decibelsToGain(inputGainSmoother.getCurrentValue());
            buffer.applyGain(inputGainLinear);
        }

        const float inputRms = calculateRMS(buffer);
        const float currentInputDb = inputRmsLevel.load(std::memory_order_relaxed);
        const float targetInputDb = juce::Decibels::gainToDecibels(inputRms, kRmsMinDb);
        inputRmsLevel.store(currentInputDb * kRmsSmoothingCoeff + targetInputDb * (1.0f - kRmsSmoothingCoeff), 
                           std::memory_order_relaxed);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        // Update EQ parameters every frame
        const float lowCutFreq = lowCutFreqParam->load(std::memory_order_relaxed);
        parametricEQ.setLowCut(lowCutFreq, lowCutQParam->load(std::memory_order_relaxed), lowCutFreq > 10.0f);
        parametricEQ.setLowMid(lowMidFreqParam->load(std::memory_order_relaxed),
                               lowMidGainParam->load(std::memory_order_relaxed),
                               lowMidQParam->load(std::memory_order_relaxed));
        parametricEQ.setHighMid(highMidFreqParam->load(std::memory_order_relaxed),
                                highMidGainParam->load(std::memory_order_relaxed),
                                highMidQParam->load(std::memory_order_relaxed));
        const float highCutFreq = highCutFreqParam->load(std::memory_order_relaxed);
        parametricEQ.setHighCut(highCutFreq, highCutQParam->load(std::memory_order_relaxed), highCutFreq < 30000.0f);

        if (!eqBypass)
            parametricEQ.process(context);

        warmDistortion.setVolumeCompensation(volumeComp);
        warmDistortion.process(context, &smoothedColorDrive);

        bitCrusher.setBitCrushMix(bitCrush);
        bitCrusher.process(buffer);

        chorus.setChorusMix(chorusMix);
        chorus.process(buffer);

        pingPongDelay.setDelayTime(delayTime);
        pingPongDelay.setDelayMix(delayMix);
        pingPongDelay.setFeedback(delayFeedback);
        pingPongDelay.process(buffer);

        // Apply output gain with proper per-sample smoothing
        if (outputGainSmoother.isSmoothing())
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    const float gain = juce::Decibels::decibelsToGain(outputGainSmoother.getNextValue());
                    channelData[sample] *= gain;
                }
            }
        }
        else
        {
            const float outputGainLinear = juce::Decibels::decibelsToGain(outputGainSmoother.getCurrentValue());
            buffer.applyGain(outputGainLinear);
        }

        // Feed spectrum analyzer via lock-free FIFO (mono mix)
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();
        const float* left = buffer.getReadPointer(0);
        const float* right = (numChannels > 1) ? buffer.getReadPointer(1) : nullptr;

        int space = spectrumFifo.getFreeSpace();
        if (space > 0)
        {
            int toWrite = juce::jmin(numSamples, space);
            auto writeOp = spectrumFifo.write(toWrite);
            
            auto writeData = [&](int startIndex, int bSize, int sourceOffset) {
                for (int i = 0; i < bSize; ++i)
                {
                    if (numChannels == 1)
                        spectrumBuffer[(size_t)(startIndex + i)] = left[sourceOffset + i];
                    else
                        spectrumBuffer[(size_t)(startIndex + i)] = (left[sourceOffset + i] + right[sourceOffset + i]) * 0.5f;
                }
            };
            
            if (writeOp.blockSize1 > 0)
                writeData(writeOp.startIndex1, writeOp.blockSize1, 0);
            if (writeOp.blockSize2 > 0)
                writeData(writeOp.startIndex2, writeOp.blockSize2, writeOp.blockSize1);
        }

        const float outputRms = calculateRMS(buffer);
        const float currentOutputDb = outputRmsLevel.load(std::memory_order_relaxed);
        const float targetOutputDb = juce::Decibels::gainToDecibels(outputRms, kRmsMinDb);
        outputRmsLevel.store(currentOutputDb * kRmsSmoothingCoeff + targetOutputDb * (1.0f - kRmsSmoothingCoeff), 
                            std::memory_order_relaxed);

        const int distortionLatency = warmDistortion.getLatencySamples();
        const int eqLatency = parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
        currentTotalLatency.store(totalLatency);
        setLatencySamples(totalLatency);

        updateAllDSPParameters();
    }

void AudioPluginAudioProcessor::updateAllDSPParameters()
    {
        const float color = colorParam->load();
        smoothedColorDrive.setCurrentAndTargetValue(color); // Color (0-1) directly maps to drive (0-1)
        const float drive = smoothedColorDrive.getCurrentValue();
        
        warmDistortion.setDrive(drive);
        warmDistortion.setVolumeCompensation(volumeCompParam->load() > kBooleanThreshold);

        bitCrusher.setBitCrushMix(bitCrushParam->load());

        pingPongDelay.setDelayTime(delayTimeParam->load());
        pingPongDelay.setDelayMix(delayMixParam->load());
        pingPongDelay.setFeedback(delayFeedbackParam->load());

        chorus.setChorusMix(chorusMixParam->load());

        const float lowCutFreq = lowCutFreqParam->load();
        parametricEQ.setLowCut(lowCutFreq, lowCutQParam->load(), lowCutFreq > 10.0f);
        
        parametricEQ.setLowMid(lowMidFreqParam->load(), lowMidGainParam->load(), lowMidQParam->load());
        parametricEQ.setHighMid(highMidFreqParam->load(), highMidGainParam->load(), highMidQParam->load());
        
        const float highCutFreq = highCutFreqParam->load();
        parametricEQ.setHighCut(highCutFreq, highCutQParam->load(), highCutFreq < 30000.0f);
    }

    juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
    {
        return new AudioPluginAudioProcessorEditor(*this);
    }

    bool AudioPluginAudioProcessor::hasEditor() const
    {
        return true;
    }

    void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }

    void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState != nullptr)
        {
            if (xmlState->hasTagName(apvts.state.getType()))
            {
                apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
            }
        }
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}