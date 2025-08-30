#pragma once

#include "Synthortion/PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
        ~AudioPluginAudioProcessorEditor() override;

        //==============================================================================
        void resized() override;

    private:
        using Resource = juce::WebBrowserComponent::Resource;
        std::optional<Resource> getResource(const juce::String &url) const;

        // This reference is provided as a quick way for your editor to
        // access the processor object that created it.
        AudioPluginAudioProcessor &processorRef;

        // Web relay components for parameter communication
        juce::WebSliderRelay driveRelay;
        juce::WebSliderRelay mixRelay;
        juce::WebSliderRelay saturationTypeRelay;

        // WebView component
        juce::WebBrowserComponent webView;

        // Web parameter attachments
        juce::WebSliderParameterAttachment driveAttachment;
        juce::WebSliderParameterAttachment mixAttachment;
        juce::WebSliderParameterAttachment saturationTypeAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}
