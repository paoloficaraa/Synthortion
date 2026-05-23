#pragma once

#include "Synthortion/PluginProcessor.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::Timer
    {
    public:
        explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
        ~AudioPluginAudioProcessorEditor() override;

        void paint(juce::Graphics &) override;
        void resized() override;
        void timerCallback() override;

    private:
        static constexpr float kRotaryStartAngle = juce::MathConstants<float>::pi * 1.25f;
        static constexpr float kRotaryEndAngle = juce::MathConstants<float>::pi * 2.75f;
        static constexpr float kVelocitySensitivity = 0.5f;
        static constexpr int kVelocityThreshold = 1;
        static constexpr float kVelocityOffset = 0.1f;
        static constexpr int kTimerHz = 60;
        static constexpr int kWindowWidth = 720;
        static constexpr int kWindowHeight = 490;

        void setupKnob(juce::Slider& knob);
        void setupKnobWithLabel(juce::Slider& knob, juce::Label& titleLabel, juce::Label& valueLabel,
                                const juce::String& title, const juce::String& paramId,
                                std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment);

        juce::String formatPercentage(float normalizedValue);
        juce::String formatDB(float dbValue);
        juce::String formatMilliseconds(float ms);

        void updateMainControlLabels();

        AudioPluginAudioProcessor &processorRef;

        SynthortionLookAndFeel lookAndFeel;

        // Main controls
        juce::Slider driveKnob;
        juce::Slider bitCrushKnob;
        juce::Slider delayTimeKnob;
        juce::Slider delayMixKnob;
        juce::Slider delayFeedbackKnob;
        juce::Slider chorusMixKnob;
        juce::ComboBox presetSelector;

        // Labels
        juce::Label driveLabel;
        juce::Label bitCrushLabel;
        juce::Label delayTimeLabel;
        juce::Label delayMixLabel;
        juce::Label delayFeedbackLabel;
        juce::Label chorusMixLabel;
        juce::Label presetLabel;

        juce::Label bitCrushTitleLabel;
        juce::Label delayTimeTitleLabel;
        juce::Label delayMixTitleLabel;
        juce::Label delayFeedbackTitleLabel;
        juce::Label chorusMixTitleLabel;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<ComboBoxAttachment> presetAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}
