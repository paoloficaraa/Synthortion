#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"

namespace synthortion
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

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

        // EQ Section Titles
        lowCutTitle.setText("LOW CUT", juce::dontSendNotification);
        lowCutTitle.setJustificationType(juce::Justification::centred);
        lowCutTitle.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
        lowCutTitle.setColour(juce::Label::textColourId, juce::Colour(0xffB4B4B4));
        addAndMakeVisible(lowCutTitle);

        lowMidTitle.setText("LOW MID", juce::dontSendNotification);
        lowMidTitle.setJustificationType(juce::Justification::centred);
        lowMidTitle.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
        lowMidTitle.setColour(juce::Label::textColourId, juce::Colour(0xffB4B4B4));
        addAndMakeVisible(lowMidTitle);

        highMidTitle.setText("HIGH MID", juce::dontSendNotification);
        highMidTitle.setJustificationType(juce::Justification::centred);
        highMidTitle.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
        highMidTitle.setColour(juce::Label::textColourId, juce::Colour(0xffB4B4B4));
        addAndMakeVisible(highMidTitle);

        highCutTitle.setText("HIGH CUT", juce::dontSendNotification);
        highCutTitle.setJustificationType(juce::Justification::centred);
        highCutTitle.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
        highCutTitle.setColour(juce::Label::textColourId, juce::Colour(0xffB4B4B4));
        addAndMakeVisible(highCutTitle);

        // Add spectrum analyzer
        addAndMakeVisible(spectrumAnalyzer);
        spectrumAnalyzer.setSampleRate(processorRef.getSampleRate());

        // Connect spectrum analyzer to audio processor
        // IMPORTANT: This creates a callback that captures 'this'.
        // The callback MUST be cleared in the destructor to prevent crashes.
        processorRef.setSpectrumAnalyzerCallback([this](float sample)
                                                 { spectrumAnalyzer.pushNextSampleIntoFifo(sample); });

        setResizable(false, false);
        setSize(1024, 700);

        startTimerHz(60);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        // CRITICAL: Clear the spectrum analyzer callback to prevent crashes
        // when the editor is destroyed while audio is still processing
        processorRef.setSpectrumAnalyzerCallback(nullptr);

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

        // EQ section
        lookAndFeel.drawSectionPanel(g, eqSection.reduced(6.f), 10.f);
        lookAndFeel.drawFrameLabel(g, eqSection.reduced(10.f).removeFromTop(18.f).withWidth(150.f), "EQ / FILTER");
    }

    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        // I meter si aggiornano automaticamente tramite il loro timer interno
        // Non è necessario chiamare update() manualmente
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

        // EQ SECTION (right bottom) - layout aggiornato: freq knobs più grandi e distanziati,
        // riempiono meglio l'area; Gain/Q con etichette più basse per migliore leggibilità.
        {
            auto area = eqSection.reduced(10);

            const int sectionHeaderOffset = 28;
            const int bandCount = 4;

            // Dimensioni maggiorate
            const int freqKnobSize = juce::jmin(80, area.getWidth() / 9);
            const int smallKnobSize = 48;
            const int titleHeight = 18;
            const int labelHeight = 16;
            const int verticalGap = 30;
            const int smallKnobGap = 25;
            const int labelYOffset = 12;
            const int sidePadding = 60;
            const int bottomPadding = 10;

            area.removeFromTop(sectionHeaderOffset);
            area.removeFromBottom(bottomPadding);

            // Centri orizzontali equidistanti
            int usableWidth = area.getWidth() - sidePadding * 2;
            double step = (bandCount > 1) ? (double)usableWidth / (bandCount - 1) : 0.0;

            // Asse Y principale per i freq knobs - centrato verticalmente nell'area disponibile
            int totalVerticalSpace = area.getHeight() - titleHeight - 4;
            int knobsAreaHeight = freqKnobSize + verticalGap + smallKnobSize + labelYOffset + labelHeight;
            int verticalCenterOffset = (totalVerticalSpace - knobsAreaHeight) / 2;

            int centerYFreqRow = area.getY() + titleHeight + 4 + verticalCenterOffset + freqKnobSize / 2;
            int secondRowY = centerYFreqRow + freqKnobSize / 2 + verticalGap + smallKnobSize / 2;

            auto placeBand = [&](int bandIndex,
                                 juce::Label &title,
                                 juce::Slider &freqKnob, juce::Label &freqLabel,
                                 juce::Slider *gainKnob, juce::Label *gainLabel,
                                 juce::Slider *qKnob, juce::Label *qLabel)
            {
                int centerX = area.getX() + sidePadding + (int)std::round(step * bandIndex);

                // Titolo
                title.setBounds(centerX - 60, area.getY(), 120, titleHeight);

                // Freq knob
                juce::Rectangle<int> freqBounds(centerX - freqKnobSize / 2, centerYFreqRow - freqKnobSize / 2,
                                                freqKnobSize, freqKnobSize);
                freqKnob.setBounds(freqBounds);
                // Label freq (vuota) posizionata ma più in basso nel caso futuro di display
                freqLabel.setBounds(freqBounds.withY(freqBounds.getBottom() + labelYOffset).withHeight(labelHeight));

                // Gain/Q riga sotto
                if (gainKnob || qKnob)
                {
                    int pairTotalWidth = (gainKnob && qKnob) ? (smallKnobSize * 2 + smallKnobGap) : smallKnobSize;
                    int startX = centerX - pairTotalWidth / 2;
                    int currentX = startX;

                    auto placeSmall = [&](juce::Slider *knob, juce::Label *lab)
                    {
                        if (!knob || !lab)
                            return;
                        juce::Rectangle<int> k(currentX, secondRowY - smallKnobSize / 2, smallKnobSize, smallKnobSize);
                        knob->setBounds(k);
                        lab->setBounds(k.withY(k.getBottom() + labelYOffset).withHeight(labelHeight));
                        currentX += smallKnobSize + smallKnobGap;
                    };

                    if (gainKnob && gainLabel)
                        placeSmall(gainKnob, gainLabel);
                    if (qKnob && qLabel)
                        placeSmall(qKnob, qLabel);
                }
                else if (qKnob && qLabel)
                {
                    // Solo Q
                    juce::Rectangle<int> k(centerX - smallKnobSize / 2, secondRowY - smallKnobSize / 2, smallKnobSize, smallKnobSize);
                    qKnob->setBounds(k);
                    qLabel->setBounds(k.withY(k.getBottom() + labelYOffset).withHeight(labelHeight));
                }
            };

            // Banda 0: Low Cut (freq + Q sotto)
            placeBand(0,
                      lowCutTitle,
                      lowCutFreqKnob, lowCutFreqLabel,
                      nullptr, nullptr,
                      &lowCutQKnob, &lowCutQLabel);

            // Banda 1: Low Mid (freq + Gain + Q)
            placeBand(1,
                      lowMidTitle,
                      lowMidFreqKnob, lowMidFreqLabel,
                      &lowMidGainKnob, &lowMidGainLabel,
                      &lowMidQKnob, &lowMidQLabel);

            // Banda 2: High Mid (freq + Gain + Q)
            placeBand(2,
                      highMidTitle,
                      highMidFreqKnob, highMidFreqLabel,
                      &highMidGainKnob, &highMidGainLabel,
                      &highMidQKnob, &highMidQLabel);

            // Banda 3: High Cut (freq + Q)
            placeBand(3,
                      highCutTitle,
                      highCutFreqKnob, highCutFreqLabel,
                      nullptr, nullptr,
                      &highCutQKnob, &highCutQLabel);

            // Linear Phase button - centrally positioned below all EQ controls
            const int buttonWidth = 120;
            const int buttonHeight = 30;
            const int buttonBottomMargin = -1;
            auto buttonX = area.getCentreX() - buttonWidth / 2;
            auto buttonY = area.getBottom() - buttonBottomMargin - buttonHeight;
            linearPhaseButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
        }
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

        // Linear Phase Toggle Button
        linearPhaseButton.setButtonText("LINEAR PHASE");
        linearPhaseButton.setClickingTogglesState(true); // Enable toggle behavior
        linearPhaseButton.setColour(juce::ToggleButton::textColourId, LIGHT_GREY);
        linearPhaseButton.setColour(juce::ToggleButton::tickColourId, PURPLE);
        linearPhaseButton.setColour(juce::ToggleButton::tickDisabledColourId, MID_GREY);
        addAndMakeVisible(linearPhaseButton);
        linearPhaseAttachment = std::make_unique<ButtonAttachment>(processorRef.apvts, "LINEAR_PHASE", linearPhaseButton);
    }
}
