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
        apvts.addParameterListener("DRIVE", this);

        apvts.addParameterListener("INPUT_GAIN", this);
        apvts.addParameterListener("OUTPUT_GAIN", this);

        // apvts.addParameterListener("PRESET", this);
        apvts.addParameterListener("COLOR", this);
        apvts.addParameterListener("NOISE_AMOUNT", this);
        apvts.addParameterListener("BITCRUSH", this);
        apvts.addParameterListener("DELAY_TIME", this);
        apvts.addParameterListener("DELAY_MIX", this);
        apvts.addParameterListener("DELAY_FEEDBACK", this);
        apvts.addParameterListener("CHORUS_MIX", this);

        apvts.addParameterListener("EQ_BYPASS", this);
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

        noiseBuffer.setSize(2, samplesPerBlock);

        inputRmsLevel.reset(sampleRate, 0.1);
        outputRmsLevel.reset(sampleRate, 0.1);
        inputRmsLevel.setCurrentAndTargetValue(-60.0f);
        outputRmsLevel.setCurrentAndTargetValue(-60.0f);

        const int distortionLatency = juce::jmax(1, warmDistortion.getLatencySamples());
        const int eqLatency = parametricEQ.getLatencySamples();
        const int totalLatency = distortionLatency + eqLatency;
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
                                                 juce::MidiBuffer &midiMessages)
    {
        juce::ignoreUnused(midiMessages);

        if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
            return;

        juce::ScopedNoDenormals noDenormals;

        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        inputRmsLevel.skip(buffer.getNumSamples());
        outputRmsLevel.skip(buffer.getNumSamples());

        // Reading parameters
        float inputGainValue = apvts.getRawParameterValue("INPUT_GAIN")->load();
        float driveBase = apvts.getRawParameterValue("DRIVE")->load();
        float color = apvts.getRawParameterValue("COLOR")->load();

        // COLOR controls effects intensity
        // Drive increases with color (more saturation as color increases)
        float drive = driveBase + (color * 0.3f); // Adds up to 30% extra drive at full color

        // Noise increases with color, but scaled down to avoid being too loud
        float noiseBase = apvts.getRawParameterValue("NOISE_AMOUNT")->load();
        float noiseAmount = noiseBase * color * 0.5f; // Max 50% of noise knob value to keep it subtle

        // BitCrush, Delay, Chorus controlled by color as master mix
        float bitCrushDepth = apvts.getRawParameterValue("BITCRUSH")->load();
        float effectiveBitDepth = 16.0f - ((16.0f - bitCrushDepth) * color);
        float delayTime = apvts.getRawParameterValue("DELAY_TIME")->load();
        float delayMix = apvts.getRawParameterValue("DELAY_MIX")->load() * color;
        float delayFeedback = apvts.getRawParameterValue("DELAY_FEEDBACK")->load();
        float chorusMix = apvts.getRawParameterValue("CHORUS_MIX")->load() * color;
        float outputGainValue = apvts.getRawParameterValue("OUTPUT_GAIN")->load();
        bool eqBypass = apvts.getRawParameterValue("EQ_BYPASS")->load() > 0.5f;

        // INPUT GAIN
        float inputGainLinear = juce::Decibels::decibelsToGain(inputGainValue);
        buffer.applyGain(inputGainLinear);

        float inputRms = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            inputRms += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        inputRms /= static_cast<float>(buffer.getNumChannels());
        inputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(inputRms, -60.0f));

        // WARM DISTORTION
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        warmDistortion.setDrive(drive);
        warmDistortion.process(context);

        // NOISE
        if (noiseAmount > 0.001f)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto *channelData = buffer.getWritePointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    float noise = (noiseGenerator.nextFloat() * 2.0f - 1.0f) * noiseAmount * 0.1f;
                    channelData[i] += noise;
                }
            }
        }

        // BITCRUSHER (scaled by color: 0% color = 16-bit clean, 100% color = full crush)
        bitCrusher.setBitDepth(effectiveBitDepth);
        bitCrusher.process(buffer);

        // PARAMETRIC EQ
        if (!eqBypass)
        {
            parametricEQ.process(context);
        }

        // CHORUS
        chorus.setChorusMix(chorusMix);
        chorus.process(context);

        // PING-PONG DELAY
        pingPongDelay.setDelayTime(delayTime);
        pingPongDelay.setDelayMix(delayMix);
        pingPongDelay.setFeedback(delayFeedback);
        pingPongDelay.process(buffer);

        // OUTPUT GAIN
        float outputGainLinear = juce::Decibels::decibelsToGain(outputGainValue);
        buffer.applyGain(outputGainLinear);

        float outputRms = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            outputRms += buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        outputRms /= static_cast<float>(buffer.getNumChannels());
        outputRmsLevel.setTargetValue(juce::Decibels::gainToDecibels(outputRms, -60.0f));

        // Spectrum analyzer callback
        auto callback = spectrumAnalyzerCallback;
        if (callback && buffer.getNumChannels() > 0)
        {
            auto *channelData = buffer.getReadPointer(0);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                callback(channelData[i]);
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
            // else if (parameterID == "PRESET")
            // {
            //     loadPreset(static_cast<int>(newValue));
            // }
            else if (parameterID == "BITCRUSH")
            {
                bitCrusher.setBitDepth(newValue);
            }
            else if (parameterID == "DELAY_TIME")
            {
                pingPongDelay.setDelayTime(newValue);
            }
            else if (parameterID == "DELAY_FEEDBACK")
            {
                pingPongDelay.setFeedback(newValue);
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
        try
        {
            // Update distortion parameters
            auto drive = juce::jlimit(0.0f, 1.0f, apvts.getRawParameterValue("DRIVE")->load());
            warmDistortion.setDrive(drive);

            auto volumeComp = apvts.getRawParameterValue("VOLUME_COMPENSATION")->load() > 0.5f;
            warmDistortion.setVolumeCompensation(volumeComp);

            // BitCrusher
            auto bitDepth = apvts.getRawParameterValue("BITCRUSH")->load();
            bitCrusher.setBitDepth(bitDepth);

            // Delay
            auto delayTime = apvts.getRawParameterValue("DELAY_TIME")->load();
            pingPongDelay.setDelayTime(delayTime);

            auto delayMix = apvts.getRawParameterValue("DELAY_MIX")->load();
            pingPongDelay.setDelayMix(delayMix);

            auto delayFeedback = apvts.getRawParameterValue("DELAY_FEEDBACK")->load();
            pingPongDelay.setFeedback(delayFeedback);

            // Chorus
            auto chorusMix = apvts.getRawParameterValue("CHORUS_MIX")->load();
            chorus.setChorusMix(chorusMix);

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
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "COLOR",
            "Color",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        // NOISE parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "NOISE_AMOUNT",
            "Noise Amount",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        // BITCRUSH parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "BITCRUSH",
            "BitCrush",
            juce::NormalisableRange<float>(1.0f, 16.0f, 1.0f),
            16.0f));

        // DELAY_TIME parameter (in milliseconds)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "DELAY_TIME",
            "Delay Time",
            juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f),
            250.0f));

        // DELAY_MIX parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "DELAY_MIX",
            "Delay Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

        // DELAY_FEEDBACK parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "DELAY_FEEDBACK",
            "Delay Feedback",
            juce::NormalisableRange<float>(0.0f, 0.9f, 0.01f),
            0.4f));

        // CHORUS_MIX parameter
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            "CHORUS_MIX",
            "Chorus Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f));

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