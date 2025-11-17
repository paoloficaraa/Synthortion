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

        //==============================================================================
        void paint(juce::Graphics &) override;
        void resized() override;

        void timerCallback() override;

    private:
        void setupEQControls();

        // Utility functions for parameter value formatting
        juce::String formatFrequency(float freq);
        juce::String formatGain(float gain);
        juce::String formatQ(float q);
        juce::String formatPercentage(float normalizedValue);
        juce::String formatDB(float dbValue);
        juce::String formatMilliseconds(float ms);
        juce::String formatBitDepth(float bits);
        void updateEQLabels();
        void updateMainControlLabels();

        // This reference is provided as a quick way for your editor to
        // access the processor object that created it.
        AudioPluginAudioProcessor &processorRef;

        SynthortionLookAndFeel lookAndFeel;

        SpectrumAnalyzer spectrumAnalyzer;

        // Main Controls (matching new layout)
        juce::Slider driveKnob;         // Color master mix knob (mapped to COLOR parameter)
        juce::Slider inputGainKnob;     // Left side with meter
        juce::Slider outputGainKnob;    // Right side with meter
        juce::Slider noiseKnob;         // Noise amount knob
        juce::Slider bitCrushKnob;      // BitCrush depth knob
        juce::Slider delayTimeKnob;     // Delay time in ms
        juce::Slider delayMixKnob;      // Delay mix amount
        juce::Slider delayFeedbackKnob; // Delay feedback amount
        juce::Slider chorusMixKnob;     // Chorus mix amount
        juce::ComboBox presetSelector;  // Preset selection

        // Vertical meters for input/output
        synthortion::Gui::VerticalDiscreteMeter inputMeter;
        synthortion::Gui::VerticalDiscreteMeter outputMeter;

        // EQ Controls
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

        // EQ Bypass Button
        juce::ToggleButton eqBypassButton;

        // Labels
        juce::Label driveLabel; // Label for Color knob (driveKnob)
        juce::Label noiseLabel;
        juce::Label bitCrushLabel;
        juce::Label delayTimeLabel;
        juce::Label delayMixLabel;
        juce::Label delayFeedbackLabel;
        juce::Label chorusMixLabel;
        juce::Label presetLabel;

        // Effects title labels
        juce::Label noiseTitleLabel;
        juce::Label bitCrushTitleLabel;
        juce::Label delayTimeTitleLabel;
        juce::Label delayMixTitleLabel;
        juce::Label delayFeedbackTitleLabel;
        juce::Label chorusMixTitleLabel;

        // EQ Labels
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

        // EQ Section Titles
        juce::Label lowCutTitle;
        juce::Label lowMidTitle;
        juce::Label highMidTitle;
        juce::Label highCutTitle;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

        // Control Attachments
        std::unique_ptr<SliderAttachment> driveAttachment; // Attached to COLOR parameter
        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
        std::unique_ptr<SliderAttachment> noiseAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<ComboBoxAttachment> presetAttachment;

        // EQ Attachments
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
