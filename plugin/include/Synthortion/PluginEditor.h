#pragma once

#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/AnimationController.h"
#include "Synthortion/AudioScopeRingBuffer.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/GlitchOverlay.h"
#include "Synthortion/MeterComponent.h"
#include "Synthortion/OscilloscopeComponent.h"
#include "Synthortion/PanelComponent.h"
#include "Synthortion/PluginProcessor.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include <array>
#include <juce_gui_extra/juce_gui_extra.h>

namespace synthortion
{
    class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  public juce::Timer
    {
    public:
        explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
        ~AudioPluginAudioProcessorEditor() override;

        void paint (juce::Graphics&) override;
        void paintOverChildren (juce::Graphics&) override;
        void resized() override;
        void timerCallback() override;
        void visibilityChanged() override;

        BypassComponent& getBypassComponent() noexcept { return bypassComponent; }
        OscilloscopeComponent& getOscilloscope() noexcept { return oscilloscope; }
        MeterComponent& getInputMeter() noexcept { return inputMeter; }
        MeterComponent& getOutputMeter() noexcept { return outputMeter; }
        AnimatedKnob& getInputGainKnob() noexcept { return inputGainKnob; }
        AnimatedKnob& getOutputGainKnob() noexcept { return outputGainKnob; }
        AnimationController& getAnimationController() noexcept { return animationController; }

    private:
        static constexpr int kTimerHz = 60;

        static constexpr int kWindowWidth = 800;
        static constexpr int kWindowHeight = 480;

        static constexpr int kSideBarWidth = 55;
        static constexpr int kTopBarHeight = 90;
        static constexpr int kBypassWidth = 130;
        static constexpr int kGap = 10;

        void setupKnobWithLabel (AnimatedKnob& knob, juce::Label& titleLabel, juce::Label& valueLabel,
                                 const juce::String& title, const juce::String& paramId,
                                 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
                                 juce::Component& parent);

        juce::String formatPercentage (float normalizedValue);
        juce::String formatDB (float dbValue);
        juce::String formatMilliseconds (float ms);

        void updateMainControlLabels();
        void updateBypassState();
        void drawGrainOverlay (juce::Graphics& g);

        AudioPluginAudioProcessor& processorRef;
        SynthortionLookAndFeel lookAndFeel;
        AnimationController animationController;

        PanelComponent distortionPanel;
        PanelComponent chorusPanel;
        PanelComponent delayPanel;
        PanelComponent comingSoonPanel;
        BypassComponent bypassComponent;
        OscilloscopeComponent oscilloscope;
        MeterComponent inputMeter;
        MeterComponent outputMeter;

        // DISTORTION section
        AnimatedKnob driveKnob;
        juce::Label driveTitleLabel;
        juce::Label driveLabel;

        AnimatedKnob bitCrushKnob;
        juce::Label bitCrushTitleLabel;
        juce::Label bitCrushLabel;

        // CHORUS section
        AnimatedKnob chorusMixKnob;
        juce::Label chorusMixTitleLabel;
        juce::Label chorusMixLabel;

        // DELAY section
        AnimatedKnob delayTimeKnob;
        juce::Label delayTimeTitleLabel;
        juce::Label delayTimeLabel;

        AnimatedKnob delayFeedbackKnob;
        juce::Label delayFeedbackTitleLabel;
        juce::Label delayFeedbackLabel;

        AnimatedKnob delayMixKnob;
        juce::Label delayMixTitleLabel;
        juce::Label delayMixLabel;

        // Side bars
        AnimatedKnob inputGainKnob;
        juce::Label inputGainTitleLabel;
        juce::Label inputGainLabel;

        AnimatedKnob outputGainKnob;
        juce::Label outputGainTitleLabel;
        juce::Label outputGainLabel;

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

        std::unique_ptr<SliderAttachment> driveAttachment;
        std::unique_ptr<SliderAttachment> bitCrushAttachment;
        std::unique_ptr<SliderAttachment> chorusMixAttachment;
        std::unique_ptr<SliderAttachment> delayTimeAttachment;
        std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
        std::unique_ptr<SliderAttachment> delayMixAttachment;
        std::unique_ptr<SliderAttachment> inputGainAttachment;
        std::unique_ptr<SliderAttachment> outputGainAttachment;

        GlitchOverlay glitchOverlay;

        bool lastBypassState = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
    };
}
