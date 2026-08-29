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
#include "Synthortion/AudioCaptureFifo.h"
#include "Synthortion/PresetManager.h"
namespace synthortion
{
    struct UIPreferences
    {
        static constexpr const char* kNodeName = "UIPreferences";
        static constexpr const char* kUiScale = "uiScale";
        static constexpr const char* kSpectrumDecay = "spectrumDecay";
        static constexpr const char* kSkipBootSequence = "skipBootSequence";

        static constexpr double kDefaultUiScale = 1.0;
        static constexpr double kDefaultSpectrumDecay = 0.25;
        static constexpr bool kDefaultSkipBootSequence = false;

        static juce::ValueTree createDefaultTree()
        {
            juce::ValueTree tree(kNodeName);
            tree.setProperty(kUiScale, kDefaultUiScale, nullptr);
            tree.setProperty(kSpectrumDecay, kDefaultSpectrumDecay, nullptr);
            tree.setProperty(kSkipBootSequence, kDefaultSkipBootSequence, nullptr);
            return tree;
        }

        static void ensureTree(juce::ValueTree& parent)
        {
            if (!parent.isValid())
                return;

            if (!parent.hasProperty("version"))
                parent.setProperty("version", 1, nullptr);

            auto uiPrefs = parent.getChildWithName(kNodeName);
            if (!uiPrefs.isValid())
            {
                parent.appendChild(createDefaultTree(), nullptr);
            }
        }
    };
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
        void processBlockBypassed(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
        juce::AudioProcessorParameter* getBypassParameter() const override;

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

        juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }

        AudioCaptureFifo& getAudioFifo() noexcept { return audioFifo; }
        const AudioCaptureFifo& getAudioFifo() const noexcept { return audioFifo; }

        struct MeterPeaks { float input = 0.0f; float output = 0.0f; };
        MeterPeaks getMeterPeaks() const noexcept { return { inputPeak.load(), outputPeak.load() }; }
        double getCurrentBpm() const noexcept;
        PresetManager& getPresetManager() noexcept { return presetManager; }
        const PresetManager& getPresetManager() const noexcept { return presetManager; }

    private:
        static constexpr float kSmootherRampTime = 0.05f;
        static constexpr float kBooleanThreshold = 0.5f;
        static constexpr int kLatencyBufferMargin = 4;

        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void captureAndMeasurePeaks(const juce::AudioBuffer<float>& buffer);

        dsp::WarmDistortion warmDistortion;
        dsp::SynthortionChorus chorus;
        dsp::PingPongDelay pingPongDelay;
        dsp::BitCrusher bitCrusher;

        AudioCaptureFifo audioFifo;
        juce::AudioProcessorValueTreeState apvts;
        PresetManager presetManager;

        std::atomic<float>* inputGainParam = nullptr;
        std::atomic<float>* outputGainParam = nullptr;
        std::atomic<float>* colorParam = nullptr;
        std::atomic<float>* bitCrushParam = nullptr;
        std::atomic<float>* delayTimeFreeParam = nullptr;
        std::atomic<float>* delayTimeSyncParam = nullptr;
        std::atomic<float>* delayMixParam = nullptr;
        std::atomic<float>* delayFeedbackParam = nullptr;
        std::atomic<float>* chorusMixParam = nullptr;
        std::atomic<float>* bypassParam = nullptr;
        std::atomic<float>* driveOnParam = nullptr;
        std::atomic<float>* bitcrushOnParam = nullptr;
        std::atomic<float>* delayOnParam = nullptr;
        std::atomic<float>* chorusOnParam = nullptr;
        std::atomic<float>* driveRouteParam = nullptr;
        std::atomic<float>* delaySyncParam = nullptr;
        std::atomic<float>* chorusWideParam = nullptr;

        juce::SmoothedValue<float> inputGainSmoother;
        juce::SmoothedValue<float> outputGainSmoother;
        std::atomic<float> inputPeak{0.0f};
        std::atomic<float> outputPeak{0.0f};

        std::atomic<int> currentTotalLatency{0};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
    };
}
