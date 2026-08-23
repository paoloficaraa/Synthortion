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
                                                  public juce::Timer
    {
    public:
        explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
        ~AudioPluginAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;

        void parameterChanged (const juce::String& parameterID, float newValue) override;
        void sendParameterChange (const juce::String& parameterID, float newValue);
        void sendAllParameters();
        void timerCallback() override;
        void sendSpectrumFrame (const std::array<float, SpectrumAnalyzer::kNumBands>& magnitudes);
        void sendMeterFrame(float inputPeak, float outputPeak);
        SpectrumAnalyzer& getSpectrumAnalyzer() noexcept { return spectrumAnalyzer; }
        const SpectrumAnalyzer& getSpectrumAnalyzer() const noexcept { return spectrumAnalyzer; }


        std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    private:
        static constexpr int kDefaultWidth = 960;
        static constexpr int kDefaultHeight = 600;
        static constexpr int kMinWidth = 768;
        static constexpr int kMinHeight = 480;
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
