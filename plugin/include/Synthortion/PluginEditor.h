#pragma once

#include "Synthortion/PluginProcessor.h"
#include "Synthortion/SpectrumAnalyzer.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_events/juce_events.h>
#include <memory>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  public juce::AudioProcessorValueTreeState::Listener,
                                                  public juce::ValueTree::Listener,
                                                  public juce::Timer
    {
    public:
        explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
        ~AudioPluginAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;

        void parameterChanged (const juce::String& parameterID, float newValue) override;
        void sendParameterChange (const juce::String& parameterID);
        juce::var buildInitPayload();
        void handleConnect();
        void handleSetParameter (const juce::var& data);
        void handleRequestPresetList();
        void handleLoadPreset (const juce::var& data);
        void handleSavePreset (const juce::var& data);
        void handleDeletePreset (const juce::var& data);
        void sendPresetListUpdate();
        juce::var buildPresetListPayload();
        void sendPresetOperationResult (bool success, const juce::String& op, const juce::String& errorCode, const juce::String& message);
        void sendUIPreferencesChange();
        juce::var buildUIPreferencesPayload();
        void timerCallback() override;
        void sendSpectrumFrame (const std::array<float, SpectrumAnalyzer::kNumBands>& magnitudes);
        juce::var buildSpectrumPayload (const std::array<float, SpectrumAnalyzer::kNumBands>& magnitudes);
        void sendMeterFrame(AudioPluginAudioProcessor::MeterPeaks peaks);
        juce::var buildMeterPayload(AudioPluginAudioProcessor::MeterPeaks peaks);
        SpectrumAnalyzer& getSpectrumAnalyzer() noexcept { return spectrumAnalyzer; }
        const SpectrumAnalyzer& getSpectrumAnalyzer() const noexcept { return spectrumAnalyzer; }


        void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
        void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
        void valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree&, int) override {}
        void valueTreeChildOrderChanged (juce::ValueTree&, int, int) override {}
        void valueTreeParentChanged (juce::ValueTree&) override {}
        void valueTreeRedirected (juce::ValueTree&) override;

        std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    private:
        static constexpr int kDefaultWidth = 1056;
        static constexpr int kDefaultHeight = 660;
        static constexpr int kMinWidth = 1056;
        static constexpr int kMinHeight = 660;
        static constexpr int kMaxWidth = 1920;
        static constexpr int kMaxHeight = 1200;
        static constexpr double kAspectRatio = static_cast<double> (kDefaultWidth) / static_cast<double> (kDefaultHeight);
        static juce::File getDistDirectory();
        AudioPluginAudioProcessor& processorRef;
        std::unique_ptr<juce::WebBrowserComponent> webView;

        SpectrumAnalyzer spectrumAnalyzer;
        std::array<float, SpectrumAnalyzer::kFftSize> analysisBuffer {};
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
    };
}
