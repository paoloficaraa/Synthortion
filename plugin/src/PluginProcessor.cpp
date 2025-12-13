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
        // Cache parameter pointers for fast access
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
        // Clear any callbacks to prevent dangling references
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
        globalMixerSpec.maximumBlockSize = spec.maximumBlockSize * 4; // Margin for latency
        globalDryWet.prepare(globalMixerSpec);
        globalDryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
        globalDryWet.reset();

        // Prepare dry buffer for latency-matched signal
        delayMatchedDryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
        delayMatchedDryBuffer.clear();

        // Initialize parameter smoothers for audio-quality parameters (0.05s ramp time)
        // NOTE: Color is NOT smoothed - it needs instant response for phase-accurate mixing
        const float rampTimeSeconds = 0.05f;
        
        driveSmoother.reset(sampleRate, rampTimeSeconds);
        driveSmoother.setCurrentAndTargetValue(driveParam->load());
        
        inputGainSmoother.reset(sampleRate, rampTimeSeconds);
        inputGainSmoother.setCurrentAndTargetValue(inputGainParam->load());
        
        outputGainSmoother.reset(sampleRate, rampTimeSeconds);
        outputGainSmoother.setCurrentAndTargetValue(outputGainParam->load());

        // Initialize bypass state (instant switching)
        previousEqBypassState = eqBypassParam->load() > 0.5f;

        // Reset RMS levels
        inputRmsLevel.store(-60.0f);
        outputRmsLevel.store(-60.0f);

        // Calculate initial latency
        const int distortionLatency = juce::jmax(1, warmDistortion.getLatencySamples());
        const int eqLatency = parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
        currentTotalLatency.store(totalLatency);
        setLatencySamples(totalLatency);

        updateDSPParameters();
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

    void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
        [[maybe_unused]] juce::MidiBuffer &midiMessages)
    {
        juce::ignoreUnused(midiMessages);

        if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
            return;

        juce::ScopedNoDenormals noDenormals;

        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // ===== INSTANT COLOR READING: No smoothing for phase-accurate response =====
        float color = colorParam->load(std::memory_order_relaxed); // INSTANT - no ramp!
        
        // ===== PARAMETER SMOOTHING: Only for audio-quality parameters (prevents clicks) =====
        driveSmoother.setTargetValue(driveParam->load(std::memory_order_relaxed));
        inputGainSmoother.setTargetValue(inputGainParam->load(std::memory_order_relaxed));
        outputGainSmoother.setTargetValue(outputGainParam->load(std::memory_order_relaxed));

        // Get smoothed values for this block
        float driveBase = driveSmoother.getNextValue();
        float inputGainValue = inputGainSmoother.getNextValue();
        float outputGainValue = outputGainSmoother.getNextValue();

        // COLOR controls effects intensity
        // Drive increases with color (more saturation as color increases)
        float drive = driveBase + (color * 0.6f); // Adds up to 60% extra drive at full color

        // BitCrush, Delay, Chorus controlled by color as master mix
        float bitCrushMix = bitCrushParam->load(std::memory_order_relaxed) * color;
        bitCrusher.setBitCrushMix(bitCrushMix);
        
        float dacNoise = dacNoiseParam->load(std::memory_order_relaxed);
        bitCrusher.setDACNoise(dacNoise);
        
        float delayTime = delayTimeParam->load(std::memory_order_relaxed);
        float delayMix = delayMixParam->load(std::memory_order_relaxed) * color;
        float delayFeedback = delayFeedbackParam->load(std::memory_order_relaxed);
        
        float chorusMix = chorusMixParam->load(std::memory_order_relaxed) * color;
        bool volumeComp = volumeCompParam->load(std::memory_order_relaxed) > 0.5f;

        // ===== INSTANT EQ BYPASS: No ramping for immediate phase response =====
        bool currentEqBypassState = eqBypassParam->load(std::memory_order_relaxed) > 0.5f;

        // Update EQ parameters
        // Note: In a highly optimized plugin, we would check if these changed before updating
        // but for this scale, updating per block is acceptable and safer than per-sample.
        auto lowCutFreq = juce::jlimit(0.0f, 1000.0f, lowCutFreqParam->load(std::memory_order_relaxed));
        auto lowCutQ = juce::jlimit(0.025f, 40.0f, lowCutQParam->load(std::memory_order_relaxed));
        parametricEQ.setLowCut(lowCutFreq, lowCutQ, lowCutFreq > 0.0f);

        auto lowMidFreq = juce::jlimit(100.0f, 2000.0f, lowMidFreqParam->load(std::memory_order_relaxed));
        auto lowMidGain = juce::jlimit(-15.0f, 15.0f, lowMidGainParam->load(std::memory_order_relaxed));
        auto lowMidQ = juce::jlimit(0.025f, 40.0f, lowMidQParam->load(std::memory_order_relaxed));
        parametricEQ.setLowMid(lowMidFreq, lowMidGain, lowMidQ);

        auto highMidFreq = juce::jlimit(1000.0f, 8000.0f, highMidFreqParam->load(std::memory_order_relaxed));
        auto highMidGain = juce::jlimit(-15.0f, 15.0f, highMidGainParam->load(std::memory_order_relaxed));
        auto highMidQ = juce::jlimit(0.025f, 40.0f, highMidQParam->load(std::memory_order_relaxed));
        parametricEQ.setHighMid(highMidFreq, highMidGain, highMidQ);

        auto highCutFreq = juce::jlimit(5000.0f, 20000.0f, highCutFreqParam->load(std::memory_order_relaxed));
        auto highCutQ = juce::jlimit(0.025f, 40.0f, highCutQParam->load(std::memory_order_relaxed));
        parametricEQ.setHighCut(highCutFreq, highCutQ, highCutFreq < 20000.0f);

        // INPUT GAIN (smoothed)
        float inputGainLinear = juce::Decibels::decibelsToGain(inputGainValue);
        buffer.applyGain(inputGainLinear);

        // Calculate Input RMS
        float inputRms = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            inputRms += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        inputRms /= static_cast<float>(buffer.getNumChannels());
        
        // Smooth the RMS value for display (simple IIR)
        float currentInputDb = inputRmsLevel.load(std::memory_order_relaxed);
        float targetInputDb = juce::Decibels::gainToDecibels(inputRms, -60.0f);
        inputRmsLevel.store(currentInputDb * 0.9f + targetInputDb * 0.1f, std::memory_order_relaxed);

        // ===== COPY DRY SIGNAL BEFORE PROCESSING (for latency compensation) =====
        delayMatchedDryBuffer.makeCopyOf(buffer);

        // WARM DISTORTION
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        
        warmDistortion.setDrive(drive);
        warmDistortion.setVolumeCompensation(volumeComp);
        warmDistortion.process(context);

        bitCrusher.process(buffer);

        // ===== PARAMETRIC EQ WITH INSTANT BYPASS (NO CROSSFADE) =====
        if (!currentEqBypassState) // Process EQ only if NOT bypassed
        {
            parametricEQ.process(context);
        }
        // When bypassed, signal passes through unprocessed (instant, phase-correct)

        // CHORUS
        chorus.setChorusMix(chorusMix);
        chorus.process(context);

        // PING-PONG DELAY
        pingPongDelay.setDelayTime(delayTime);
        pingPongDelay.setDelayMix(delayMix);
        pingPongDelay.setFeedback(delayFeedback);
        pingPongDelay.process(buffer);

        // OUTPUT GAIN (smoothed)
        float outputGainLinear = juce::Decibels::decibelsToGain(outputGainValue);
        buffer.applyGain(outputGainLinear);

        // Calculate Output RMS
        float outputRms = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            outputRms += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        outputRms /= static_cast<float>(buffer.getNumChannels());
        
        float currentOutputDb = outputRmsLevel.load(std::memory_order_relaxed);
        float targetOutputDb = juce::Decibels::gainToDecibels(outputRms, -60.0f);
        outputRmsLevel.store(currentOutputDb * 0.9f + targetOutputDb * 0.1f, std::memory_order_relaxed);

        // ===== GLOBAL DRY/WET MIX WITH LATENCY COMPENSATION =====
        // Calculate total latency from all active processors
        const int distortionLatency = warmDistortion.getLatencySamples();
        const int eqLatency = currentEqBypassState ? 0 : parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
        
        // Update reported latency if changed (dynamic latency reporting)
        if (totalLatency != currentTotalLatency.load())
        {
            currentTotalLatency.store(totalLatency);
            setLatencySamples(totalLatency);
        }
        
        // Set the latency compensation in the mixer
        globalDryWet.setWetLatency(static_cast<float>(totalLatency));
        
        // When color is 0, the mix should be 100% dry (no effects)
        globalDryWet.setWetMixProportion(colorParam->load(std::memory_order_relaxed));
        
        // Push the dry signal into the mixer's internal delay buffer
        globalDryWet.pushDrySamples(juce::dsp::AudioBlock<float>(delayMatchedDryBuffer));
        
        // Mix the processed (wet) signal with the latency-compensated dry signal
        globalDryWet.mixWetSamples(block);

        // Spectrum analyzer callback
        // Optimized: Pass the whole buffer pointer instead of per-sample callback
        auto callback = spectrumAnalyzerCallback;
        if (callback && buffer.getNumChannels() > 0)
        {
            // Use the first channel (mono sum would be better but this is standard)
            callback(buffer.getReadPointer(0), buffer.getNumSamples());
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
        // Save parameter state to memory block using APVTS built-in method
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());

        if (xml != nullptr)
        {
            juce::MemoryOutputStream stream(destData, false);
            xml->writeTo(stream);
            DBG("Plugin state saved successfully");
        }
        else
        {
            DBG("Failed to save plugin state");
        }
    }

    void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
    {
        // Restore parameter state from memory block using APVTS built-in method
        juce::String xmlString = juce::String::createStringFromData(data, sizeInBytes);

        if (xmlString.isNotEmpty())
        {
            auto xmlState = juce::XmlDocument::parse(xmlString);

            if (xmlState != nullptr)
            {
                auto valueTree = juce::ValueTree::fromXml(*xmlState);
                if (valueTree.isValid())
                {
                    apvts.replaceState(valueTree);

                    // Force DSP objects to sync with restored parameters
                    updateDSPParameters();

                    DBG("Plugin state restored successfully");
                }
                else
                {
                    DBG("Failed to restore plugin state - invalid ValueTree");
                }
            }
            else
            {
                DBG("Failed to restore plugin state - invalid XML data");
            }
        }
        else
        {
            DBG("Failed to restore plugin state - empty data");
        }
    }

    void AudioPluginAudioProcessor::updateDSPParameters()
    {
        try
        {
            // Update distortion parameters
            auto driveBase = driveParam->load();
            auto color = colorParam->load();
            auto drive = driveBase + (color * 0.6f);
            warmDistortion.setDrive(drive);

            auto volumeComp = volumeCompParam->load() > 0.5f;
            warmDistortion.setVolumeCompensation(volumeComp);

            // BitCrusher
            auto bitcrushMix = bitCrushParam->load();
            bitCrusher.setBitCrushMix(bitcrushMix);

            auto dacNoise = dacNoiseParam->load();
            bitCrusher.setDACNoise(dacNoise);

            // Delay
            auto delayTime = delayTimeParam->load();
            pingPongDelay.setDelayTime(delayTime);

            auto delayMix = delayMixParam->load();
            pingPongDelay.setDelayMix(delayMix);

            auto delayFeedback = delayFeedbackParam->load();
            pingPongDelay.setFeedback(delayFeedback);

            // Chorus
            auto chorusMix = chorusMixParam->load();
            chorus.setChorusMix(chorusMix);

            // Update EQ parameters
            auto lowCutFreq = juce::jlimit(0.0f, 1000.0f, lowCutFreqParam->load());
            auto lowCutQ = juce::jlimit(0.1f, 10.0f, lowCutQParam->load());
            parametricEQ.setLowCut(lowCutFreq, lowCutQ, lowCutFreq > 0.0f);

            auto lowMidFreq = juce::jlimit(100.0f, 2000.0f, lowMidFreqParam->load());
            auto lowMidGain = juce::jlimit(-15.0f, 15.0f, lowMidGainParam->load());
            auto lowMidQ = juce::jlimit(0.025f, 40.0f, lowMidQParam->load());
            parametricEQ.setLowMid(lowMidFreq, lowMidGain, lowMidQ);

            auto highMidFreq = juce::jlimit(1000.0f, 8000.0f, highMidFreqParam->load());
            auto highMidGain = juce::jlimit(-15.0f, 15.0f, highMidGainParam->load());
            auto highMidQ = juce::jlimit(0.025f, 40.0f, highMidQParam->load());
            parametricEQ.setHighMid(highMidFreq, highMidGain, highMidQ);

            auto highCutFreq = juce::jlimit(5000.0f, 20000.0f, highCutFreqParam->load());
            auto highCutQ = juce::jlimit(0.025f, 40.0f, highCutQParam->load());
            parametricEQ.setHighCut(highCutFreq, highCutQ, highCutFreq < 20000.0f);

            DBG("DSP parameters synchronized on initialization");
        }
        catch (const std::exception &e)
        {
            DBG("Error updating DSP parameters: " << e.what());
            (void)e; // Suppress unused variable warning
        }
    }

    juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        // DRIVE parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 0.0f, 1.0f, 0.0f));

        // INPUT parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("INPUT_GAIN", "Input Gain", -24.0f, 24.0f, 0.0f));

        // COLOR parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("COLOR", "Color", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        // BITCRUSH parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("BITCRUSH", "BitCrush Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("DAC_NOISE", "DAC Noise", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        // DELAY_TIME parameter (in milliseconds)
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_TIME", "Delay Time", juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f), 250.0f));

        // DELAY_MIX parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_MIX", "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        // DELAY_FEEDBACK parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("DELAY_FEEDBACK", "Delay Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.4f));

        // CHORUS_MIX parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("CHORUS_MIX", "Chorus Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        // PRESET parameter
        // layout.add(std::make_unique<juce::AudioParameterChoice>(
        //     "PRESET",
        //     "Preset",
        //     juce::StringArray{"User", "Clean Tape", "Lofi Chaos", "Ambient Wash", "Aggressive Crunch"},
        //     0));

        // EQ Parameters
        layout.add(std::make_unique<juce::AudioParameterFloat>("LOW_CUT_FREQ", "Low Cut Freq", 0.0f, 1000.0f, 0.0f));

        // Custom Q range with center at 1.0 (0.025 to 1.0 in first half, 1.0 to 40.0 in second half)
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

        // Volume compensation toggle
        layout.add(std::make_unique<juce::AudioParameterBool>("VOLUME_COMPENSATION", "Volume Compensation", true));

        // EQ bypass toggle
        layout.add(std::make_unique<juce::AudioParameterBool>("EQ_BYPASS", "EQ Bypass", false));

        // OUTPUT parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>("OUTPUT_GAIN", "Output Gain", -24.0f, 24.0f, 0.0f));
        return layout;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}