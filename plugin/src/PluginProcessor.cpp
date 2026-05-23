#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"
#include <thread>
#include <vector>

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
             juce::ParameterID{"VOLUME_COMPENSATION", 1},
             "Volume Compensation",
             juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f),
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

        volumeCompParam = apvts.getRawParameterValue("VOLUME_COMPENSATION");
        jassert(volumeCompParam != nullptr);


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

        // Set latency based on distortion only
        currentTotalLatency.store(distortionLatency);
        setLatencySamples(distortionLatency);

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



        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);



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





        const int distortionLatency = warmDistortion.getLatencySamples();
        currentTotalLatency.store(distortionLatency);
        setLatencySamples(distortionLatency);

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

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new synthortion::AudioPluginAudioProcessor();
}