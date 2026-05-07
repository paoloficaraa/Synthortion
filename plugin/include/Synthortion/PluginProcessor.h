#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <juce_dsp/juce_dsp.h>

#include "Synthortion/WarmDistortion.h"
#include "Synthortion/ParametricEQ.h"
#include "Synthortion/SynthortionChorus.h"
#include "Synthortion/PingPongDelay.h"
#include "Synthortion/BitCrusher.h"

namespace synthortion
{
    // Main audio processor: warm distortion, EQ, chorus, delay, bit-crusher
    class AudioPluginAudioProcessor final : public juce::AudioProcessor
    {
    public:
        AudioPluginAudioProcessor();
        ~AudioPluginAudioProcessor() override;

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

        juce::AudioProcessorValueTreeState apvts;

        void updateDSPParameters();

        void setSpectrumAnalyzerCallback(std::function<void(const float*, int)> callback)
        {
            spectrumAnalyzerCallback = std::move(callback);
        }

        ParametricEQ &getEQ() { return parametricEQ; }
        float getInputRmsLevel() const { return inputRmsLevel.load(std::memory_order_relaxed); }
        float getOutputRmsLevel() const { return outputRmsLevel.load(std::memory_order_relaxed); }

    private:
        static constexpr float kRmsSmoothingCoeff = 0.9f;
        static constexpr float kRmsMinDb = -60.0f;
        static constexpr float kSmootherRampTime = 0.05f;
        static constexpr float kBooleanThreshold = 0.5f;
        static constexpr int kLatencyBufferMargin = 4;

        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void updateAllDSPParameters();
        float calculateRMS(const juce::AudioBuffer<float>& buffer);

        WarmDistortion warmDistortion;
        ParametricEQ parametricEQ;
        SynthortionChorus chorus;
        PingPongDelay pingPongDelay;
        BitCrusher bitCrusher;

        juce::dsp::DryWetMixer<float> globalDryWet;
        juce::AudioBuffer<float> delayMatchedDryBuffer;

        std::atomic<float> inputRmsLevel{kRmsMinDb};
        std::atomic<float> outputRmsLevel{kRmsMinDb};

        std::atomic<float>* inputGainParam = nullptr;
        std::atomic<float>* outputGainParam = nullptr;
        std::atomic<float>* colorParam = nullptr;
        std::atomic<float>* bitCrushParam = nullptr;
        std::atomic<float>* delayTimeParam = nullptr;
        std::atomic<float>* delayMixParam = nullptr;
        std::atomic<float>* delayFeedbackParam = nullptr;
        std::atomic<float>* chorusMixParam = nullptr;
        std::atomic<float>* eqBypassParam = nullptr;
        std::atomic<float>* volumeCompParam = nullptr;

        std::atomic<float>* lowCutFreqParam = nullptr;
        std::atomic<float>* lowCutQParam = nullptr;
        std::atomic<float>* lowMidFreqParam = nullptr;
        std::atomic<float>* lowMidGainParam = nullptr;
        std::atomic<float>* lowMidQParam = nullptr;
        std::atomic<float>* highMidFreqParam = nullptr;
        std::atomic<float>* highMidGainParam = nullptr;
        std::atomic<float>* highMidQParam = nullptr;
        std::atomic<float>* highCutFreqParam = nullptr;
        std::atomic<float>* highCutQParam = nullptr;

        juce::SmoothedValue<float> inputGainSmoother;
        juce::SmoothedValue<float> outputGainSmoother;
        juce::LinearSmoothedValue<float> smoothedColorDrive { 0.0f };
        
        std::atomic<int> currentTotalLatency{0};
        std::function<void(const float*, int)> spectrumAnalyzerCallback;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}