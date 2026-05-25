#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    namespace Colours
    {
        const juce::Colour ANTHRACITE(0xFF130D1A);
        const juce::Colour CREAM(0xFFE0E0E0);
        const juce::Colour COPPER(0xFF9B5DE5);
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p)
    {
        setLookAndFeel(&lookAndFeel);
        startTimer(1000 / kTimerHz);

        // WARM DIST - Large Drive knob
        setupKnobWithLabel(driveKnob, driveTitleLabel, driveLabel, "DRIVE", "COLOR", driveAttachment);

        // EFFECTS - BitCrush row
        setupKnobWithLabel(bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment);
        setupKnobWithLabel(chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "CHORUS MIX", "CHORUS_MIX", chorusMixAttachment);

        // EFFECTS - Delay row
        setupKnobWithLabel(delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "TIME", "DELAY_TIME", delayTimeAttachment);
        setupKnobWithLabel(delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "FEEDBACK", "DELAY_FEEDBACK", delayFeedbackAttachment);
        setupKnobWithLabel(delayMixKnob, delayMixTitleLabel, delayMixLabel, "DELAY MIX", "DELAY_MIX", delayMixAttachment);

        // GAIN section
        setupKnobWithLabel(inputGainKnob, inputGainTitleLabel, inputGainLabel, "INPUT", "INPUT_GAIN", inputGainAttachment);
        setupKnobWithLabel(outputGainKnob, outputGainTitleLabel, outputGainLabel, "OUTPUT", "OUTPUT_GAIN", outputGainAttachment);

        // Global Bypass
        pluginBypassButton.setColour(juce::ToggleButton::textColourId, Colours::CREAM);
        pluginBypassButton.setColour(juce::ToggleButton::tickColourId, Colours::COPPER);
        addAndMakeVisible(pluginBypassButton);
        bypassAttachment = std::make_unique<ButtonAttachment>(processorRef.apvts, "PLUGIN_BYPASS", pluginBypassButton);

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

        if (warmDistBounds.isEmpty())
            return;

        lookAndFeel.drawPanelBackground(g, warmDistBounds, false, "DISTORTION");

        bool bypassOn = pluginBypassButton.getToggleState();
        auto ledBounds = juce::Rectangle<int>(pluginBypassButton.getX() - 14, pluginBypassButton.getY() + 4, 8, 8);
        
        g.setColour(bypassOn ? juce::Colour(0xFF00F5D4) : juce::Colour(0xFF1A1A1A));
        g.fillEllipse(ledBounds.toFloat());
        if (bypassOn)
        {
            g.setColour(juce::Colour(0xFF00F5D4).withAlpha(0.3f));
            g.fillEllipse(ledBounds.expanded(2).toFloat());
        }

        lookAndFeel.drawPanelBackground(g, effectsBounds, true, "MODULATION");
        lookAndFeel.drawPanelBackground(g, gainBounds, false, "GAIN");

        // Corner screws on rack ears
        g.setColour(juce::Colour(0xFF404040));
        auto drawScrew = [&](int x, int y) {
            g.fillEllipse(static_cast<float>(x), static_cast<float>(y), 6.0f, 6.0f);
            g.setColour(juce::Colour(0xFF202020));
            g.drawLine(x + 1.0f, y + 3.0f, x + 5.0f, y + 3.0f, 1.0f);
            g.setColour(juce::Colour(0xFF404040));
        };
        drawScrew(4, 14);
        drawScrew(getWidth() - 11, 14);
        drawScrew(4, getHeight() - 20);
        drawScrew(getWidth() - 11, getHeight() - 20);
    }

    void AudioPluginAudioProcessorEditor::drawRackBackground(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient bgGrad(
            Colours::ANTHRACITE.brighter(0.05f), bounds.getTopLeft(),
            Colours::ANTHRACITE.darker(0.1f), bounds.getBottomLeft(),
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
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(kRackEarWidth);
        bounds.removeFromRight(kRackEarWidth);
        bounds.removeFromTop(12);
        bounds.removeFromBottom(12);

        // Three Column Layout
        const int col1Width = 190;
        const int col2Width = 230;
        
        warmDistBounds = bounds.removeFromLeft(col1Width);
        bounds.removeFromLeft(kSectionGap);
        
        effectsBounds = bounds.removeFromLeft(col2Width);
        bounds.removeFromLeft(kSectionGap);
        
        gainBounds = bounds;

        // --- DISTORTION SECTION ---
        auto distContent = warmDistBounds.reduced(8);
        warmDistTitleBounds = distContent.removeFromTop(16);

        auto distLeft = distContent.removeFromLeft(distContent.getWidth() / 2);

        // Bypass button at top right of the section
        auto bypassBtnBounds = juce::Rectangle<int>(
            warmDistBounds.getRight() - 75,
            warmDistBounds.getY() + 14,
            65, 18);
        pluginBypassButton.setBounds(bypassBtnBounds);
        warmDistTitleBounds.removeFromRight(80);

        const int largeKnobSize = 75;
        const int smallKnobSize = 55;
        const int labelH = 10;

        // Manually place Drive and BitCrush
        auto driveKBounds = juce::Rectangle<int>(distLeft.getCentreX() - largeKnobSize / 2, distLeft.getCentreY() - largeKnobSize / 2 - 5, largeKnobSize, largeKnobSize);
        driveKnob.setBounds(driveKBounds);
        driveTitleLabel.setBounds(juce::Rectangle<int>(driveKBounds.getX() - 10, driveKBounds.getY() - labelH - 4, largeKnobSize + 20, labelH));
        driveLabel.setBounds(juce::Rectangle<int>(driveKBounds.getX() - 10, driveKBounds.getBottom() + 4, largeKnobSize + 20, labelH));
        // Note: The "Drive" knob intentionally has no title above it, it's implied by the main section or size.

        auto crushKBounds = juce::Rectangle<int>(distContent.getCentreX() - smallKnobSize / 2, distContent.getCentreY() - smallKnobSize / 2 - 5, smallKnobSize, smallKnobSize);
        bitCrushKnob.setBounds(crushKBounds);
        bitCrushTitleLabel.setBounds(juce::Rectangle<int>(crushKBounds.getX() - 20, crushKBounds.getY() - labelH - 4, smallKnobSize + 40, labelH));
        bitCrushLabel.setBounds(juce::Rectangle<int>(crushKBounds.getX() - 20, crushKBounds.getBottom() + 4, smallKnobSize + 40, labelH));

        // --- EFFECTS SECTION (Modulation/Space) ---
        auto modContent = effectsBounds.reduced(8);
        modContent.removeFromTop(16); // Title space
        
        auto modTopRow = modContent.removeFromTop(modContent.getHeight() / 2);
        auto modBotRow = modContent;
        
        auto placeKnobInCell = [&](juce::Rectangle<int> cell, juce::Slider& knob, juce::Label& title, juce::Label& val)
        {
            auto kb = juce::Rectangle<int>(cell.getCentreX() - smallKnobSize / 2, cell.getCentreY() - smallKnobSize / 2, smallKnobSize, smallKnobSize);
            knob.setBounds(kb);
            title.setBounds(juce::Rectangle<int>(kb.getX() - 20, kb.getY() - labelH - 2, smallKnobSize + 40, labelH));
            val.setBounds(juce::Rectangle<int>(kb.getX() - 20, kb.getBottom() + 2, smallKnobSize + 40, labelH));
        };

        placeKnobInCell(modTopRow.removeFromLeft(modTopRow.getWidth() / 2), delayTimeKnob, delayTimeTitleLabel, delayTimeLabel);
        placeKnobInCell(modTopRow, delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel);
        
        placeKnobInCell(modBotRow.removeFromLeft(modBotRow.getWidth() / 2), delayMixKnob, delayMixTitleLabel, delayMixLabel);
        placeKnobInCell(modBotRow, chorusMixKnob, chorusMixTitleLabel, chorusMixLabel);

        // --- GAIN SECTION ---
        auto gainContent = gainBounds.reduced(8);
        gainContent.removeFromTop(16); // Title space
        
        placeKnobInCell(gainContent.removeFromLeft(gainContent.getWidth() / 2), inputGainKnob, inputGainTitleLabel, inputGainLabel);
        placeKnobInCell(gainContent, outputGainKnob, outputGainTitleLabel, outputGainLabel);
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
        titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f).withStyle("Bold")));
        titleLabel.setColour(juce::Label::textColourId, Colours::CREAM);
        addAndMakeVisible(titleLabel);

        valueLabel.setText("", juce::dontSendNotification);
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        valueLabel.setColour(juce::Label::textColourId, Colours::CREAM);
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