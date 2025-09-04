#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p),
          inputMeter([&]()
                     { return processorRef.getInputRmsLevel(); }),
          outputMeter([&]()
                      { return processorRef.getOutputRmsLevel(); })
    {
        setLookAndFeel(&lookAndFeel);

        // Drive Knob (large central knob)
        driveKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        driveKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f,
                                      true);
        driveKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        driveKnob.setVelocityBasedMode(true);
        addAndMakeVisible(driveKnob);
        driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "DRIVE", driveKnob);
        driveLabel.setText("DRIVE", juce::dontSendNotification);
        driveLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(driveLabel);

        // Input Gain Knob (left side)
        inputGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        inputGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                          juce::MathConstants<float>::pi * 2.75f,
                                          true);
        inputGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        inputGainKnob.setVelocityBasedMode(true);
        addAndMakeVisible(inputGainKnob);
        inputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "INPUT_GAIN", inputGainKnob);
        inputGainLabel.setText("INPUT GAIN", juce::dontSendNotification);
        inputGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(inputGainLabel);

        // Output Gain Knob (right side)
        outputGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        outputGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        outputGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        outputGainKnob.setVelocityBasedMode(true);
        addAndMakeVisible(outputGainKnob);
        outputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "OUTPUT_GAIN", outputGainKnob);
        outputGainLabel.setText("OUTPUT GAIN", juce::dontSendNotification);
        outputGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(outputGainLabel);

        // Mix/Dry-Wet Knob
        mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mixKnob.setVelocityBasedMode(true);
        addAndMakeVisible(mixKnob);
        mixAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "MIX", mixKnob);
        mixLabel.setText("DRY/WET", juce::dontSendNotification);
        mixLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(mixLabel);

        // Delay Knob
        delayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        delayKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f,
                                      true);
        delayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        delayKnob.setVelocityBasedMode(true);
        addAndMakeVisible(delayKnob);
        delayAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "DELAY", delayKnob);
        delayLabel.setText("DELAY", juce::dontSendNotification);
        delayLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(delayLabel);

        // Chorus Knob
        chorusKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        chorusKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                       juce::MathConstants<float>::pi * 2.75f,
                                       true);
        chorusKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        chorusKnob.setVelocityBasedMode(true);
        addAndMakeVisible(chorusKnob);
        chorusAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "CHORUS", chorusKnob);
        chorusLabel.setText("CHORUS", juce::dontSendNotification);
        chorusLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(chorusLabel);

        // Saturation Type Selector (bottom right)
        saturationTypeSelector.addItem("Smooth", 1);
        saturationTypeSelector.addItem("Tube", 2);
        saturationTypeSelector.addItem("Tape", 3);
        addAndMakeVisible(saturationTypeSelector);
        saturationTypeAttachment = std::make_unique<ComboBoxAttachment>(processorRef.apvts, "SATURATION_TYPE", saturationTypeSelector);
        saturationTypeLabel.setText("TYPE", juce::dontSendNotification);
        saturationTypeLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(saturationTypeLabel);

        // Add meters
        addAndMakeVisible(inputMeter);
        addAndMakeVisible(outputMeter);

        // EQ Controls
        setupEQControls();

        // Add spectrum analyzer
        addAndMakeVisible(spectrumAnalyzer);

        // Connect spectrum analyzer to audio processor
        processorRef.setSpectrumAnalyzerCallback([this](float sample)
                                                 { spectrumAnalyzer.pushNextSampleIntoFifo(sample); });

        setResizable(false, false);
        setSize(1024, 700);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
    {
        // Base background
        g.fillAll(BLACK);

        // Mirror the same layout used in resized() con le nuove proporzioni
        auto bounds = getLocalBounds().reduced(15).toFloat();

        // METRI AI LATI ESTREMI - sezioni più larghe
        auto inputMeterArea = bounds.removeFromLeft(80.f);   // Allargata
        auto outputMeterArea = bounds.removeFromRight(80.f); // Allargata

        auto leftSection = bounds.removeFromLeft(250.f); // Sezione sinistra
        auto rightSection = bounds;                      // Sezione destra per EQ e spectrum

        auto leftTop = leftSection.removeFromTop(320.f); // Area per drive + type (ingrandita)
        auto leftBottom = leftSection;                   // Area per effects (ridotta)

        auto spectrumSection = rightSection.removeFromTop(350.f); // Spectrum ingrandito
        auto eqSection = rightSection;                            // EQ prende tutto il resto

        // Panels per meter 
        lookAndFeel.drawSectionPanel(g, inputMeterArea.reduced(3.f), 6.f);
        lookAndFeel.drawFrameLabel(g, inputMeterArea.reduced(5.f).removeFromTop(18.f), "INPUT");

        lookAndFeel.drawSectionPanel(g, outputMeterArea.reduced(3.f), 6.f);
        lookAndFeel.drawFrameLabel(g, outputMeterArea.reduced(5.f).removeFromTop(18.f), "OUTPUT");

        // Drive + Type 
        lookAndFeel.drawSectionPanel(g, leftTop.reduced(6.f), 10.f);
        lookAndFeel.drawFrameLabel(g, leftTop.reduced(10.f).removeFromTop(20.f).withWidth(120.f), "COLOR");

        // Effects section 
        lookAndFeel.drawSectionPanel(g, leftBottom.reduced(6.f), 8.f);
        lookAndFeel.drawFrameLabel(g, leftBottom.reduced(10.f).removeFromTop(18.f).withWidth(100.f), "EFFECTS");

        // Spectrum section
        lookAndFeel.drawSectionPanel(g, spectrumSection.reduced(6.f), 10.f);
        lookAndFeel.drawFrameLabel(g, spectrumSection.reduced(10.f).removeFromTop(18.f).withWidth(150.f), "SPECTRUM");

        // EQ section
        lookAndFeel.drawSectionPanel(g, eqSection.reduced(6.f), 10.f);
        lookAndFeel.drawFrameLabel(g, eqSection.reduced(10.f).removeFromTop(18.f).withWidth(150.f), "EQ / FILTER");
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds().reduced(15);

        // METER
        auto inputMeterArea = bounds.removeFromLeft(80);   // Allargata da 50 a 80
        auto outputMeterArea = bounds.removeFromRight(80); // Allargata da 50 a 80

        // Layout principale: left (controls) e right (EQ+spectrum)
        auto leftSection = bounds.removeFromLeft(250); // Sezione sinistra per controlli principali
        auto rightSection = bounds;                    // Sezione destra per EQ e spectrum

        // LEFT SECTION - dividiamo in top (drive + type) e bottom (effects)
        auto leftTop = leftSection.removeFromTop(320); // Area per drive + type (ingrandita)
        auto leftBottom = leftSection;                 // Area per effects (ridotta)

        // RIGHT SECTION - spectrum e EQ più grandi per colmare lo spazio vuoto
        auto spectrumSection = rightSection.removeFromTop(350); 
        auto eqSection = rightSection;                         

        // INPUT METER + KNOB
        {
            const int meterH = 550, meterW = 20;
            const int knobSize = 60;
            const int topPadding = 25;
            const int bottomPadding = 15;
            const int meterKnobGap = 10; // Spazio tra meter e knob

            auto workingArea = inputMeterArea.reduced(5, topPadding);
            auto cx = inputMeterArea.getCentreX();

            // Knob in fondo alla sezione
            auto knobY = inputMeterArea.getBottom() - bottomPadding - knobSize;
            inputGainKnob.setBounds(juce::Rectangle<int>(0, knobY, knobSize, knobSize).withCentre({cx, knobY + knobSize / 2}));

            // Meter posizionato proprio sopra il knob
            auto meterBottomY = knobY - meterKnobGap;
            auto meterY = meterBottomY - meterH;
            auto inputMeterBounds = juce::Rectangle<int>(0, meterY, meterW, meterH).withCentre({cx, meterY + meterH / 2});
            inputMeter.setBounds(inputMeterBounds);

            // Nascondi la label (non più necessaria)
            inputGainLabel.setBounds(juce::Rectangle<int>(0, 0, 0, 0));
        }

        // OUTPUT METER + KNOB
        {
            const int meterH = 550, meterW = 20;
            const int knobSize = 60;
            const int topPadding = 25;
            const int bottomPadding = 15;
            const int meterKnobGap = 10; // Spazio tra meter e knob

            auto workingArea = outputMeterArea.reduced(5, topPadding);
            auto cx = outputMeterArea.getCentreX();

            // Knob in fondo alla sezione
            auto knobY = outputMeterArea.getBottom() - bottomPadding - knobSize;
            outputGainKnob.setBounds(juce::Rectangle<int>(0, knobY, knobSize, knobSize).withCentre({cx, knobY + knobSize / 2}));

            // Meter posizionato proprio sopra il knob
            auto meterBottomY = knobY - meterKnobGap;
            auto meterY = meterBottomY - meterH;
            auto outputMeterBounds = juce::Rectangle<int>(0, meterY, meterW, meterH).withCentre({cx, meterY + meterH / 2});
            outputMeter.setBounds(outputMeterBounds);

            // Nascondi la label (non più necessaria)
            outputGainLabel.setBounds(juce::Rectangle<int>(0, 0, 0, 0));
        }

        // DRIVE + TYPE AREA (nella stessa sezione COLOR, come nell'immagine)
        {
            // Drive knob principale
            auto driveKnobArea = leftTop.removeFromTop(220);
            auto driveLabelArea = driveKnobArea.removeFromBottom(25);
            driveKnob.setBounds(driveKnobArea.withSizeKeepingCentre(120, 120)); // Knob grande
            driveLabel.setBounds(driveLabelArea);

            // Type selector nella stessa sezione (sotto il drive, non sezione separata)
            auto typeArea = leftTop.removeFromTop(80); // Area per type selector
            auto typeLabelArea = typeArea.removeFromTop(25);
            saturationTypeLabel.setText("TYPE", juce::dontSendNotification);
            saturationTypeLabel.setBounds(typeLabelArea);
            saturationTypeSelector.setBounds(typeArea.reduced(20, 10));
        }

        // EFFECTS SECTION (bottom) - sezione più compatta con 3 knob
        {
            const int knobSize = 65; // Knob più piccoli per sezione compatta
            const int labelH = 20;
            const int knobsPerRow = 3;

            auto effectsArea = leftBottom.reduced(10);
            auto labelsArea = effectsArea.removeFromBottom(labelH);

            // Dividi l'area in 3 colonne per i 3 knob
            auto knobWidth = effectsArea.getWidth() / knobsPerRow;

            // Dry/Wet knob
            auto mixArea = effectsArea.removeFromLeft(knobWidth);
            mixKnob.setBounds(mixArea.withSizeKeepingCentre(knobSize, knobSize));
            auto mixLabelArea = labelsArea.removeFromLeft(knobWidth);
            mixLabel.setBounds(mixLabelArea);

            // Delay knob
            auto delayArea = effectsArea.removeFromLeft(knobWidth);
            delayKnob.setBounds(delayArea.withSizeKeepingCentre(knobSize, knobSize));
            auto delayLabelArea = labelsArea.removeFromLeft(knobWidth);
            delayLabel.setBounds(delayLabelArea);

            // Chorus knob
            auto chorusArea = effectsArea;
            chorusKnob.setBounds(chorusArea.withSizeKeepingCentre(knobSize, knobSize));
            auto chorusLabelArea = labelsArea;
            chorusLabel.setBounds(chorusLabelArea);
        }

        // SPECTRUM SECTION (right top) - più grande per colmare spazio vuoto
        spectrumAnalyzer.setBounds(spectrumSection.reduced(10));

        // EQ SECTION (right bottom) - più grande, prende tutto lo spazio rimanente
        auto eqBandWidth = eqSection.getWidth() / 4;

        // Low Cut (band 1)
        auto lowCutArea = eqSection.removeFromLeft(eqBandWidth).reduced(8);
        auto knobHeight = (lowCutArea.getHeight() - 30) / 2;
        lowCutFreqKnob.setBounds(lowCutArea.removeFromTop(knobHeight).withSizeKeepingCentre(50, 50));
        lowCutFreqLabel.setBounds(lowCutArea.removeFromTop(18));
        lowCutQKnob.setBounds(lowCutArea.removeFromTop(knobHeight).withSizeKeepingCentre(50, 50));
        lowCutQLabel.setBounds(lowCutArea.removeFromTop(18));

        // Low Mid (band 2)
        auto lowMidArea = eqSection.removeFromLeft(eqBandWidth).reduced(8);
        auto lowMidKnobHeight = (lowMidArea.getHeight() - 54) / 3;
        lowMidFreqKnob.setBounds(lowMidArea.removeFromTop(lowMidKnobHeight).withSizeKeepingCentre(48, 48));
        lowMidFreqLabel.setBounds(lowMidArea.removeFromTop(18));
        lowMidGainKnob.setBounds(lowMidArea.removeFromTop(lowMidKnobHeight).withSizeKeepingCentre(48, 48));
        lowMidGainLabel.setBounds(lowMidArea.removeFromTop(18));
        lowMidQKnob.setBounds(lowMidArea.removeFromTop(lowMidKnobHeight).withSizeKeepingCentre(48, 48));
        lowMidQLabel.setBounds(lowMidArea.removeFromTop(18));

        // High Mid (band 3)
        auto highMidArea = eqSection.removeFromLeft(eqBandWidth).reduced(8);
        auto highMidKnobHeight = (highMidArea.getHeight() - 54) / 3;
        highMidFreqKnob.setBounds(highMidArea.removeFromTop(highMidKnobHeight).withSizeKeepingCentre(48, 48));
        highMidFreqLabel.setBounds(highMidArea.removeFromTop(18));
        highMidGainKnob.setBounds(highMidArea.removeFromTop(highMidKnobHeight).withSizeKeepingCentre(48, 48));
        highMidGainLabel.setBounds(highMidArea.removeFromTop(18));
        highMidQKnob.setBounds(highMidArea.removeFromTop(highMidKnobHeight).withSizeKeepingCentre(48, 48));
        highMidQLabel.setBounds(highMidArea.removeFromTop(18));

        // High Cut (band 4)
        auto highCutArea = eqSection.reduced(8);
        auto highCutKnobHeight = (highCutArea.getHeight() - 36) / 2;
        highCutFreqKnob.setBounds(highCutArea.removeFromTop(highCutKnobHeight).withSizeKeepingCentre(50, 50));
        highCutFreqLabel.setBounds(highCutArea.removeFromTop(18));
        highCutQKnob.setBounds(highCutArea.removeFromTop(highCutKnobHeight).withSizeKeepingCentre(50, 50));
        highCutQLabel.setBounds(highCutArea.removeFromTop(18));
    }
    void AudioPluginAudioProcessorEditor::setupEQControls()
    {
        // Low Cut Frequency & Q
        lowCutFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowCutFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        lowCutFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowCutFreqKnob.setVelocityBasedMode(true);
        addAndMakeVisible(lowCutFreqKnob);
        lowCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_FREQ", lowCutFreqKnob);
        lowCutFreqLabel.setText("", juce::dontSendNotification);
        lowCutFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowCutFreqLabel);

        lowCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowCutQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
        lowCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowCutQKnob.setVelocityBasedMode(true);
        addAndMakeVisible(lowCutQKnob);
        lowCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_Q", lowCutQKnob);
        lowCutQLabel.setText("Q", juce::dontSendNotification);
        lowCutQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowCutQLabel);

        // Low Mid Frequency, Gain & Q
        lowMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        lowMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidFreqKnob.setVelocityBasedMode(true);
        addAndMakeVisible(lowMidFreqKnob);
        lowMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_FREQ", lowMidFreqKnob);
        lowMidFreqLabel.setText("", juce::dontSendNotification);
        lowMidFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidFreqLabel);

        lowMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        lowMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidGainKnob.setVelocityBasedMode(true);
        addAndMakeVisible(lowMidGainKnob);
        lowMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_GAIN", lowMidGainKnob);
        lowMidGainLabel.setText("Gain", juce::dontSendNotification);
        lowMidGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidGainLabel);

        lowMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
        lowMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidQKnob.setVelocityBasedMode(true);
        addAndMakeVisible(lowMidQKnob);
        lowMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_Q", lowMidQKnob);
        lowMidQLabel.setText("Q", juce::dontSendNotification);
        lowMidQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(lowMidQLabel);

        // High Mid Frequency, Gain & Q
        highMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidFreqKnob.setVelocityBasedMode(true);
        addAndMakeVisible(highMidFreqKnob);
        highMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_FREQ", highMidFreqKnob);
        highMidFreqLabel.setText("", juce::dontSendNotification);
        highMidFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidFreqLabel);

        highMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidGainKnob.setVelocityBasedMode(true);
        addAndMakeVisible(highMidGainKnob);
        highMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_GAIN", highMidGainKnob);
        highMidGainLabel.setText("Gain", juce::dontSendNotification);
        highMidGainLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidGainLabel);

        highMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                         juce::MathConstants<float>::pi * 2.75f,
                                         true);
        highMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidQKnob.setVelocityBasedMode(true);
        addAndMakeVisible(highMidQKnob);
        highMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_Q", highMidQKnob);
        highMidQLabel.setText("Q", juce::dontSendNotification);
        highMidQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highMidQLabel);

        // High Cut Frequency & Q
        highCutFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highCutFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highCutFreqKnob.setVelocityBasedMode(true);
        addAndMakeVisible(highCutFreqKnob);
        highCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_FREQ", highCutFreqKnob);
        highCutFreqLabel.setText("", juce::dontSendNotification);
        highCutFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highCutFreqLabel);

        highCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                         juce::MathConstants<float>::pi * 2.75f,
                                         true);
        highCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highCutQKnob.setVelocityBasedMode(true);
        addAndMakeVisible(highCutQKnob);
        highCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_Q", highCutQKnob);
        highCutQLabel.setText("Q", juce::dontSendNotification);
        highCutQLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(highCutQLabel);
    }
}
