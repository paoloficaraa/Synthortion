#pragma once

#include "Synthortion/PluginProcessor.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/SpectrumAnalyzer.h"
#include "Synthortion/VerticalDiscreteMeter.h"
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

        void setupEQControls();
        void setupKnob(juce::Slider& knob);
        void setupKnobWithLabel(juce::Slider& knob, juce::Label& titleLabel, juce::Label& valueLabel,
                                const juce::String& title, const juce::String& paramId,
                                std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment);

        juce::String formatFrequency(float freq);
        juce::String formatQ(float q);
        juce::String formatPercentage(float normalizedValue);
        juce::String formatDB(float dbValue);
        juce::String formatMilliseconds(float ms);
        
        void updateEQLabels();
        void updateMainControlLabels();

        AudioPluginAudioProcessor &processorRef;

        SynthortionLookAndFeel lookAndFeel;
        SpectrumAnalyzer spectrumAnalyzer;

        // Main controls
        juce::Slider driveKnob;
        juce::Slider inputGainKnob;
        juce::Slider outputGainKnob;
        juce::Slider bitCrushKnob;
        juce::Slider delayTimeKnob;
        juce::Slider delayMixKnob;
        juce::Slider delayFeedbackKnob;
        juce::Slider chorusMixKnob;
        juce::ComboBox presetSelector;

        synthortion::Gui::VerticalDiscreteMeter inputMeter;
        synthortion::Gui::VerticalDiscreteMeter outputMeter;

        // EQ controls
        juce::Slider lowCutFreqKnob;
        juce::Slider lowCutQKnob;
        juce::Slider lowMidFreqKnob;
        juce::Slider lowMidGainKnob;
        juce::Slider lowMidQKnob;
        juce::Slider highMidFreqKnob;
        juce::Slider highMidGainKnob;
        juce::Slider highMidQKnob;
        juce::Slider highCutFreqKnob;
        juce::Slider highCutQKnob;

        juce::ToggleButton eqBypassButton;

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

        juce::Label lowCutFreqLabel;
        juce::Label lowCutQLabel;
        juce::Label lowMidFreqLabel;
        juce::Label lowMidGainLabel;
        juce::Label lowMidQLabel;
        juce::Label highMidFreqLabel;
        juce::Label highMidGainLabel;
        juce::Label highMidQLabel;
        juce::Label highCutFreqLabel;
        juce::Label highCutQLabel;

        juce::Label lowCutTitle;
        juce::Label lowMidTitle;
        juce::Label highMidTitle;
        juce::Label highCutTitle;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<ComboBoxAttachment> presetAttachment;

        std::unique_ptr<SliderAttachment> lowCutFreqAttachment;
        std::unique_ptr<SliderAttachment> lowCutQAttachment;
        std::unique_ptr<SliderAttachment> lowMidFreqAttachment;
        std::unique_ptr<SliderAttachment> lowMidGainAttachment;
        std::unique_ptr<SliderAttachment> lowMidQAttachment;
        std::unique_ptr<SliderAttachment> highMidFreqAttachment;
        std::unique_ptr<SliderAttachment> highMidGainAttachment;
        std::unique_ptr<SliderAttachment> highMidQAttachment;
        std::unique_ptr<SliderAttachment> highCutFreqAttachment;
        std::unique_ptr<SliderAttachment> highCutQAttachment;
        std::unique_ptr<ButtonAttachment> eqBypassAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}
