#pragma once

#include "Synthortion/PluginProcessor.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/SpectrumAnalyzer.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
        ~AudioPluginAudioProcessorEditor() override;

        //==============================================================================
        void paint(juce::Graphics &) override;
        void resized() override;

    private:
        void setupEQControls();

        // This reference is provided as a quick way for your editor to
        // access the processor object that created it.
        AudioPluginAudioProcessor &processorRef;

        SynthortionLookAndFeel lookAndFeel;

        SpectrumAnalyzer spectrumAnalyzer;

        juce::Slider driveKnob;
        juce::Slider mixKnob;
        juce::Slider outputGainKnob;
        juce::ComboBox saturationTypeSelector;

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

        // Labels
        juce::Label driveLabel;
        juce::Label mixLabel;
        juce::Label outputGainLabel;
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

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

        // Distortion Attachments
        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> mixAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;
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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
    };
}
