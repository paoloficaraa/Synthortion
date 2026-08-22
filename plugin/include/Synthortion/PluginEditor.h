#pragma once

#include "Synthortion/PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
        ~AudioPluginAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;

        void parameterChanged (const juce::String& parameterID, float newValue) override;
        void sendParameterChange (const juce::String& parameterID, float newValue);
        void sendAllParameters();

        std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    private:
        static constexpr int kDefaultWidth = 800;
        static constexpr int kDefaultHeight = 480;
        static constexpr int kMinWidth = 600;
        static constexpr int kMinHeight = 360;
        static constexpr int kMaxWidth = 1600;
        static constexpr int kMaxHeight = 960;
        static constexpr double kAspectRatio = static_cast<double> (kDefaultWidth) / static_cast<double> (kDefaultHeight);
        static juce::File getDistDirectory();

        AudioPluginAudioProcessor& processorRef;
        std::unique_ptr<juce::WebBrowserComponent> webView;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
    };
}
