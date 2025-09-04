#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "Synthortion/WarmDistortion.h"
#include "Synthortion/ParametricEQ.h"

namespace synthortion
{
    //==============================================================================
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

        juce::AudioProcessorValueTreeState apvts;

        // Spectrum analyzer callback
        std::function<void(float)> spectrumAnalyzerCallback;
        void setSpectrumAnalyzerCallback(std::function<void(float)> callback)
        {
            spectrumAnalyzerCallback = std::move(callback);
        }

        // EQ access for UI
        ParametricEQ &getEQ() { return parametricEQ; }

        // RMS level access for meters
        float getInputRmsLevel() const { return inputRmsLevel.getCurrentValue(); }
        float getOutputRmsLevel() const { return outputRmsLevel.getCurrentValue(); }

    private:
        //==============================================================================
        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        WarmDistortion warmDistortion;
        ParametricEQ parametricEQ;

        // RMS level tracking
        juce::LinearSmoothedValue<float> inputRmsLevel{-60.0f};
        juce::LinearSmoothedValue<float> outputRmsLevel{-60.0f};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}