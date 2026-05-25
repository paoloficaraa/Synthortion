#pragma once

#include "Synthortion/PluginProcessor.h"
#include "Synthortion/AnalogLookAndFeel.h"
#include <gin_plugin/gin_plugin.h>
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
        static constexpr int kTimerHz = 60;
        static constexpr int kWindowWidth = 660;
        static constexpr int kWindowHeight = 260;
        static constexpr int kRackEarWidth = 15;

        static constexpr int kSectionGap = 12;

        void setupKnob(juce::Slider& knob);
        void setupKnobWithLabel(juce::Slider& knob, juce::Label& titleLabel, juce::Label& valueLabel,
                                const juce::String& title, const juce::String& paramId,
                                std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment);

        juce::String formatPercentage(float normalizedValue);
        juce::String formatDB(float dbValue);
        juce::String formatMilliseconds(float ms);

        void updateMainControlLabels();
        void drawRackBackground(juce::Graphics& g);

        AudioPluginAudioProcessor &processorRef;
        AnalogLookAndFeel lookAndFeel;

        // WARM DIST section - Large drive knob
        juce::Slider driveKnob;
        juce::Label driveTitleLabel;
        juce::Label driveLabel;

        // EFFECTS section - BitCrush (row 1) and Delay controls (row 2)
        juce::Slider bitCrushKnob;
        juce::Label bitCrushTitleLabel;
        juce::Label bitCrushLabel;

        juce::Slider chorusMixKnob;
        juce::Label chorusMixTitleLabel;
        juce::Label chorusMixLabel;

        juce::Slider delayTimeKnob;
        juce::Label delayTimeTitleLabel;
        juce::Label delayTimeLabel;

        juce::Slider delayFeedbackKnob;
        juce::Label delayFeedbackTitleLabel;
        juce::Label delayFeedbackLabel;

        juce::Slider delayMixKnob;
        juce::Label delayMixTitleLabel;
        juce::Label delayMixLabel;

        // GAIN section
        juce::Slider inputGainKnob;
        juce::Label inputGainTitleLabel;
        juce::Label inputGainLabel;

        juce::Slider outputGainKnob;
        juce::Label outputGainTitleLabel;
        juce::Label outputGainLabel;

        // Global bypass toggle
        juce::ToggleButton pluginBypassButton;

        juce::Rectangle<int> warmDistBounds;
        juce::Rectangle<int> effectsBounds;
        juce::Rectangle<int> gainBounds;
        juce::Rectangle<int> warmDistTitleBounds;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
        std::unique_ptr<ButtonAttachment> bypassAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}