#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "Synthortion/WarmDistortion.h"
#include "Synthortion/ParametricEQ.h"
#include <juce_dsp/juce_dsp.h>

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

        // Spectrum analyzer callback for real-time visualization
        std::function<void(float)> spectrumAnalyzerCallback;

        /**
         * @brief Set callback function for spectrum analyzer
         * @param callback Function to receive audio samples for analysis
         */
        void setSpectrumAnalyzerCallback(std::function<void(float)> callback)
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
        float getInputRmsLevel() const { return inputRmsLevel.getCurrentValue(); }

        /**
         * @brief Get current output RMS level in dB
         * @return Output level [-60.0 to 0.0] dB
         */
        float getOutputRmsLevel() const { return outputRmsLevel.getCurrentValue(); }

    private:
        //==============================================================================
        /**
         * @brief Create parameter layout for APVTS
         * @return Parameter layout with all plugin parameters
         */
        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

        // Audio processing components
        WarmDistortion warmDistortion; ///< Main distortion processor
        ParametricEQ parametricEQ;     ///< 4-band equalizer

        // Global dry/wet mixer (applied at end of chain)
        juce::dsp::DryWetMixer<float> globalDryWet;

        // RMS level tracking for meters (smoothed values in dB)
        juce::LinearSmoothedValue<float> inputRmsLevel{-60.0f};  ///< Input level meter
        juce::LinearSmoothedValue<float> outputRmsLevel{-60.0f}; ///< Output level meter

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}