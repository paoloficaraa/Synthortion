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
        // Register parameter listeners for all parameters
        apvts.addParameterListener("DRIVE", this);
        apvts.addParameterListener("SATURATION_TYPE", this);
        apvts.addParameterListener("LOW_CUT_FREQ", this);
        apvts.addParameterListener("LOW_CUT_Q", this);
        apvts.addParameterListener("LOW_MID_FREQ", this);
        apvts.addParameterListener("LOW_MID_GAIN", this);
        apvts.addParameterListener("LOW_MID_Q", this);
        apvts.addParameterListener("HIGH_MID_FREQ", this);
        apvts.addParameterListener("HIGH_MID_GAIN", this);
        apvts.addParameterListener("HIGH_MID_Q", this);
        apvts.addParameterListener("HIGH_CUT_FREQ", this);
        apvts.addParameterListener("HIGH_CUT_Q", this);
        apvts.addParameterListener("VOLUME_COMPENSATION", this);
        apvts.addParameterListener("STEREO_BALANCE", this);
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
        juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumInputChannels()};
        warmDistortion.prepare(spec);
        parametricEQ.prepare(spec);

        // Configure global dry/wet mixer with sufficient maximum latency
        {
            const int distortionLatency = juce::jmax(1, warmDistortion.getLatencySamples());
            const int eqLatency = parametricEQ.getLatencySamples();
            const int totalLatency = juce::jmax(1, distortionLatency + eqLatency);

            globalDryWet = juce::dsp::DryWetMixer<float>(totalLatency);
            globalDryWet.prepare(spec);
            globalDryWet.reset();
            globalDryWet.setWetLatency(static_cast<float>(totalLatency));
            // Equal-power style crossfade for more musical blending
            globalDryWet.setMixingRule(juce::dsp::DryWetMixer<float>::MixingRule::squareRoot3dB);

            // Report total latency to host
            setLatencySamples(totalLatency);
        }

        // Initialize RMS level smoothing
        inputRmsLevel.reset(sampleRate, 0.1); // 100ms smoothing
        outputRmsLevel.reset(sampleRate, 0.1);
        inputRmsLevel.setCurrentAndTargetValue(-60.0f);
        outputRmsLevel.setCurrentAndTargetValue(-60.0f);

        // Initialize all parameters to ensure DSP objects match saved values
        updateDSPParameters();
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

        // Enhanced validation checks
        if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        {
            DBG("Warning: Empty audio buffer received");
            return;
        }

        if (getSampleRate() <= 0.0)
        {
            DBG("Error: Invalid sample rate: " << getSampleRate());
            return;
        }

        // Validate buffer contents for NaN/Inf values
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto *channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                if (!std::isfinite(channelData[sample]))
                {
                    DBG("Warning: Non-finite value detected in input buffer, channel " << channel << ", sample " << sample);
                    channelData[sample] = 0.0f; // Replace with silence
                }
            }
        }

        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        // Clear unused output channels
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // Skip RMS processing ahead
        inputRmsLevel.skip(buffer.getNumSamples());
        outputRmsLevel.skip(buffer.getNumSamples());

        auto mixValue = apvts.getRawParameterValue("MIX")->load();

        // Validate mix parameter
        if (!std::isfinite(mixValue))
        {
            DBG("Warning: Non-finite mix value detected, using default");
            mixValue = 1.0f;
        }
        mixValue = juce::jlimit(0.0f, 1.0f, mixValue);

        // Apply input gain
        auto inputGainValue = apvts.getRawParameterValue("INPUT_GAIN")->load();
        if (std::isfinite(inputGainValue))
        {
            float inputGainLinear = juce::Decibels::decibelsToGain(inputGainValue);
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                juce::FloatVectorOperations::multiply(buffer.getWritePointer(channel), inputGainLinear, buffer.getNumSamples());
            }
        }

        // Calculate input RMS level with error checking using optimized operations
        float inputRms = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            // Use JUCE's optimized RMS calculation
            float channelRms = buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
            if (std::isfinite(channelRms))
            {
                inputRms += channelRms;
            }
            else
            {
                DBG("Warning: Non-finite RMS level in channel " << channel);
            }
        }
        inputRms /= static_cast<float>(buffer.getNumChannels());

        // Validate and set input RMS level
        if (std::isfinite(inputRms))
        {
            inputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(inputRms, -60.0f));
        }
        else
        {
            inputRmsLevel.setTargetValue(-60.0f); // Fallback to minimum level
        }

        // Short-circuit paths for MIX extremes
        const float eps = 1.0e-6f;

        // Fully dry: leave buffer untouched and skip all processing/mixing
        if (mixValue <= eps)
        {
            // Output RMS equals input in this case
            outputRmsLevel.setTargetValue(inputRmsLevel.getTargetValue());

            // Analyzer on dry - process all samples for accuracy
            auto callbackDry = spectrumAnalyzerCallback;
            if (callbackDry && buffer.getNumChannels() > 0)
            {
                auto *dryData = buffer.getReadPointer(0);

                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    callbackDry(dryData[sample]);
            }
            return;
        }

        // For partially wet mixes, capture dry before processing
        const bool isFullyWet = (mixValue >= 1.0f - eps);
        if (!isFullyWet)
        {
            try
            {
                globalDryWet.pushDrySamples(juce::dsp::AudioBlock<const float>(buffer));
            }
            catch (const std::exception &e)
            {
                DBG("Error in dry samples processing: " << e.what());
                (void)e; // Suppress unused variable warning
                // Continue with wet processing only
            }
        }

        // Wet processing chain with error handling
        try
        {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            warmDistortion.process(context);

            // Apply EQ post-distortion
            parametricEQ.process(context);
        }
        catch (const std::exception &e)
        {
            DBG("Error in wet processing chain: " << e.what());
            (void)e; // Suppress unused variable warning
            // Leave buffer as-is if processing fails
        }

        // Apply global dry/wet mix at end (only if partially wet)
        if (!isFullyWet)
        {
            try
            {
                globalDryWet.setWetMixProportion(juce::jlimit(0.0f, 1.0f, mixValue));
                globalDryWet.mixWetSamples(juce::dsp::AudioBlock<float>(buffer));
            }
            catch (const std::exception &e)
            {
                DBG("Error in dry/wet mixing: " << e.what());
                (void)e; // Suppress unused variable warning
                // Continue without mixing
            }
        }

        // Validate output buffer for non-finite values
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto *channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                if (!std::isfinite(channelData[sample]))
                {
                    DBG("Warning: Non-finite value in output buffer, channel " << channel << ", sample " << sample);
                    channelData[sample] = 0.0f; // Replace with silence
                }
            }
        }

        // Apply output gain before RMS calculation and spectrum analysis
        auto outputGainValue = apvts.getRawParameterValue("OUTPUT_GAIN")->load();
        if (std::isfinite(outputGainValue))
        {
            float outputGainLinear = juce::Decibels::decibelsToGain(outputGainValue);
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                juce::FloatVectorOperations::multiply(buffer.getWritePointer(channel), outputGainLinear, buffer.getNumSamples());
            }
        }

        // Calculate output RMS level with error checking (after output gain)
        float outputRms = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float channelRms = buffer.getRMSLevel(channel, 0, buffer.getNumSamples());
            if (std::isfinite(channelRms))
            {
                outputRms += channelRms;
            }
            else
            {
                DBG("Warning: Non-finite output RMS level in channel " << channel);
            }
        }
        outputRms /= static_cast<float>(buffer.getNumChannels());

        // Validate and set output RMS level
        if (std::isfinite(outputRms))
        {
            outputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(outputRms, -60.0f));
        }
        else
        {
            outputRmsLevel.setTargetValue(-60.0f); // Fallback to minimum level
        }

        // Send final output audio data to spectrum analyzer (post-everything including dry/wet)
        // This shows exactly what goes to the output, reflecting all processing and mixing
        auto callback = spectrumAnalyzerCallback;
        if (callback && buffer.getNumChannels() > 0)
        {
            auto *channelData = buffer.getReadPointer(0);

            // Process all samples for better frequency resolution and accuracy
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                callback(channelData[sample]);
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

    void AudioPluginAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
    {
        // Validate parameter value
        if (!std::isfinite(newValue))
        {
            DBG("Warning: Non-finite parameter value for " << parameterID);
            return; // Skip invalid parameter changes
        }

        try
        {
            if (parameterID == "DRIVE")
            {
                float validDrive = juce::jlimit(0.0f, 1.0f, newValue);
                warmDistortion.setDrive(validDrive);
            }
            else if (parameterID == "SATURATION_TYPE")
            {
                int typeInt = juce::jlimit(0, 2, static_cast<int>(newValue));
                warmDistortion.setSaturationType(static_cast<WarmDistortion::SaturationType>(typeInt));
            }
            else if (parameterID == "LOW_CUT_FREQ" || parameterID == "LOW_CUT_Q")
            {
                auto lowCutFreq = juce::jlimit(0.0f, 1000.0f, apvts.getRawParameterValue("LOW_CUT_FREQ")->load());
                auto lowCutQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("LOW_CUT_Q")->load());
                parametricEQ.setLowCut(lowCutFreq, lowCutQ, lowCutFreq > 0.0f);
            }
            else if (parameterID == "LOW_MID_FREQ" || parameterID == "LOW_MID_GAIN" || parameterID == "LOW_MID_Q")
            {
                auto lowMidFreq = juce::jlimit(100.0f, 2000.0f, apvts.getRawParameterValue("LOW_MID_FREQ")->load());
                auto lowMidGain = juce::jlimit(-15.0f, 15.0f, apvts.getRawParameterValue("LOW_MID_GAIN")->load());
                auto lowMidQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("LOW_MID_Q")->load());
                parametricEQ.setLowMid(lowMidFreq, lowMidGain, lowMidQ);
            }
            else if (parameterID == "HIGH_MID_FREQ" || parameterID == "HIGH_MID_GAIN" || parameterID == "HIGH_MID_Q")
            {
                auto highMidFreq = juce::jlimit(1000.0f, 8000.0f, apvts.getRawParameterValue("HIGH_MID_FREQ")->load());
                auto highMidGain = juce::jlimit(-15.0f, 15.0f, apvts.getRawParameterValue("HIGH_MID_GAIN")->load());
                auto highMidQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("HIGH_MID_Q")->load());
                parametricEQ.setHighMid(highMidFreq, highMidGain, highMidQ);
            }
            else if (parameterID == "HIGH_CUT_FREQ" || parameterID == "HIGH_CUT_Q")
            {
                auto highCutFreq = juce::jlimit(5000.0f, 20000.0f, apvts.getRawParameterValue("HIGH_CUT_FREQ")->load());
                auto highCutQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("HIGH_CUT_Q")->load());
                // Inverted behavior: disabled when freq >= 20000.0f, enabled when freq < 20000.0f
                parametricEQ.setHighCut(highCutFreq, highCutQ, highCutFreq < 20000.0f);
            }
            else if (parameterID == "VOLUME_COMPENSATION")
            {
                bool enabled = newValue > 0.5f;
                warmDistortion.setVolumeCompensation(enabled);
            }
        }
        catch (const std::exception &e)
        {
            DBG("Error processing parameter change for " << parameterID << ": " << e.what());
            (void)e; // Suppress unused variable warning
        }
    }

    void AudioPluginAudioProcessor::updateDSPParameters()
    {
        // Force all DSP objects to sync with current parameter values
        // This ensures saved state is properly restored on plugin initialization

        try
        {
            // Update distortion parameters
            auto drive = juce::jlimit(0.0f, 1.0f, apvts.getRawParameterValue("DRIVE")->load());
            warmDistortion.setDrive(drive);

            auto satType = juce::jlimit(0, 2, static_cast<int>(apvts.getRawParameterValue("SATURATION_TYPE")->load()));
            warmDistortion.setSaturationType(static_cast<WarmDistortion::SaturationType>(satType));

            auto volumeComp = apvts.getRawParameterValue("VOLUME_COMPENSATION")->load() > 0.5f;
            warmDistortion.setVolumeCompensation(volumeComp);

            // Update EQ parameters
            auto lowCutFreq = juce::jlimit(0.0f, 1000.0f, apvts.getRawParameterValue("LOW_CUT_FREQ")->load());
            auto lowCutQ = juce::jlimit(0.1f, 10.0f, apvts.getRawParameterValue("LOW_CUT_Q")->load());
            parametricEQ.setLowCut(lowCutFreq, lowCutQ, lowCutFreq > 0.0f);

            auto lowMidFreq = juce::jlimit(100.0f, 2000.0f, apvts.getRawParameterValue("LOW_MID_FREQ")->load());
            auto lowMidGain = juce::jlimit(-15.0f, 15.0f, apvts.getRawParameterValue("LOW_MID_GAIN")->load());
            auto lowMidQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("LOW_MID_Q")->load());
            parametricEQ.setLowMid(lowMidFreq, lowMidGain, lowMidQ);

            auto highMidFreq = juce::jlimit(1000.0f, 8000.0f, apvts.getRawParameterValue("HIGH_MID_FREQ")->load());
            auto highMidGain = juce::jlimit(-15.0f, 15.0f, apvts.getRawParameterValue("HIGH_MID_GAIN")->load());
            auto highMidQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("HIGH_MID_Q")->load());
            parametricEQ.setHighMid(highMidFreq, highMidGain, highMidQ);

            auto highCutFreq = juce::jlimit(5000.0f, 20000.0f, apvts.getRawParameterValue("HIGH_CUT_FREQ")->load());
            auto highCutQ = juce::jlimit(0.025f, 40.0f, apvts.getRawParameterValue("HIGH_CUT_Q")->load());
            // Inverted behavior: disabled when freq >= 20000.0f, enabled when freq < 20000.0f
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

        // Stereo balance correction toggle
        layout.add(std::make_unique<juce::AudioParameterBool>("STEREO_BALANCE", "Stereo Balance Correction", true));

        return layout;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}