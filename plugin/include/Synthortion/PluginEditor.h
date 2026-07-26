#pragma once

#include "Synthortion/PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <memory>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
    {
    public:
        explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
        ~AudioPluginAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        static constexpr int kWindowWidth = 800;
        static constexpr int kWindowHeight = 480;

        static constexpr int kHeaderHeight = 60;
        static constexpr int kSidebarWidth = 120;
        static constexpr int kSeparatorY1 = 60;
        static constexpr int kSeparatorY2 = 240;
        static constexpr int kSeparatorY3 = 420;

        static juce::String formatDB (float dbValue);
        static juce::String formatPercentage (float normalizedValue);
        static juce::String formatMilliseconds (float msValue);

        AudioPluginAudioProcessor& processorRef;

        juce::Label titleLabel;
        juce::Label comingSoonLabel;

        juce::Label inputGainTitleLabel;
        juce::Label outputGainTitleLabel;
        juce::Label colorTitleLabel;
        juce::Label bitCrushTitleLabel;
        juce::Label chorusMixTitleLabel;
        juce::Label delayTimeTitleLabel;
        juce::Label delayFeedbackTitleLabel;
        juce::Label delayMixTitleLabel;

        juce::Slider inputGainSlider;
        juce::Slider outputGainSlider;
        juce::Slider colorSlider;
        juce::Slider bitCrushSlider;
        juce::Slider chorusMixSlider;
        juce::Slider delayTimeSlider;
        juce::Slider delayFeedbackSlider;
        juce::Slider delayMixSlider;

        juce::ToggleButton bypassButton;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
        std::unique_ptr<SliderAttachment> colorAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<ButtonAttachment> bypassAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
    };
}
