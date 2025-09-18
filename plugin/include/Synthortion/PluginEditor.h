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

        // This reference is provided as a quick way for your editor to
        // access the processor object that created it.
        AudioPluginAudioProcessor &processorRef;

        SynthortionLookAndFeel lookAndFeel;

        SpectrumAnalyzer spectrumAnalyzer;

        // Main Controls (matching image layout)
        juce::Slider driveKnob;
        juce::Slider inputGainKnob;  // Left side with meter
        juce::Slider outputGainKnob; // Right side with meter
        juce::Slider mixKnob;        // DRY/WET knob
        juce::Slider delayKnob;      // Delay knob
        juce::Slider chorusKnob;     // Chorus knob
        juce::ComboBox saturationTypeSelector;

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

        // Linear Phase toggle button for EQ
        juce::ToggleButton linearPhaseButton;

        // Labels
        juce::Label driveLabel;
        juce::Label inputGainLabel;
        juce::Label outputGainLabel;
        juce::Label mixLabel;
        juce::Label delayLabel;
        juce::Label chorusLabel;
        juce::Label saturationTypeLabel;

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

        // Distortion Attachments
        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
        std::unique_ptr<SliderAttachment> mixAttachment;
        std::unique_ptr<SliderAttachment> delayAttachment;
        std::unique_ptr<SliderAttachment> chorusAttachment;
        std::unique_ptr<ComboBoxAttachment> saturationTypeAttachment;

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

        // Linear Phase button attachment
        std::unique_ptr<ButtonAttachment> linearPhaseAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}
