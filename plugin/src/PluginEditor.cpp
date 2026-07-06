#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    namespace Colours
    {
        const juce::Colour ANTHRACITE(0xFF130D1A);
        const juce::Colour CREAM(0xFFE0E0E0);
        const juce::Colour COPPER(0xFF9B5DE5);
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p),
          processorRef (p),
          distortionPanel ("DISTORTION", lookAndFeel.findColour (AnalogLookAndFeel::panelColourId)),
          modulationPanel ("MODULATION", lookAndFeel.findColour (AnalogLookAndFeel::panelRecessedColourId)),
          gainPanel ("GAIN", lookAndFeel.findColour (AnalogLookAndFeel::panelColourId)),
          bypassComponent (processorRef.apvts, "PLUGIN_BYPASS")
    {
        setOpaque (true);
        setLookAndFeel (&lookAndFeel);
        startTimer (1000 / kTimerHz);

        addAndMakeVisible (distortionPanel);
        addAndMakeVisible (modulationPanel);
        addAndMakeVisible (gainPanel);
        addAndMakeVisible (bypassComponent);

        // DISTORTION
        setupKnobWithLabel (driveKnob, driveTitleLabel, driveLabel, "DRIVE", "COLOR", driveAttachment, distortionPanel);
        setupKnobWithLabel (bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment, distortionPanel);

        // MODULATION
        setupKnobWithLabel (delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "TIME", "DELAY_TIME", delayTimeAttachment, modulationPanel);
        setupKnobWithLabel (delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "FEEDBACK", "DELAY_FEEDBACK", delayFeedbackAttachment, modulationPanel);
        setupKnobWithLabel (delayMixKnob, delayMixTitleLabel, delayMixLabel, "DELAY MIX", "DELAY_MIX", delayMixAttachment, modulationPanel);
        setupKnobWithLabel (chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "CHORUS MIX", "CHORUS_MIX", chorusMixAttachment, modulationPanel);

        // GAIN
        setupKnobWithLabel (inputGainKnob, inputGainTitleLabel, inputGainLabel, "INPUT", "INPUT_GAIN", inputGainAttachment, gainPanel);
        setupKnobWithLabel (outputGainKnob, outputGainTitleLabel, outputGainLabel, "OUTPUT", "OUTPUT_GAIN", outputGainAttachment, gainPanel);

        setResizable (false, false);
        setSize (kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel (nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
    {
        drawRackBackground (g);

        // Corner screws on rack ears
        g.setColour (juce::Colour (0xFF404040));
        auto drawScrew = [&] (int x, int y)
        {
            g.fillEllipse (static_cast<float> (x), static_cast<float> (y), 6.0f, 6.0f);
            g.setColour (juce::Colour (0xFF202020));
            g.drawLine (x + 1.0f, y + 3.0f, x + 5.0f, y + 3.0f, 1.0f);
            g.setColour (juce::Colour (0xFF404040));
        };
        drawScrew (4, 14);
        drawScrew (getWidth() - 11, 14);
        drawScrew (4, getHeight() - 20);
        drawScrew (getWidth() - 11, getHeight() - 20);
    }

    void AudioPluginAudioProcessorEditor::drawRackBackground (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient bgGrad (
            Colours::ANTHRACITE.brighter (0.05f), bounds.getTopLeft(),
            Colours::ANTHRACITE.darker (0.1f), bounds.getBottomLeft(),
            false);
        g.setGradientFill (bgGrad);
        g.fillRect (bounds);

        auto localBounds = getLocalBounds();
        auto leftEar = localBounds.removeFromLeft (kRackEarWidth);
        auto rightEar = localBounds.removeFromRight (kRackEarWidth);

        g.setColour (juce::Colour (0xFF151515));
        g.fillRect (leftEar);
        g.fillRect (rightEar);

        g.setColour (juce::Colour (0xFF252525));
        int numLines = leftEar.getHeight() / 15;
        for (int i = 0; i < numLines; ++i)
        {
            int y = leftEar.getY() + 15 + i * 15;
            g.fillRect (leftEar.getX() + 3, y, leftEar.getWidth() - 6, 2);
            g.fillRect (rightEar.getX() + 3, y, rightEar.getWidth() - 6, 2);
        }

        g.setColour (juce::Colour (0xFF1F1F1F));
        g.drawRect (leftEar, 1);
        g.drawRect (rightEar, 1);
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (kRackEarWidth);
        bounds.removeFromRight (kRackEarWidth);

        constexpr int kTopMargin = 18;
        constexpr int kBottomMargin = 18;

        bounds.removeFromTop (kTopMargin);
        bounds.removeFromBottom (kBottomMargin);

        bypassComponent.setBounds (bounds.removeFromTop (kTopBarHeight));

        auto panelsArea = bounds;

        distortionPanel.setBounds (panelsArea.removeFromLeft (kDistortionColumnWidth));
        panelsArea.removeFromLeft (kSectionGap1);

        modulationPanel.setBounds (panelsArea.removeFromLeft (kModulationColumnWidth));
        panelsArea.removeFromLeft (kSectionGap2);

        gainPanel.setBounds (panelsArea);

        const int largeKnobSize = 78;
        const int smallKnobSize = 55;
        const int labelH = 15;

        // --- DISTORTION SECTION ---
        auto distArea = distortionPanel.getLocalBounds();
        auto distTop = distArea.removeFromTop (kTopSectionHeight);
        auto distBottom = distArea;

        auto driveBounds = juce::Rectangle<int> (distTop.getCentreX() - largeKnobSize / 2,
                                                 distTop.getCentreY() - largeKnobSize / 2,
                                                 largeKnobSize, largeKnobSize);
        driveKnob.setBounds (driveBounds);
        driveTitleLabel.setBounds (juce::Rectangle<int> (driveBounds.getX() - 10, driveBounds.getY() - labelH - 4,
                                                         largeKnobSize + 20, labelH));
        driveLabel.setBounds (juce::Rectangle<int> (driveBounds.getX() - 10, driveBounds.getBottom() + 4,
                                                    largeKnobSize + 20, labelH));

        auto crushBounds = juce::Rectangle<int> (distBottom.getCentreX() - smallKnobSize / 2,
                                                 distBottom.getCentreY() - smallKnobSize / 2,
                                                 smallKnobSize, smallKnobSize);
        bitCrushKnob.setBounds (crushBounds);
        bitCrushTitleLabel.setBounds (juce::Rectangle<int> (crushBounds.getX() - 20, crushBounds.getY() - labelH - 4,
                                                            smallKnobSize + 40, labelH));
        bitCrushLabel.setBounds (juce::Rectangle<int> (crushBounds.getX() - 20, crushBounds.getBottom() + 4,
                                                       smallKnobSize + 40, labelH));

        // --- MODULATION SECTION ---
        auto modArea = modulationPanel.getLocalBounds();
        auto modTopRow = modArea.removeFromTop (kTopSectionHeight);
        auto modBottomRow = modArea;

        auto placeKnobInCell = [&] (juce::Rectangle<int> cell, juce::Slider& knob,
                                    juce::Label& title, juce::Label& val)
        {
            auto kb = juce::Rectangle<int> (cell.getCentreX() - smallKnobSize / 2,
                                            cell.getCentreY() - smallKnobSize / 2,
                                            smallKnobSize, smallKnobSize);
            knob.setBounds (kb);
            title.setBounds (juce::Rectangle<int> (kb.getX() - 20, kb.getY() - labelH - 2,
                                                   smallKnobSize + 40, labelH));
            val.setBounds (juce::Rectangle<int> (kb.getX() - 20, kb.getBottom() + 2,
                                                 smallKnobSize + 40, labelH));
        };

        placeKnobInCell (modTopRow.removeFromLeft (modTopRow.getWidth() / 2), delayTimeKnob, delayTimeTitleLabel, delayTimeLabel);
        placeKnobInCell (modTopRow, delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel);

        placeKnobInCell (modBottomRow.removeFromLeft (modBottomRow.getWidth() / 2), delayMixKnob, delayMixTitleLabel, delayMixLabel);
        placeKnobInCell (modBottomRow, chorusMixKnob, chorusMixTitleLabel, chorusMixLabel);

        // --- GAIN SECTION ---
        auto gainArea = gainPanel.getLocalBounds();
        auto gainTop = gainArea.removeFromTop (kTopSectionHeight);
        auto gainBottom = gainArea;

        auto inputBounds = juce::Rectangle<int> (gainTop.getCentreX() - largeKnobSize / 2,
                                                 gainTop.getCentreY() - largeKnobSize / 2,
                                                 largeKnobSize, largeKnobSize);
        inputGainKnob.setBounds (inputBounds);
        inputGainTitleLabel.setBounds (juce::Rectangle<int> (inputBounds.getX() - 10, inputBounds.getY() - labelH - 4,
                                                             largeKnobSize + 20, labelH));
        inputGainLabel.setBounds (juce::Rectangle<int> (inputBounds.getX() - 10, inputBounds.getBottom() + 4,
                                                        largeKnobSize + 20, labelH));

        auto outputBounds = juce::Rectangle<int> (gainBottom.getCentreX() - smallKnobSize / 2,
                                                  gainBottom.getCentreY() - smallKnobSize / 2,
                                                  smallKnobSize, smallKnobSize);
        outputGainKnob.setBounds (outputBounds);
        outputGainTitleLabel.setBounds (juce::Rectangle<int> (outputBounds.getX() - 20, outputBounds.getY() - labelH - 4,
                                                              smallKnobSize + 40, labelH));
        outputGainLabel.setBounds (juce::Rectangle<int> (outputBounds.getX() - 20, outputBounds.getBottom() + 4,
                                                         smallKnobSize + 40, labelH));
    }

    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        updateMainControlLabels();
    }

    void AudioPluginAudioProcessorEditor::setupKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setRotaryParameters (kRotaryStartAngle, kRotaryEndAngle, true);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob.setVelocityBasedMode (true);
        knob.setVelocityModeParameters (0.5f, 1, 0.1f, false);
    }

    void AudioPluginAudioProcessorEditor::setupKnobWithLabel (
        juce::Slider& knob,
        juce::Label& titleLabel,
        juce::Label& valueLabel,
        const juce::String& title,
        const juce::String& paramId,
        std::unique_ptr<SliderAttachment>& attachment,
        juce::Component& parent)
    {
        setupKnob (knob);
        parent.addAndMakeVisible (knob);
        attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramId, knob);

        titleLabel.setText (title, juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);
        titleLabel.setFont (lookAndFeel.getParameterLabelFont());
        titleLabel.setColour (juce::Label::textColourId, Colours::CREAM);
        parent.addAndMakeVisible (titleLabel);

        valueLabel.setText ("", juce::dontSendNotification);
        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setFont (lookAndFeel.getParameterValueFont());
        valueLabel.setColour (juce::Label::textColourId, Colours::CREAM);
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
