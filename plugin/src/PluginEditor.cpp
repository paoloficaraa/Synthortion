#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p)
    {
        setLookAndFeel(&lookAndFeel);
        startTimer(1000 / kTimerHz);

        // Drive knob (Color)
        setupKnob(driveKnob);
        addAndMakeVisible(driveKnob);
        driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "COLOR", driveKnob);
        driveLabel.setText("", juce::dontSendNotification);
        driveLabel.setJustificationType(juce::Justification::centred);
        driveLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        driveLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(driveLabel);

        // BitCrush
        setupKnobWithLabel(bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment);

        // Delay Time
        setupKnobWithLabel(delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "DELAY TIME", "DELAY_TIME", delayTimeAttachment);

        // Delay Mix
        setupKnobWithLabel(delayMixKnob, delayMixTitleLabel, delayMixLabel, "DELAY MIX", "DELAY_MIX", delayMixAttachment);

        // Delay Feedback
        setupKnobWithLabel(delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "DELAY FB", "DELAY_FEEDBACK", delayFeedbackAttachment);

        // Chorus Mix
        setupKnobWithLabel(chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "CHORUS MIX", "CHORUS_MIX", chorusMixAttachment);

        // Preset selector (commented out)
        // presetSelector.addItemList({"User", "Clean Tape", "Lofi Chaos", "Ambient Wash", "Aggressive Crunch"}, 1);
        // addAndMakeVisible(presetSelector);
        // presetAttachment = std::make_unique<ComboBoxAttachment>(processorRef.apvts, "PRESET", presetSelector);
        // presetLabel.setText("PRESET", juce::dontSendNotification);
        // presetLabel.setJustificationType(juce::Justification::centred);
        // addAndMakeVisible(presetLabel);

        setResizable(false, false);
        setSize(kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
    {
        g.fillAll(BLACK);

        auto bounds = getLocalBounds().reduced(11).toFloat();

        // Main area
        auto mainArea = bounds;
        auto colorArea = mainArea.removeFromTop(160.f);
        auto effectsArea = mainArea;

        // Color panel
        lookAndFeel.drawSectionPanel(g, colorArea.reduced(4.f), 7.f);
        lookAndFeel.drawFrameLabel(g, colorArea.reduced(7.f).removeFromTop(14.f).withWidth(70.f), "COLOR");

        // Effects panel
        lookAndFeel.drawSectionPanel(g, effectsArea.reduced(4.f), 6.f);
        lookAndFeel.drawFrameLabel(g, effectsArea.reduced(7.f).removeFromTop(13.f).withWidth(70.f), "EFFECTS");
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds().reduced(11);

        auto mainArea = bounds;
        auto colorArea = mainArea.removeFromTop(160);
        auto effectsArea = mainArea;

        // COLOR KNOB
        {
            const int colorKnobSize = 70;
            const int colorLabelH = 12;
            const int labelGap = 2;

            auto colorCenterX = colorArea.getCentreX();
            auto colorCenterY = colorArea.getCentreY();

            auto colorKnobBounds = juce::Rectangle<int>(colorCenterX - colorKnobSize / 2, colorCenterY - colorKnobSize / 2, colorKnobSize, colorKnobSize);
            driveKnob.setBounds(colorKnobBounds);

            auto colorLabelBounds = juce::Rectangle<int>(colorCenterX - 40, colorKnobBounds.getBottom() + labelGap, 80, colorLabelH);
            driveLabel.setBounds(colorLabelBounds);
        }

        // EFFECTS CONTROLS
        {
            const int knobSize = 46;
            const int titleH = 10;
            const int valueH = 10;
            const int labelGap = 1;
            const int rowGap = 8;
            const int labelWidth = 70;
            const int topMargin = 22;

            auto effectsAreaLocal = effectsArea.reduced(10, 12);
            effectsAreaLocal.removeFromTop(topMargin);

            auto totalContentHeight = titleH + labelGap + knobSize + labelGap + valueH;
            auto availableHeight = effectsAreaLocal.getHeight();
            auto totalGaps = rowGap * 2;
            auto totalRowsHeight = totalContentHeight * 3;
            auto extraSpace = availableHeight - totalRowsHeight - totalGaps;
            auto rowHeight = totalContentHeight + (extraSpace / 3);

            auto row1 = effectsAreaLocal.removeFromTop(rowHeight);
            effectsAreaLocal.removeFromTop(rowGap);
            auto row2 = effectsAreaLocal.removeFromTop(rowHeight);
            effectsAreaLocal.removeFromTop(rowGap);
            auto row3 = effectsAreaLocal;

            auto placeKnob = [&](juce::Rectangle<int> area, juce::Slider &knob, juce::Label &titleLabel, juce::Label &valueLabel)
            {
                auto centerX = area.getCentreX();
                auto centerY = area.getCentreY();

                auto knobBounds = juce::Rectangle<int>(centerX - knobSize / 2, centerY - knobSize / 2, knobSize, knobSize);
                knob.setBounds(knobBounds);

                auto titleBounds = juce::Rectangle<int>(centerX - labelWidth / 2, knobBounds.getY() - titleH - labelGap - 1, labelWidth, titleH);
                titleLabel.setBounds(titleBounds);

                auto valueBounds = juce::Rectangle<int>(centerX - labelWidth / 2, knobBounds.getBottom() + labelGap + 1, labelWidth, valueH);
                valueLabel.setBounds(valueBounds);
            };

            // Row 1: BitCrush
            placeKnob(row1, bitCrushKnob, bitCrushTitleLabel, bitCrushLabel);

            // Row 2: Delay Time, Delay Feedback
            {
                int halfWidth = row2.getWidth() / 2;
                auto delayTimeArea = row2.removeFromLeft(halfWidth);
                auto delayFeedbackArea = row2;
                placeKnob(delayTimeArea, delayTimeKnob, delayTimeTitleLabel, delayTimeLabel);
                placeKnob(delayFeedbackArea, delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel);
            }

            // Row 3: Delay Mix, Chorus Mix
            {
                int halfWidth = row3.getWidth() / 2;
                auto delayMixArea = row3.removeFromLeft(halfWidth);
                auto chorusMixArea = row3;
                placeKnob(delayMixArea, delayMixKnob, delayMixTitleLabel, delayMixLabel);
                placeKnob(chorusMixArea, chorusMixKnob, chorusMixTitleLabel, chorusMixLabel);
            }
        }
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
        knob.setVelocityModeParameters(kVelocitySensitivity, kVelocityThreshold, kVelocityOffset, false);
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
        titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        titleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(titleLabel);

        valueLabel.setText("", juce::dontSendNotification);
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        valueLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
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
        delayFeedbackLabel.setText(juce::String(delayFeedbackValue * 100), juce::dontSendNotification);

        auto chorusMixValue = processorRef.apvts.getRawParameterValue("CHORUS_MIX")->load();
        chorusMixLabel.setText(formatPercentage(chorusMixValue), juce::dontSendNotification);
    }
}
