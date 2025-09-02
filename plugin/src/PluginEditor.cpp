#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p)
    {
        setLookAndFeel(&lookAndFeel);

        // Drive Knob
        driveKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        driveKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(driveKnob);
        driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "DRIVE", driveKnob);
        driveLabel.setText("Drive", juce::dontSendNotification);
        driveLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(driveLabel);

        // Mix Knob
        mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(mixKnob);
        mixAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "MIX", mixKnob);
        mixLabel.setText("Mix", juce::dontSendNotification);
        mixLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(mixLabel);

        // Output Gain Knob
        outputGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        outputGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(outputGainKnob);
        outputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "OUTPUT_GAIN", outputGainKnob);
        outputGainLabel.setText("Output", juce::dontSendNotification);
        outputGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(outputGainLabel);

        // Saturation Type Selector
        saturationTypeSelector.addItem("Smooth", 1);
        saturationTypeSelector.addItem("Tube", 2);
        saturationTypeSelector.addItem("Tape", 3);
        addAndMakeVisible(saturationTypeSelector);
        saturationTypeAttachment = std::make_unique<ComboBoxAttachment>(processorRef.apvts, "SATURATION_TYPE", saturationTypeSelector);
        saturationTypeLabel.setText("Saturation Type", juce::dontSendNotification);
        saturationTypeLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(saturationTypeLabel);

        // EQ Controls
        setupEQControls();

        // Add spectrum analyzer
        addAndMakeVisible(spectrumAnalyzer);

        // Connect spectrum analyzer to audio processor
        processorRef.setSpectrumAnalyzerCallback([this](float sample)
                                                 { spectrumAnalyzer.pushNextSampleIntoFifo(sample); });

        setResizable(true, true);
        setSize(1024, 700);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
    {
        // Dark gradient background matching the UI theme
        juce::ColourGradient gradient(juce::Colour(0xff0f1419), 0, 0,
                                      juce::Colour(0xff1a2332), 0, (float)getHeight(), false);
        g.setGradientFill(gradient);
        g.fillAll();

        // Add some subtle texture/noise
        g.setColour(juce::Colour(0xff2a3441).withAlpha(0.1f));
        for (int i = 0; i < getWidth(); i += 4)
        {
            for (int j = 0; j < getHeight(); j += 4)
            {
                if (juce::Random().nextFloat() > 0.7f)
                    g.fillRect(i, j, 1, 1);
            }
        }
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds().reduced(15);

        // Layout: Drive in alto, Spectrum al centro, EQ sotto, Mix/Output/Type in basso
        auto topSection = bounds.removeFromTop(120);      // Drive knob
        auto spectrumSection = bounds.removeFromTop(140); // Spectrum analyzer
        auto eqSection = bounds.removeFromTop(160);       // EQ section with knobs
        auto bottomSection = bounds;                      // Output/Mix/Type

        // Top section: Large central drive knob
        auto driveLabelArea = topSection.removeFromBottom(25);
        driveKnob.setBounds(topSection.reduced(20));
        driveLabel.setBounds(driveLabelArea);

        // Spectrum analyzer (center area)
        spectrumAnalyzer.setBounds(spectrumSection.reduced(10));

        // EQ Section: 4 bands in a row (Low Cut, Low Mid, High Mid, High Cut)
        auto eqBandWidth = eqSection.getWidth() / 4;

        // Low Cut (Freq + Q)
        auto lowCutArea = eqSection.removeFromLeft(eqBandWidth).reduced(5);
        auto lowCutTopRow = lowCutArea.removeFromTop(lowCutArea.getHeight() / 2);
        lowCutFreqKnob.setBounds(lowCutTopRow.removeFromBottom(60).reduced(10));
        lowCutFreqLabel.setBounds(lowCutTopRow.removeFromBottom(20));
        lowCutQKnob.setBounds(lowCutArea.removeFromBottom(60).reduced(10));
        lowCutQLabel.setBounds(lowCutArea.removeFromBottom(20));

        // Low Mid (Freq + Gain + Q in vertical stack)
        auto lowMidArea = eqSection.removeFromLeft(eqBandWidth).reduced(5);
        auto lowMidRowHeight = lowMidArea.getHeight() / 3;
        auto lowMidFreqArea = lowMidArea.removeFromTop(lowMidRowHeight);
        auto lowMidGainArea = lowMidArea.removeFromTop(lowMidRowHeight);
        auto lowMidQArea = lowMidArea;

        lowMidFreqKnob.setBounds(lowMidFreqArea.removeFromBottom(45).reduced(8));
        lowMidFreqLabel.setBounds(lowMidFreqArea.removeFromBottom(15));
        lowMidGainKnob.setBounds(lowMidGainArea.removeFromBottom(45).reduced(8));
        lowMidGainLabel.setBounds(lowMidGainArea.removeFromBottom(15));
        lowMidQKnob.setBounds(lowMidQArea.removeFromBottom(45).reduced(8));
        lowMidQLabel.setBounds(lowMidQArea.removeFromBottom(15));

        // High Mid (Freq + Gain + Q in vertical stack)
        auto highMidArea = eqSection.removeFromLeft(eqBandWidth).reduced(5);
        auto highMidRowHeight = highMidArea.getHeight() / 3;
        auto highMidFreqArea = highMidArea.removeFromTop(highMidRowHeight);
        auto highMidGainArea = highMidArea.removeFromTop(highMidRowHeight);
        auto highMidQArea = highMidArea;

        highMidFreqKnob.setBounds(highMidFreqArea.removeFromBottom(45).reduced(8));
        highMidFreqLabel.setBounds(highMidFreqArea.removeFromBottom(15));
        highMidGainKnob.setBounds(highMidGainArea.removeFromBottom(45).reduced(8));
        highMidGainLabel.setBounds(highMidGainArea.removeFromBottom(15));
        highMidQKnob.setBounds(highMidQArea.removeFromBottom(45).reduced(8));
        highMidQLabel.setBounds(highMidQArea.removeFromBottom(15));

        // High Cut (Freq + Q)
        auto highCutArea = eqSection.reduced(5);
        auto highCutTopRow = highCutArea.removeFromTop(highCutArea.getHeight() / 2);
        highCutFreqKnob.setBounds(highCutTopRow.removeFromBottom(60).reduced(10));
        highCutFreqLabel.setBounds(highCutTopRow.removeFromBottom(20));
        highCutQKnob.setBounds(highCutArea.removeFromBottom(60).reduced(10));
        highCutQLabel.setBounds(highCutArea.removeFromBottom(20));

        // Bottom section: Output, Mix, Type selector in a row
        auto bottomControlWidth = bottomSection.getWidth() / 3;

        // Output Gain
        auto outputArea = bottomSection.removeFromLeft(bottomControlWidth).reduced(10);
        outputGainKnob.setBounds(outputArea.removeFromBottom(80).reduced(15));
        outputGainLabel.setBounds(outputArea.removeFromBottom(20));

        // Mix
        auto mixArea = bottomSection.removeFromLeft(bottomControlWidth).reduced(10);
        mixKnob.setBounds(mixArea.removeFromBottom(80).reduced(15));
        mixLabel.setBounds(mixArea.removeFromBottom(20));

        // Type selector
        auto typeArea = bottomSection.reduced(10);
        saturationTypeLabel.setBounds(typeArea.removeFromTop(20));
        saturationTypeSelector.setBounds(typeArea.removeFromTop(35).reduced(10, 5));
    }

    void AudioPluginAudioProcessorEditor::setupEQControls()
    {
        // Low Cut Frequency & Q
        lowCutFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowCutFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(lowCutFreqKnob);
        lowCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_FREQ", lowCutFreqKnob);
        lowCutFreqLabel.setText("Low Cut Freq", juce::dontSendNotification);
        lowCutFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowCutFreqLabel);

        lowCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(lowCutQKnob);
        lowCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_Q", lowCutQKnob);
        lowCutQLabel.setText("Low Cut Q", juce::dontSendNotification);
        lowCutQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowCutQLabel);

        // Low Mid Frequency, Gain & Q
        lowMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(lowMidFreqKnob);
        lowMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_FREQ", lowMidFreqKnob);
        lowMidFreqLabel.setText("Low Mid Freq", juce::dontSendNotification);
        lowMidFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidFreqLabel);

        lowMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(lowMidGainKnob);
        lowMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_GAIN", lowMidGainKnob);
        lowMidGainLabel.setText("Low Mid Gain", juce::dontSendNotification);
        lowMidGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidGainLabel);

        lowMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(lowMidQKnob);
        lowMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_Q", lowMidQKnob);
        lowMidQLabel.setText("Low Mid Q", juce::dontSendNotification);
        lowMidQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidQLabel);

        // High Mid Frequency, Gain & Q
        highMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(highMidFreqKnob);
        highMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_FREQ", highMidFreqKnob);
        highMidFreqLabel.setText("High Mid Freq", juce::dontSendNotification);
        highMidFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidFreqLabel);

        highMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(highMidGainKnob);
        highMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_GAIN", highMidGainKnob);
        highMidGainLabel.setText("High Mid Gain", juce::dontSendNotification);
        highMidGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidGainLabel);

        highMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(highMidQKnob);
        highMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_Q", highMidQKnob);
        highMidQLabel.setText("High Mid Q", juce::dontSendNotification);
        highMidQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidQLabel);

        // High Cut Frequency & Q
        highCutFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(highCutFreqKnob);
        highCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_FREQ", highCutFreqKnob);
        highCutFreqLabel.setText("High Cut Freq", juce::dontSendNotification);
        highCutFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highCutFreqLabel);

        highCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(highCutQKnob);
        highCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_Q", highCutQKnob);
        highCutQLabel.setText("High Cut Q", juce::dontSendNotification);
        highCutQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highCutQLabel);
    }
}
