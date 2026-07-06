#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <functional>
#include <array>
#include <juce_dsp/juce_dsp.h>

#include "Synthortion/WarmDistortion.h"
#include "Synthortion/SynthortionChorus.h"
#include "Synthortion/PingPongDelay.h"
#include "Synthortion/BitCrusher.h"

namespace synthortion
{
    // Main audio processor: warm distortion, chorus, delay, bit-crusher
    class AudioPluginAudioProcessor final : public juce::AudioProcessor
    {
    public:
        AudioPluginAudioProcessor();
        ~AudioPluginAudioProcessor() override;

        friend class AudioPluginAudioProcessorEditor;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
        using AudioProcessor::processBlock;

        juce::AudioProcessorEditor *createEditor() override;
        bool hasEditor() const override;

        const juce::String getName() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;

        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String &newName) override;

        void getStateInformation(juce::MemoryBlock &destData) override;
        void setStateInformation(const void *data, int sizeInBytes) override;

        juce::LinearSmoothedValue<float> smoothedColorDrive { 0.0f };

    private:
        static constexpr float kSmootherRampTime = 0.05f;
        static constexpr float kBooleanThreshold = 0.5f;
        static constexpr int kLatencyBufferMargin = 4;

        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void updateAllDSPParameters();

        WarmDistortion warmDistortion;
        SynthortionChorus chorus;
        PingPongDelay pingPongDelay;
        BitCrusher bitCrusher;

        juce::AudioProcessorValueTreeState apvts;

        std::atomic<float>* inputGainParam = nullptr;
        std::atomic<float>* outputGainParam = nullptr;
        std::atomic<float>* colorParam = nullptr;
        std::atomic<float>* bitCrushParam = nullptr;
        std::atomic<float>* delayTimeParam = nullptr;
        std::atomic<float>* delayMixParam = nullptr;
        std::atomic<float>* delayFeedbackParam = nullptr;
        std::atomic<float>* chorusMixParam = nullptr;
        std::atomic<float>* volumeCompParam = nullptr;
        std::atomic<float>* bypassParam = nullptr;

        juce::SmoothedValue<float> inputGainSmoother;
        juce::SmoothedValue<float> outputGainSmoother;

        std::atomic<int> currentTotalLatency{0};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}
