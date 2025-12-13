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
    /**
     * @brief Main audio processor for the Synthortion distortion plugin
     *
     * Features:
     * - High-quality warm distortion with multiple saturation types
     * - 4-band parametric equalizer
     * - Real-time spectrum analysis
     * - Input/output level metering
     * - Full parameter automation support
     */
    class AudioPluginAudioProcessor final : public juce::AudioProcessor
    {
    public:
        //==============================================================================
        AudioPluginAudioProcessor();
        ~AudioPluginAudioProcessor() override;

        //==============================================================================
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
        using AudioProcessor::processBlock;

        //==============================================================================
        juce::AudioProcessorEditor *createEditor() override;
        bool hasEditor() const override;

        //==============================================================================
        const juce::String getName() const override;

        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;

        //==============================================================================
        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String &newName) override;

        //==============================================================================
        void getStateInformation(juce::MemoryBlock &destData) override;
        void setStateInformation(const void *data, int sizeInBytes) override;

        // Parameter tree for automation and preset management
        juce::AudioProcessorValueTreeState apvts;

        //==============================================================================
        // Force update all DSP parameters (used during initialization)
        void updateDSPParameters();

        // Spectrum analyzer callback for real-time visualization
        // Optimized to pass a block of samples instead of per-sample calls
        std::function<void(const float*, int)> spectrumAnalyzerCallback;

        /**
         * @brief Set callback function for spectrum analyzer
         * @param callback Function to receive audio samples for analysis
         */
        void setSpectrumAnalyzerCallback(std::function<void(const float*, int)> callback)
        {
            spectrumAnalyzerCallback = std::move(callback);
        }

        /**
         * @brief Get reference to EQ for UI control
         * @return Reference to the internal ParametricEQ instance
         */
        ParametricEQ &getEQ() { return parametricEQ; }

        /**
         * @brief Get current input RMS level in dB
         * @return Input level [-60.0 to 0.0] dB
         */
        float getInputRmsLevel() const { return inputRmsLevel.load(std::memory_order_relaxed); }

        /**
         * @brief Get current output RMS level in dB
         * @return Output level [-60.0 to 0.0] dB
         */
        float getOutputRmsLevel() const { return outputRmsLevel.load(std::memory_order_relaxed); }

    private:
        //==============================================================================
        /**
         * @brief Create parameter layout for APVTS
         * @return Parameter layout with all plugin parameters
         */
        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

        // Audio processing components
        WarmDistortion warmDistortion;
        ParametricEQ parametricEQ;

        // Global dry/wet mixer with latency compensation (applied at end of chain)
        juce::dsp::DryWetMixer<float> globalDryWet;
        juce::AudioBuffer<float> delayMatchedDryBuffer; // Buffer for latency-compensated dry signal

        // RMS level tracking for meters (atomic for thread safety)
        std::atomic<float> inputRmsLevel{-60.0f};
        std::atomic<float> outputRmsLevel{-60.0f};

        SynthortionChorus chorus;
        PingPongDelay pingPongDelay;
        BitCrusher bitCrusher;

        // Cached parameter pointers for fast access in processBlock
        std::atomic<float>* driveParam = nullptr;
        std::atomic<float>* inputGainParam = nullptr;
        std::atomic<float>* outputGainParam = nullptr;
        std::atomic<float>* colorParam = nullptr;
        std::atomic<float>* bitCrushParam = nullptr;
        std::atomic<float>* dacNoiseParam = nullptr;
        std::atomic<float>* delayTimeParam = nullptr;
        std::atomic<float>* delayMixParam = nullptr;
        std::atomic<float>* delayFeedbackParam = nullptr;
        std::atomic<float>* chorusMixParam = nullptr;
        std::atomic<float>* eqBypassParam = nullptr;
        std::atomic<float>* volumeCompParam = nullptr;

        // EQ Parameters
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

        // Parameter smoothers for critical audio-quality parameters (eliminates clicks/pops)
        // NOTE: Color is NOT smoothed - instant response required for phase-accurate dry/wet mixing
        juce::SmoothedValue<float> driveSmoother;
        juce::SmoothedValue<float> inputGainSmoother;
        juce::SmoothedValue<float> outputGainSmoother;
        
        // Bypass state tracking (instant switching)
        bool previousEqBypassState = false;
        
        // Dynamic latency tracking
        std::atomic<int> currentTotalLatency{0};

        // Add preset loading method
        //void loadPreset(int presetIndex);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}