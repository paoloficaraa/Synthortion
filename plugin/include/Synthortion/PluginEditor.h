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

    private:
        static constexpr int kWindowWidth = 800;
        static constexpr int kWindowHeight = 480;

        AudioPluginAudioProcessor& processorRef;
        std::unique_ptr<juce::WebBrowserComponent> webView;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
    };
}
