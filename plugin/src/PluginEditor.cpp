#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    namespace Colours
    {
        const juce::Colour WHITE (0xFFFFFFFF);
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p),
          processorRef (p),
          lookAndFeel (),
          animationController (this),
          distortionPanel ("DISTORTION", lookAndFeel.findColour (SynthortionLookAndFeel::panelFillColourId)),
          chorusPanel ("CHORUS", lookAndFeel.findColour (SynthortionLookAndFeel::panelOutlineColourId)),
          delayPanel ("DELAY", lookAndFeel.findColour (SynthortionLookAndFeel::panelFillColourId)),
          comingSoonPanel ("COMING SOON", lookAndFeel.findColour (SynthortionLookAndFeel::panelOutlineColourId)),
          bypassComponent (processorRef.apvts, "PLUGIN_BYPASS", &animationController),
          oscilloscope (processorRef.getScopeBuffer(), &animationController),
          inputMeter (animationController),
          outputMeter (animationController),
          driveKnob (animationController),
          bitCrushKnob (animationController),
          chorusMixKnob (animationController),
          delayTimeKnob (animationController),
          delayFeedbackKnob (animationController),
          delayMixKnob (animationController),
          inputGainKnob (animationController),
          outputGainKnob (animationController)
    {
        setOpaque (true);
        setLookAndFeel (&lookAndFeel);
        startTimer (1000 / kTimerHz);

        addAndMakeVisible (distortionPanel);
        addAndMakeVisible (chorusPanel);
        addAndMakeVisible (delayPanel);
        addAndMakeVisible (comingSoonPanel);
        comingSoonPanel.setPlaceholder (true);
        comingSoonPanel.setGlitchOverlay (&glitchOverlay);
        addAndMakeVisible (bypassComponent);
        addAndMakeVisible (oscilloscope);
        addAndMakeVisible (inputMeter);
        addAndMakeVisible (outputMeter);

        // DISTORTION
        setupKnobWithLabel (driveKnob, driveTitleLabel, driveLabel, "COLOR", "COLOR", driveAttachment, distortionPanel);
        driveKnob.setKnobStyle (AnimatedKnob::KnobStyle::Canonical);
        setupKnobWithLabel (bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment, distortionPanel);
        bitCrushKnob.setKnobStyle (AnimatedKnob::KnobStyle::Canonical);

        // CHORUS
        setupKnobWithLabel (chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "MIX", "CHORUS_MIX", chorusMixAttachment, chorusPanel);
        chorusMixKnob.setKnobStyle (AnimatedKnob::KnobStyle::Outline);

        // DELAY
        setupKnobWithLabel (delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "TIME", "DELAY_TIME", delayTimeAttachment, delayPanel);
        delayTimeKnob.setKnobStyle (AnimatedKnob::KnobStyle::Outline);
        setupKnobWithLabel (delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "FB", "DELAY_FEEDBACK", delayFeedbackAttachment, delayPanel);
        delayFeedbackKnob.setKnobStyle (AnimatedKnob::KnobStyle::Outline);
        setupKnobWithLabel (delayMixKnob, delayMixTitleLabel, delayMixLabel, "MIX", "DELAY_MIX", delayMixAttachment, delayPanel);
        delayMixKnob.setKnobStyle (AnimatedKnob::KnobStyle::Outline);

        // SIDE BARS
        setupKnobWithLabel (inputGainKnob, inputGainTitleLabel, inputGainLabel, "INPUT", "INPUT_GAIN", inputGainAttachment, *this);
        inputGainKnob.setKnobStyle (AnimatedKnob::KnobStyle::Canonical);
        setupKnobWithLabel (outputGainKnob, outputGainTitleLabel, outputGainLabel, "OUTPUT", "OUTPUT_GAIN", outputGainAttachment, *this);
        outputGainKnob.setKnobStyle (AnimatedKnob::KnobStyle::Canonical);

        setResizable (false, false);
        setSize (kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel (nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colour (0xFF000000));

        const auto bounds = getLocalBounds();
        drawGrainOverlay (g);
        glitchOverlay.drawScanlines (g, bounds);
    }

    void AudioPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
    {
        glitchOverlay.drawDeadPixelScatter (g, getLocalBounds());
    }

    void AudioPluginAudioProcessorEditor::drawGrainOverlay (juce::Graphics& g)
    {
        glitchOverlay.drawDitherNoise (g, getLocalBounds());
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (kRackEarWidth);
        bounds.removeFromRight (kRackEarWidth);

        // Top bar
        auto topBar = bounds.removeFromTop (kTopBarHeight);
        bypassComponent.setBounds (topBar.removeFromLeft (kBypassWidth));
        topBar.removeFromLeft (kGap);
        oscilloscope.setBounds (topBar);

        // Side bars
        auto centerArea = bounds;
        auto leftBar = centerArea.removeFromLeft (kSideBarWidth);
        auto rightBar = centerArea.removeFromRight (kSideBarWidth);

        const int knobSize = 45;
        const int labelH = 12;
        const int meterH = leftBar.getHeight() - knobSize - labelH * 2 - kGap * 2;

        inputMeter.setBounds (leftBar.removeFromTop (meterH).reduced (2, 0));
        leftBar.removeFromTop (kGap);
        inputGainKnob.setBounds (leftBar.removeFromTop (knobSize));
        inputGainTitleLabel.setBounds (leftBar.removeFromTop (labelH));
        inputGainLabel.setBounds (leftBar.removeFromTop (labelH));

        outputMeter.setBounds (rightBar.removeFromTop (meterH).reduced (2, 0));
        rightBar.removeFromTop (kGap);
        outputGainKnob.setBounds (rightBar.removeFromTop (knobSize));
        outputGainTitleLabel.setBounds (rightBar.removeFromTop (labelH));
        outputGainLabel.setBounds (rightBar.removeFromTop (labelH));

        // Center panels
        centerArea.removeFromTop (kGap);
        centerArea.removeFromBottom (kGap);
        centerArea.removeFromLeft (kGap);
        centerArea.removeFromRight (kGap);

        auto distortionArea = centerArea.removeFromTop (centerArea.getHeight() * 55 / 100);
        auto bottomRow = centerArea;
        bottomRow.removeFromTop (kGap);

        distortionPanel.setBounds (distortionArea);

        const int comingSoonWidth = 110;
        comingSoonPanel.setBounds (bottomRow.removeFromRight (comingSoonWidth));
        bottomRow.removeFromRight (kGap);

        delayPanel.setBounds (bottomRow.removeFromRight (bottomRow.getWidth() * 55 / 100));
        bottomRow.removeFromRight (kGap);
        chorusPanel.setBounds (bottomRow);

        // Layout knobs inside panels
        const int largeKnob = 78;
        const int smallKnob = 50;

        auto distArea = distortionPanel.getLocalBounds();
        auto distLeft = distArea.removeFromLeft (distArea.getWidth() / 2);
        auto distRight = distArea;

        driveKnob.setBounds (distLeft.withSizeKeepingCentre (largeKnob, largeKnob));
        driveTitleLabel.setBounds (distLeft.getX(), driveKnob.getY() - labelH - 2, distLeft.getWidth(), labelH);
        driveLabel.setBounds (distLeft.getX(), driveKnob.getBottom() + 2, distLeft.getWidth(), labelH);

        bitCrushKnob.setBounds (distRight.withSizeKeepingCentre (largeKnob, largeKnob));
        bitCrushTitleLabel.setBounds (distRight.getX(), bitCrushKnob.getY() - labelH - 2, distRight.getWidth(), labelH);
        bitCrushLabel.setBounds (distRight.getX(), bitCrushKnob.getBottom() + 2, distRight.getWidth(), labelH);

        chorusMixKnob.setBounds (chorusPanel.getLocalBounds().withSizeKeepingCentre (smallKnob, smallKnob));
        chorusMixTitleLabel.setBounds (chorusPanel.getLocalBounds().getX(), chorusMixKnob.getY() - labelH - 2, chorusPanel.getLocalBounds().getWidth(), labelH);
        chorusMixLabel.setBounds (chorusPanel.getLocalBounds().getX(), chorusMixKnob.getBottom() + 2, chorusPanel.getLocalBounds().getWidth(), labelH);

        auto delayArea = delayPanel.getLocalBounds();
        auto d1 = delayArea.removeFromLeft (delayArea.getWidth() / 3);
        auto d2 = delayArea.removeFromLeft (delayArea.getWidth() / 2);
        auto d3 = delayArea;

        delayTimeKnob.setBounds (d1.withSizeKeepingCentre (smallKnob, smallKnob));
        delayTimeTitleLabel.setBounds (d1.getX(), delayTimeKnob.getY() - labelH - 2, d1.getWidth(), labelH);
        delayTimeLabel.setBounds (d1.getX(), delayTimeKnob.getBottom() + 2, d1.getWidth(), labelH);

        delayFeedbackKnob.setBounds (d2.withSizeKeepingCentre (smallKnob, smallKnob));
        delayFeedbackTitleLabel.setBounds (d2.getX(), delayFeedbackKnob.getY() - labelH - 2, d2.getWidth(), labelH);
        delayFeedbackLabel.setBounds (d2.getX(), delayFeedbackKnob.getBottom() + 2, d2.getWidth(), labelH);

        delayMixKnob.setBounds (d3.withSizeKeepingCentre (smallKnob, smallKnob));
        delayMixTitleLabel.setBounds (d3.getX(), delayMixKnob.getY() - labelH - 2, d3.getWidth(), labelH);
        delayMixLabel.setBounds (d3.getX(), delayMixKnob.getBottom() + 2, d3.getWidth(), labelH);
    }

    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        updateMainControlLabels();
        updateBypassState();
        glitchOverlay.tick();

        lookAndFeel.setBypassMix (animationController.getBypassMix());

        inputMeter.updateFromBuffer (oscilloscope.getCurrentInputBuffer());
        outputMeter.updateFromBuffer (oscilloscope.getCurrentOutputBuffer());
    }

    void AudioPluginAudioProcessorEditor::updateBypassState()
    {
        const bool bypass = processorRef.apvts.getRawParameterValue ("PLUGIN_BYPASS")->load() > 0.5f;
        if (bypass != lastBypassState)
        {
            lastBypassState = bypass;
            animationController.startBypassTransition (bypass);
            oscilloscope.setBypassed (bypass);
            inputMeter.setBypassed (bypass);
            outputMeter.setBypassed (bypass);
        }
    }

    void AudioPluginAudioProcessorEditor::setupKnobWithLabel (
        AnimatedKnob& knob,
        juce::Label& titleLabel,
        juce::Label& valueLabel,
        const juce::String& title,
        const juce::String& paramId,
        std::unique_ptr<SliderAttachment>& attachment,
        juce::Component& parent)
    {
        parent.addAndMakeVisible (knob);
        attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramId, knob);
        knob.snapToCurrentValue();

        titleLabel.setText (title, juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);
        titleLabel.setFont (lookAndFeel.getParameterLabelFont());
        titleLabel.setColour (juce::Label::textColourId, Colours::WHITE);
        parent.addAndMakeVisible (titleLabel);

        valueLabel.setText ("", juce::dontSendNotification);
        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setFont (lookAndFeel.getParameterValueFont());
        valueLabel.setColour (juce::Label::textColourId, Colours::WHITE);
        parent.addAndMakeVisible (valueLabel);
    }

    juce::String AudioPluginAudioProcessorEditor::formatPercentage (float normalizedValue)
    {
        int percentage = static_cast<int> (normalizedValue * 100.0f);
        return juce::String (percentage) + "%";
    }

    juce::String AudioPluginAudioProcessorEditor::formatDB (float dbValue)
    {
        if (dbValue > 0.0f)
            return "+" + juce::String (dbValue, 1) + " dB";

        if (dbValue < 0.0f)
            return juce::String (dbValue, 1) + " dB";

        return "0 dB";
    }

    juce::String AudioPluginAudioProcessorEditor::formatMilliseconds (float ms)
    {
        if (ms >= 1000.0f)
            return juce::String (ms / 1000.0f, 2) + " s";

        return juce::String (static_cast<int> (ms)) + " ms";
    }

    void AudioPluginAudioProcessorEditor::updateMainControlLabels()
    {
        auto colorValue = processorRef.apvts.getRawParameterValue ("COLOR")->load();
        driveLabel.setText (formatPercentage (colorValue), juce::dontSendNotification);

        auto bitCrushValue = processorRef.apvts.getRawParameterValue ("BITCRUSH")->load();
        bitCrushLabel.setText (formatPercentage (bitCrushValue), juce::dontSendNotification);

        auto delayTimeValue = processorRef.apvts.getRawParameterValue ("DELAY_TIME")->load();
        delayTimeLabel.setText (formatMilliseconds (delayTimeValue), juce::dontSendNotification);

        auto delayMixValue = processorRef.apvts.getRawParameterValue ("DELAY_MIX")->load();
        delayMixLabel.setText (formatPercentage (delayMixValue), juce::dontSendNotification);

        auto delayFeedbackValue = processorRef.apvts.getRawParameterValue ("DELAY_FEEDBACK")->load();
        delayFeedbackLabel.setText (formatPercentage (delayFeedbackValue), juce::dontSendNotification);

        auto chorusMixValue = processorRef.apvts.getRawParameterValue ("CHORUS_MIX")->load();
        chorusMixLabel.setText (formatPercentage (chorusMixValue), juce::dontSendNotification);

        auto inputGainValue = processorRef.apvts.getRawParameterValue ("INPUT_GAIN")->load();
        inputGainLabel.setText (formatDB (inputGainValue), juce::dontSendNotification);

        auto outputGainValue = processorRef.apvts.getRawParameterValue ("OUTPUT_GAIN")->load();
        outputGainLabel.setText (formatDB (outputGainValue), juce::dontSendNotification);
    }
}
