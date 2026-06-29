#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    namespace
    {
        const juce::Colour CREAM(0xFFE0E0E0);
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p),
          bypassComponent(p.apvts),
          distortionPanel("DISTORTION", false),
          modulationPanel("MODULATION", true),
          gainPanel("GAIN", false)
    {
        setLookAndFeel(&lookAndFeel);
        startTimer(1000 / kTimerHz);

        setOpaque(true);

        addAndMakeVisible(bypassComponent);

        addAndMakeVisible(distortionPanel);
        addAndMakeVisible(modulationPanel);
        addAndMakeVisible(gainPanel);

        // WARM DIST - Large Drive knob
        setupKnobWithLabel(driveKnob, driveTitleLabel, driveLabel, "DRIVE", "COLOR", driveAttachment);
        // BITCRUSH
        setupKnobWithLabel(bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment);

        // MODULATION - Delay row
        setupKnobWithLabel(delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "TIME", "DELAY_TIME", delayTimeAttachment);
        setupKnobWithLabel(delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "FEEDBACK", "DELAY_FEEDBACK", delayFeedbackAttachment);
        // MODULATION - Mix row
        setupKnobWithLabel(delayMixKnob, delayMixTitleLabel, delayMixLabel, "DELAY MIX", "DELAY_MIX", delayMixAttachment);
        setupKnobWithLabel(chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "CHORUS MIX", "CHORUS_MIX", chorusMixAttachment);

        // GAIN section
        setupKnobWithLabel(inputGainKnob, inputGainTitleLabel, inputGainLabel, "INPUT", "INPUT_GAIN", inputGainAttachment);
        setupKnobWithLabel(outputGainKnob, outputGainTitleLabel, outputGainLabel, "OUTPUT", "OUTPUT_GAIN", outputGainAttachment);

        setResizable(false, false);
        setSize(kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
    {
        drawRackBackground(g);

        auto bounds = getLocalBounds();
        bounds.removeFromLeft(kRackEarWidth);
        bounds.removeFromRight(kRackEarWidth);
        bounds.removeFromTop(12);
        bounds.removeFromBottom(12);
    }

    void AudioPluginAudioProcessorEditor::drawRackBackground(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient bgGrad(
            lookAndFeel.findColour(AnalogLookAndFeel::backgroundColourId).brighter(0.05f), bounds.getTopLeft(),
            lookAndFeel.findColour(AnalogLookAndFeel::backgroundColourId).darker(0.1f), bounds.getBottomLeft(),
            false);
        g.setGradientFill(bgGrad);
        g.fillRect(bounds);

        auto localBounds = getLocalBounds();
        auto leftEar = localBounds.removeFromLeft(kRackEarWidth);
        auto rightEar = localBounds.removeFromRight(kRackEarWidth);

        g.setColour(juce::Colour(0xFF151515));
        g.fillRect(leftEar);
        g.fillRect(rightEar);

        g.setColour(juce::Colour(0xFF252525));
        int numLines = leftEar.getHeight() / 15;
        for (int i = 0; i < numLines; ++i)
        {
            int y = leftEar.getY() + 15 + i * 15;
            g.fillRect(leftEar.getX() + 3, y, leftEar.getWidth() - 6, 2);
            g.fillRect(rightEar.getX() + 3, y, rightEar.getWidth() - 6, 2);
        }

        g.setColour(juce::Colour(0xFF1F1F1F));
        g.drawRect(leftEar, 1);
        g.drawRect(rightEar, 1);

        auto innerBounds = localBounds;
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillRect(innerBounds.removeFromTop(kBypassBarHeight));
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(kRackEarWidth);
        bounds.removeFromRight(kRackEarWidth);
        bounds.removeFromTop(12);
        bounds.removeFromBottom(12);

        // Bypass bar at top
        auto bypassArea = bounds.removeFromTop(kBypassBarHeight);
        bypassComponent.setBounds(bypassArea);

        // Three Column Layout with golden ratio proportions
        const int gap = kSectionGap;
        const int colWidths[3] = { kColumnDistortion, kColumnModulation, kColumnGain };

        juce::Rectangle<int> columnBounds[3];
        int xOffset = 0;
        for (int i = 0; i < 3; ++i)
        {
            columnBounds[i] = juce::Rectangle<int>(
                bounds.getX() + xOffset, bounds.getY(),
                colWidths[i], bounds.getHeight());
            xOffset += colWidths[i] + gap;
        }

        distortionPanel.setBounds(columnBounds[0]);
        modulationPanel.setBounds(columnBounds[1]);
        gainPanel.setBounds(columnBounds[2]);

        auto placeDistortion = [&](juce::Rectangle<int> col)
        {
            auto content = col.reduced(6);
            auto driveArea = content.removeFromTop(
                static_cast<int>(content.getHeight() * 0.618f));
            auto crushArea = content;

            driveKnob.setBounds(
                driveArea.withSizeKeepingCentre(kKnobLarge, kKnobLarge));
            driveTitleLabel.setBounds(
                juce::Rectangle<int>(driveArea.getX(), driveArea.getY() + 2,
                                     driveArea.getWidth(), 16));
            driveLabel.setBounds(
                juce::Rectangle<int>(driveArea.getX(), driveKnob.getBottom() + 2,
                                     driveArea.getWidth(), 14));

            bitCrushKnob.setBounds(
                crushArea.withSizeKeepingCentre(kKnobSmall, kKnobSmall));
            bitCrushTitleLabel.setBounds(
                juce::Rectangle<int>(crushArea.getX(), crushArea.getY() + 4,
                                     crushArea.getWidth(), 16));
            bitCrushLabel.setBounds(
                juce::Rectangle<int>(crushArea.getX(), bitCrushKnob.getBottom() + 2,
                                     crushArea.getWidth(), 14));
        };

        auto placeModulation = [&](juce::Rectangle<int> col)
        {
            auto content = col.reduced(6);
            auto topRow = content.removeFromTop(content.getHeight() / 2);
            auto botRow = content;

            auto placeKnobInRow = [&](juce::Rectangle<int> row, juce::Slider& knob,
                                       juce::Label& title, juce::Label& val)
            {
                auto half = row.removeFromLeft(row.getWidth() / 2);
                knob.setBounds(half.withSizeKeepingCentre(kKnobSmall, kKnobSmall));
                title.setBounds(juce::Rectangle<int>(half.getX(), half.getY() + 4,
                                                     half.getWidth(), 16));
                val.setBounds(juce::Rectangle<int>(half.getX(), knob.getBottom() + 2,
                                                   half.getWidth(), 14));
                return row;
            };

            auto remainingTop = placeKnobInRow(topRow, delayTimeKnob, delayTimeTitleLabel, delayTimeLabel);
            placeKnobInRow(remainingTop, delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel);

            auto remainingBot = placeKnobInRow(botRow, delayMixKnob, delayMixTitleLabel, delayMixLabel);
            placeKnobInRow(remainingBot, chorusMixKnob, chorusMixTitleLabel, chorusMixLabel);
        };

        auto placeGain = [&](juce::Rectangle<int> col)
        {
            auto content = col.reduced(6);
            auto inputArea = content.removeFromTop(
                static_cast<int>(content.getHeight() * 0.618f));
            auto outputArea = content;

            inputGainKnob.setBounds(
                inputArea.withSizeKeepingCentre(kKnobLarge, kKnobLarge));
            inputGainTitleLabel.setBounds(
                juce::Rectangle<int>(inputArea.getX(), inputArea.getY() + 2,
                                     inputArea.getWidth(), 16));
            inputGainLabel.setBounds(
                juce::Rectangle<int>(inputArea.getX(), inputGainKnob.getBottom() + 2,
                                     inputArea.getWidth(), 14));

            outputGainKnob.setBounds(
                outputArea.withSizeKeepingCentre(kKnobLarge, kKnobLarge));
            outputGainTitleLabel.setBounds(
                juce::Rectangle<int>(outputArea.getX(), outputArea.getY() + 4,
                                     outputArea.getWidth(), 16));
            outputGainLabel.setBounds(
                juce::Rectangle<int>(outputArea.getX(), outputGainKnob.getBottom() + 2,
                                     outputArea.getWidth(), 14));
        };

        placeDistortion(columnBounds[0]);
        placeModulation(columnBounds[1]);
        placeGain(columnBounds[2]);
    }

    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        updateMainControlLabels();
    }

    void AudioPluginAudioProcessorEditor::setupKnob(juce::Slider& knob)
    {
        knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setRotaryParameters(kRotaryStartAngle, kRotaryEndAngle, true);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.setVelocityBasedMode(true);
        knob.setVelocityModeParameters(0.5f, 1, 0.1f, false);
        knob.setBufferedToImage(false);
    }

    void AudioPluginAudioProcessorEditor::setupKnobWithLabel(
        juce::Slider& knob,
        juce::Label& titleLabel,
        juce::Label& valueLabel,
        const juce::String& title,
        const juce::String& paramId,
        std::unique_ptr<SliderAttachment>& attachment)
    {
        setupKnob(knob);
        addAndMakeVisible(knob);
        attachment = std::make_unique<SliderAttachment>(processorRef.apvts, paramId, knob);

        titleLabel.setText(title, juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setFont(juce::Font(juce::FontOptions().withName("Montserrat Medium").withHeight(13.0f)));
        titleLabel.setColour(juce::Label::textColourId, CREAM);
        addAndMakeVisible(titleLabel);

        valueLabel.setText("", juce::dontSendNotification);
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setFont(juce::Font(juce::FontOptions().withName("Montserrat Regular").withHeight(12.0f)));
        valueLabel.setColour(juce::Label::textColourId, CREAM);
        addAndMakeVisible(valueLabel);
    }

    juce::String AudioPluginAudioProcessorEditor::formatPercentage(float normalizedValue)
    {
        int percentage = static_cast<int>(normalizedValue * 100.0f);
        return juce::String(percentage) + "%";
    }

    juce::String AudioPluginAudioProcessorEditor::formatDB(float dbValue)
    {
        if (dbValue > 0.0f)
            return "+" + juce::String(dbValue, 1) + " dB";
        else if (dbValue < 0.0f)
            return juce::String(dbValue, 1) + " dB";
        else
            return "0 dB";
    }

    juce::String AudioPluginAudioProcessorEditor::formatMilliseconds(float ms)
    {
        if (ms >= 1000.0f)
            return juce::String(ms / 1000.0f, 2) + " s";
        else
            return juce::String(static_cast<int>(ms)) + " ms";
    }

    void AudioPluginAudioProcessorEditor::updateMainControlLabels()
    {
        auto colorValue = processorRef.apvts.getRawParameterValue("COLOR")->load();
        driveLabel.setText(formatPercentage(colorValue), juce::dontSendNotification);

        auto bitCrushValue = processorRef.apvts.getRawParameterValue("BITCRUSH")->load();
        bitCrushLabel.setText(formatPercentage(bitCrushValue), juce::dontSendNotification);

        auto delayTimeValue = processorRef.apvts.getRawParameterValue("DELAY_TIME")->load();
        delayTimeLabel.setText(formatMilliseconds(delayTimeValue), juce::dontSendNotification);

        auto delayMixValue = processorRef.apvts.getRawParameterValue("DELAY_MIX")->load();
        delayMixLabel.setText(formatPercentage(delayMixValue), juce::dontSendNotification);

        auto delayFeedbackValue = processorRef.apvts.getRawParameterValue("DELAY_FEEDBACK")->load();
        delayFeedbackLabel.setText(formatPercentage(delayFeedbackValue), juce::dontSendNotification);

        auto chorusMixValue = processorRef.apvts.getRawParameterValue("CHORUS_MIX")->load();
        chorusMixLabel.setText(formatPercentage(chorusMixValue), juce::dontSendNotification);

        auto inputGainValue = processorRef.apvts.getRawParameterValue("INPUT_GAIN")->load();
        inputGainLabel.setText(formatDB(inputGainValue), juce::dontSendNotification);

        auto outputGainValue = processorRef.apvts.getRawParameterValue("OUTPUT_GAIN")->load();
        outputGainLabel.setText(formatDB(outputGainValue), juce::dontSendNotification);
    }
}
