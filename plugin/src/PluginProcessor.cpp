#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"
#include <thread>
#include <vector>

namespace {
inline float computeMaxPeak(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* d = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) peak = std::max(peak, std::abs(d[i]));
    }
    return peak;
}
} // namespace
namespace synthortion {
    juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        auto makeGainRange = []()
        {
            auto r = juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f);
            r.setSkewForCentre(0.0f);
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

        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"PLUGIN_BYPASS", 1},
            "Bypass",
            false));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"DRIVE_ON", 1}, "Drive On", true));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"BITCRUSH_ON", 1}, "Bitcrush On", true));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"DELAY_ON", 1}, "Delay On", true));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"CHORUS_ON", 1}, "Chorus On", true));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"DRIVE_ROUTE", 1}, "Drive Route Post", false));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"DELAY_SYNC", 1}, "Delay Sync Mode",
            juce::StringArray{"SYNC", "FREE", "PING-PONG"}, 0));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"CHORUS_WIDE", 1}, "Chorus Wide", false));

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
        bypassParam = apvts.getRawParameterValue("PLUGIN_BYPASS");
        jassert(bypassParam != nullptr);
        driveOnParam = apvts.getRawParameterValue("DRIVE_ON");
        jassert(driveOnParam != nullptr);
        bitcrushOnParam = apvts.getRawParameterValue("BITCRUSH_ON");
        jassert(bitcrushOnParam != nullptr);
        delayOnParam = apvts.getRawParameterValue("DELAY_ON");
        jassert(delayOnParam != nullptr);
        chorusOnParam = apvts.getRawParameterValue("CHORUS_ON");
        jassert(chorusOnParam != nullptr);
        driveRouteParam = apvts.getRawParameterValue("DRIVE_ROUTE");
        jassert(driveRouteParam != nullptr);
        delaySyncParam = apvts.getRawParameterValue("DELAY_SYNC");
        jassert(delaySyncParam != nullptr);
        chorusWideParam = apvts.getRawParameterValue("CHORUS_WIDE");
        jassert(chorusWideParam != nullptr);
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

    void AudioPluginAudioProcessor::handleMessage(const juce::String& message)
    {
        auto json = juce::JSON::parse(message);
        if (auto* obj = json.getDynamicObject())
        {
            auto parameterId = obj->hasProperty("parameterId")
                ? obj->getProperty("parameterId").toString()
                : obj->getProperty("id").toString();
            auto value = static_cast<float>(obj->getProperty("value"));
            if (auto* param = apvts.getParameter(parameterId))
            {
                param->setValueNotifyingHost(value);
            }
        }
    }

    int AudioPluginAudioProcessor::getNumPrograms()
    {
        return 1;
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
        setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
        juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels())};

        warmDistortion.prepare(spec);
        bitCrusher.prepare(spec);
        chorus.prepare(spec);
        pingPongDelay.prepare(spec);

        inputGainSmoother.reset(sampleRate, kSmootherRampTime);
        inputGainSmoother.setCurrentAndTargetValue(inputGainParam->load());

        outputGainSmoother.reset(sampleRate, kSmootherRampTime);
        outputGainSmoother.setCurrentAndTargetValue(outputGainParam->load());

        smoothedColorDrive.reset(sampleRate, 0.05); // 50ms smoothing

        const int distortionLatency = juce::jmax(1, warmDistortion.getLatencySamples());

        // Set latency based on distortion only
        currentTotalLatency.store(distortionLatency);
        setLatencySamples(distortionLatency);

        updateAllDSPParameters();
        audioFifo.reset();
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
        const bool volumeComp = false; // Hardcoded since UI control removed
        const bool bypass = bypassParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool driveOn = driveOnParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool bitcrushOn = bitcrushOnParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool delayOn = delayOnParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool chorusOn = chorusOnParam->load(std::memory_order_relaxed) > kBooleanThreshold;
        const bool drivePost = driveRouteParam->load(std::memory_order_relaxed) > kBooleanThreshold;

        inputGainSmoother.setTargetValue(inputGain);
        outputGainSmoother.setTargetValue(outputGain);

        smoothedColorDrive.setTargetValue(color);

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
        inputPeak.store(bypass ? 0.0f : computeMaxPeak(buffer));

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        if (!bypass)
        {
            auto runDrive = [&]() {
                if (driveOn) {
                    warmDistortion.setVolumeCompensation(volumeComp);
                    warmDistortion.process(context, &smoothedColorDrive);
                }
            };

            auto runFx = [&]() {
                if (bitcrushOn) {
                    bitCrusher.setBitCrushMix(bitCrush);
                    bitCrusher.process(buffer);
                }

                if (chorusOn) {
                    chorus.setChorusMix(chorusMix);
                    chorus.process(buffer);
                }

                if (delayOn) {
                    pingPongDelay.setDelayTime(delayTime);
                    pingPongDelay.setDelayMix(delayMix);
                    pingPongDelay.setFeedback(delayFeedback);
                    pingPongDelay.process(buffer);
                }
            };

            if (drivePost) {
                runFx();
                runDrive();
            } else {
                runDrive();
                runFx();
            }
        }

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
        outputPeak.store(bypass ? 0.0f : computeMaxPeak(buffer));

        const int distortionLatency = warmDistortion.getLatencySamples();
        currentTotalLatency.store(distortionLatency);
        setLatencySamples(distortionLatency);

        audioFifo.push(buffer);
        updateAllDSPParameters();
    }

    void AudioPluginAudioProcessor::updateAllDSPParameters()
    {
        const float color = colorParam->load();
        smoothedColorDrive.setCurrentAndTargetValue(color);
        const float drive = smoothedColorDrive.getCurrentValue();

        warmDistortion.setDrive(drive);

        bitCrusher.setBitCrushMix(bitCrushParam->load());

        pingPongDelay.setDelayTime(delayTimeParam->load());
        pingPongDelay.setDelayMix(delayMixParam->load());
        pingPongDelay.setFeedback(delayFeedbackParam->load());

        chorus.setChorusMix(chorusMixParam->load());
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
        // Copy the data to a local buffer for background processing
        std::vector<char> buffer(static_cast<const char*>(data), static_cast<const char*>(data) + sizeInBytes);

        std::thread([this, buffer = std::move(buffer)]() mutable {
            std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(buffer.data(), static_cast<int>(buffer.size())));

            if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
            {
                auto newState = juce::ValueTree::fromXml(*xmlState);
                juce::MessageManager::callAsync([this, state = std::move(newState)]() mutable {
                    apvts.replaceState(state);
                });
            }
        }).detach();
    }
}

    juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
    {
        return new synthortion::AudioPluginAudioProcessor();
    }
