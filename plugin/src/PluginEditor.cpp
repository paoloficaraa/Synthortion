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
        driveKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(driveKnob);
        driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "DRIVE", driveKnob);
        driveLabel.setText("COLOR", juce::dontSendNotification);
        driveLabel.setJustificationType(juce::Justification::centred);
        driveLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        driveLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(driveLabel);

        // Input Gain Knob (left side)
        inputGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        inputGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                          juce::MathConstants<float>::pi * 2.75f,
                                          true);
        inputGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        inputGainKnob.setVelocityBasedMode(true);
        inputGainKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(inputGainKnob);
        inputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "INPUT_GAIN", inputGainKnob);

        // Output Gain Knob (right side)
        outputGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        outputGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        outputGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        outputGainKnob.setVelocityBasedMode(true);
        outputGainKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(outputGainKnob);
        outputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "OUTPUT_GAIN", outputGainKnob);

        // Mix/Dry-Wet Knob
        mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mixKnob.setVelocityBasedMode(true);
        mixKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(mixKnob);
        mixAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "MIX", mixKnob);
        mixLabel.setText("DRY/WET", juce::dontSendNotification);
        mixLabel.setJustificationType(juce::Justification::centred);
        mixLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        mixLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(mixLabel);

        // Delay Knob
        delayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        delayKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f,
                                      true);
        delayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        delayKnob.setVelocityBasedMode(true);
        delayKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(delayKnob);
        delayAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "DELAY", delayKnob);
        delayLabel.setText("DELAY", juce::dontSendNotification);
        delayLabel.setJustificationType(juce::Justification::centred);
        delayLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        delayLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(delayLabel);

        // Chorus Knob
        chorusKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        chorusKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                       juce::MathConstants<float>::pi * 2.75f,
                                       true);
        chorusKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        chorusKnob.setVelocityBasedMode(true);
        chorusKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(chorusKnob);
        chorusAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "CHORUS", chorusKnob);
        chorusLabel.setText("CHORUS", juce::dontSendNotification);
        chorusLabel.setJustificationType(juce::Justification::centred);
        chorusLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        chorusLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(chorusLabel);

        // Effects title labels (static titles above knobs)
        mixTitleLabel.setText("MIX", juce::dontSendNotification);
        mixTitleLabel.setJustificationType(juce::Justification::centred);
        mixTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        mixTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(mixTitleLabel);

        delayTitleLabel.setText("DELAY", juce::dontSendNotification);
        delayTitleLabel.setJustificationType(juce::Justification::centred);
        delayTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        delayTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(delayTitleLabel);

        chorusTitleLabel.setText("CHORUS", juce::dontSendNotification);
        chorusTitleLabel.setJustificationType(juce::Justification::centred);
        chorusTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        chorusTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(chorusTitleLabel);

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
        setSize(720, 490);

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
        auto bounds = getLocalBounds().reduced(11).toFloat();

        // METRI AI LATI ESTREMI - sezioni ridotte proporzionalmente
        auto inputMeterArea = bounds.removeFromLeft(56.f);
        auto outputMeterArea = bounds.removeFromRight(56.f);

        auto leftSection = bounds.removeFromLeft(175.f);
        auto rightSection = bounds;

        auto leftTop = leftSection.removeFromTop(224.f);
        auto leftBottom = leftSection;

        auto spectrumSection = rightSection.removeFromTop(245.f);
        auto eqSection = rightSection;

        // Panels per meter
        lookAndFeel.drawSectionPanel(g, inputMeterArea.reduced(2.f), 4.f);
        lookAndFeel.drawFrameLabel(g, inputMeterArea.reduced(4.f).removeFromTop(13.f), "IN");

        lookAndFeel.drawSectionPanel(g, outputMeterArea.reduced(2.f), 4.f);
        lookAndFeel.drawFrameLabel(g, outputMeterArea.reduced(4.f).removeFromTop(13.f), "OUT");

        // Drive + Type
        lookAndFeel.drawSectionPanel(g, leftTop.reduced(4.f), 7.f);
        lookAndFeel.drawFrameLabel(g, leftTop.reduced(7.f).removeFromTop(14.f).withWidth(84.f), "COLOR");

        // Effects section
        lookAndFeel.drawSectionPanel(g, leftBottom.reduced(4.f), 6.f);
        lookAndFeel.drawFrameLabel(g, leftBottom.reduced(7.f).removeFromTop(13.f).withWidth(70.f), "EFFECTS");

        // EQ section
        lookAndFeel.drawSectionPanel(g, eqSection.reduced(4.f), 7.f);
        lookAndFeel.drawFrameLabel(g, eqSection.reduced(7.f).removeFromTop(13.f).withWidth(105.f), "EQ / FILTER");

        // EQ separators between bands
        {
            auto separatorArea = eqSection.reduced(7.f);
            separatorArea.removeFromTop(20.f);   // Skip header area
            separatorArea.removeFromBottom(7.f); // Skip bottom padding

            const int bandCount = 4;
            int totalWidth = (int)separatorArea.getWidth();
            int bandWidth = totalWidth / bandCount;

            // Draw 3 separator lines between the 4 bands
            g.setColour(juce::Colour(0xFF333333));
            for (int i = 1; i < bandCount; ++i)
            {
                int separatorX = (int)separatorArea.getX() + i * bandWidth;
                g.drawVerticalLine(separatorX, separatorArea.getY(), separatorArea.getBottom());
            }
        }
    }

    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        // Update EQ parameter labels dynamically
        updateEQLabels();

        // Update main control labels dynamically
        updateMainControlLabels();
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds().reduced(11);

        // METER
        auto inputMeterArea = bounds.removeFromLeft(56);
        auto outputMeterArea = bounds.removeFromRight(56);

        // Layout principale: left (controls) e right (EQ+spectrum)
        auto leftSection = bounds.removeFromLeft(175);
        auto rightSection = bounds; // Sezione destra per EQ e spectrum

        // LEFT SECTION - dividiamo in top (drive + type) e bottom (effects)
        auto leftTop = leftSection.removeFromTop(224);
        auto leftBottom = leftSection;

        // RIGHT SECTION - spectrum e EQ più grandi per colmare lo spazio vuoto
        auto spectrumSection = rightSection.removeFromTop(245);
        auto eqSection = rightSection;

        // INPUT METER + KNOB
        {
            const int meterH = 385, meterW = 14;
            const int knobSize = 42;
            const int topPadding = 18;
            const int bottomPadding = 11;
            const int meterKnobGap = 7;

            auto workingArea = inputMeterArea.reduced(4, topPadding);
            auto cx = inputMeterArea.getCentreX();

            // Knob in fondo alla sezione
            auto knobY = inputMeterArea.getBottom() - bottomPadding - knobSize;
            inputGainKnob.setBounds(juce::Rectangle<int>(0, knobY, knobSize, knobSize).withCentre({cx, knobY + knobSize / 2}));

            // Meter posizionato proprio sopra il knob
            auto meterBottomY = knobY - meterKnobGap;
            auto meterY = meterBottomY - meterH;
            auto inputMeterBounds = juce::Rectangle<int>(0, meterY, meterW, meterH).withCentre({cx, meterY + meterH / 2});
            inputMeter.setBounds(inputMeterBounds);
        }

        // OUTPUT METER + KNOB
        {
            const int meterH = 385, meterW = 14;
            const int knobSize = 42;
            const int topPadding = 18;
            const int bottomPadding = 11;
            const int meterKnobGap = 7;

            auto workingArea = outputMeterArea.reduced(4, topPadding);
            auto cx = outputMeterArea.getCentreX();

            // Knob in fondo alla sezione
            auto knobY = outputMeterArea.getBottom() - bottomPadding - knobSize;
            outputGainKnob.setBounds(juce::Rectangle<int>(0, knobY, knobSize, knobSize).withCentre({cx, knobY + knobSize / 2}));

            // Meter posizionato proprio sopra il knob
            auto meterBottomY = knobY - meterKnobGap;
            auto meterY = meterBottomY - meterH;
            auto outputMeterBounds = juce::Rectangle<int>(0, meterY, meterW, meterH).withCentre({cx, meterY + meterH / 2});
            outputMeter.setBounds(outputMeterBounds);
        }

        // DRIVE + TYPE AREA (nella stessa sezione COLOR, come nell'immagine)
        {
            const int driveKnobSize = 84;
            const int driveLabelH = 12;
            const int labelGap = 2; // Same small gap as other knobs

            // Drive knob principale con etichetta vicina
            auto driveArea = leftTop.removeFromTop(154);
            auto driveCenterX = driveArea.getCentreX();
            auto driveCenterY = driveArea.getCentreY();

            // Posiziona il drive knob al centro dell'area
            auto driveKnobBounds = juce::Rectangle<int>(driveCenterX - driveKnobSize / 2, driveCenterY - driveKnobSize / 2, driveKnobSize, driveKnobSize);
            driveKnob.setBounds(driveKnobBounds);

            // Etichetta percentuale appena sotto il knob (molto vicina)
            auto driveLabelBounds = juce::Rectangle<int>(driveCenterX - 40, driveKnobBounds.getBottom() + labelGap, 80, driveLabelH);
            driveLabel.setBounds(driveLabelBounds);

            // Type selector nella stessa sezione (sotto il drive, non sezione separata)
            auto typeArea = leftTop.removeFromTop(56);
            auto typeLabelArea = typeArea.removeFromTop(18);
            saturationTypeLabel.setText("TYPE", juce::dontSendNotification);
            saturationTypeLabel.setBounds(typeLabelArea);
            saturationTypeSelector.setBounds(typeArea.reduced(14, 7));
        }

        // EFFECTS SECTION (bottom) - Mix in alto centro, Delay e Chorus sotto in orizzontale
        {
            const int knobSize = 46;
            const int titleH = 11;  // Title labels above knobs (reduced)
            const int valueH = 11;  // Value labels below knobs (reduced)
            const int labelGap = 2; // Very small gap between knobs and labels
            const int rowGap = 8;   // Gap between rows

            auto effectsArea = leftBottom.reduced(7);

            // Calcola le altezze per le due righe
            auto totalContentHeight = titleH + labelGap + knobSize + labelGap + valueH;
            auto topRowHeight = totalContentHeight + 50;

            auto topRow = effectsArea.removeFromTop(topRowHeight);
            effectsArea.removeFromTop(rowGap); // Gap tra le righe
            auto bottomRow = effectsArea;

            // TOP ROW: Mix knob centrato con etichette molto vicine
            {
                auto mixCenterX = topRow.getCentreX();
                auto mixCenterY = topRow.getCentreY();

                // Posiziona il knob al centro
                auto mixKnobBounds = juce::Rectangle<int>(mixCenterX - knobSize / 2, mixCenterY - knobSize / 2, knobSize, knobSize);
                mixKnob.setBounds(mixKnobBounds);

                // Etichetta titolo appena sopra il knob
                auto mixTitleBounds = juce::Rectangle<int>(mixCenterX - 30, mixKnobBounds.getY() - titleH - labelGap, 60, titleH);
                mixTitleLabel.setBounds(mixTitleBounds);

                // Etichetta valore appena sotto il knob
                auto mixValueBounds = juce::Rectangle<int>(mixCenterX - 30, mixKnobBounds.getBottom() + labelGap, 60, valueH);
                mixLabel.setBounds(mixValueBounds);
            }

            // BOTTOM ROW: Delay e Chorus affiancati con etichette molto vicine
            {
                auto halfWidth = bottomRow.getWidth() / 2;
                auto knobCenterY = bottomRow.getCentreY() - 20;

                // Delay (sinistra)
                auto delayCenterX = bottomRow.getX() + halfWidth / 2;
                auto delayKnobBounds = juce::Rectangle<int>(delayCenterX - knobSize / 2, knobCenterY - knobSize / 2, knobSize, knobSize);
                delayKnob.setBounds(delayKnobBounds);

                auto delayTitleBounds = juce::Rectangle<int>(delayCenterX - 30, delayKnobBounds.getY() - titleH - labelGap, 60, titleH);
                delayTitleLabel.setBounds(delayTitleBounds);

                auto delayValueBounds = juce::Rectangle<int>(delayCenterX - 30, delayKnobBounds.getBottom() + labelGap, 60, valueH);
                delayLabel.setBounds(delayValueBounds);

                // Chorus (destra)
                auto chorusCenterX = bottomRow.getX() + halfWidth + halfWidth / 2;
                auto chorusKnobBounds = juce::Rectangle<int>(chorusCenterX - knobSize / 2, knobCenterY - knobSize / 2, knobSize, knobSize);
                chorusKnob.setBounds(chorusKnobBounds);

                auto chorusTitleBounds = juce::Rectangle<int>(chorusCenterX - 30, chorusKnobBounds.getY() - titleH - labelGap, 60, titleH);
                chorusTitleLabel.setBounds(chorusTitleBounds);

                auto chorusValueBounds = juce::Rectangle<int>(chorusCenterX - 30, chorusKnobBounds.getBottom() + labelGap, 60, valueH);
                chorusLabel.setBounds(chorusValueBounds);
            }
        }

        // SPECTRUM SECTION (right top) - più grande per colmare spazio vuoto
        spectrumAnalyzer.setBounds(spectrumSection.reduced(7));

        // EQ SECTION (right bottom) - layout aggiornato: freq knobs più grandi e distanziati,
        // riempiono meglio l'area; Gain/Q con etichette più basse per migliore leggibilità.
        {
            auto area = eqSection.reduced(7);

            const int sectionHeaderOffset = 20;
            const int bandCount = 4;

            // Dimensioni scalate al ~70%
            const int freqKnobSize = juce::jmin(56, area.getWidth() / 9);
            const int smallKnobSize = 34;
            const int titleHeight = 13;
            const int labelHeight = 11;
            const int verticalGap = 35;
            const int smallKnobGap = 18;
            const int labelYOffset = 8;
            const int bottomPadding = 7;

            area.removeFromTop(sectionHeaderOffset);
            area.removeFromBottom(bottomPadding);

            // Divisione equa in 4 bande orizzontali
            int totalWidth = area.getWidth();
            int bandWidth = totalWidth / bandCount;

            // Asse Y principale per i freq knobs - spostato leggermente verso l'alto
            int totalVerticalSpace = area.getHeight() - titleHeight - 4;
            int knobsAreaHeight = freqKnobSize + verticalGap + smallKnobSize + labelYOffset + labelHeight;
            int verticalCenterOffset = (totalVerticalSpace - knobsAreaHeight) / 3;

            int centerYFreqRow = area.getY() + titleHeight + 4 + verticalCenterOffset + freqKnobSize / 2;
            int secondRowY = centerYFreqRow + freqKnobSize / 2 + verticalGap + smallKnobSize / 2;

            auto placeBand = [&](int bandIndex,
                                 juce::Label &title,
                                 juce::Slider &freqKnob, juce::Label &freqLabel,
                                 juce::Slider *gainKnob, juce::Label *gainLabel,
                                 juce::Slider *qKnob, juce::Label *qLabel)
            {
                // Centro della banda corrente
                int bandLeft = area.getX() + bandIndex * bandWidth;
                int centerX = bandLeft + bandWidth / 2;

                // Titolo centrato nella banda
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
                        // Etichetta più larga per evitare troncature
                        int labelWidth = smallKnobSize + 20; // Aggiungo 20 pixel per più spazio
                        int labelX = k.getCentreX() - labelWidth / 2;
                        lab->setBounds(labelX, k.getBottom() + labelYOffset, labelWidth, labelHeight);
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
                    // Etichetta più larga per evitare troncature
                    int labelWidth = smallKnobSize + 20; // Aggiungo 20 pixel per più spazio
                    int labelX = k.getCentreX() - labelWidth / 2;
                    qLabel->setBounds(labelX, k.getBottom() + labelYOffset, labelWidth, labelHeight);
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
        lowCutFreqKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(lowCutFreqKnob);
        lowCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_FREQ", lowCutFreqKnob);
        lowCutFreqLabel.setText("", juce::dontSendNotification);
        lowCutFreqLabel.setJustificationType(juce::Justification::centred);
        lowCutFreqLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        lowCutFreqLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(lowCutFreqLabel);

        lowCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowCutQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
        lowCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowCutQKnob.setVelocityBasedMode(true);
        lowCutQKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(lowCutQKnob);
        lowCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_CUT_Q", lowCutQKnob);
        lowCutQLabel.setText("Q", juce::dontSendNotification);
        lowCutQLabel.setJustificationType(juce::Justification::centred);
        lowCutQLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        lowCutQLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(lowCutQLabel);

        // Low Mid Frequency, Gain & Q
        lowMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        lowMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidFreqKnob.setVelocityBasedMode(true);
        lowMidFreqKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(lowMidFreqKnob);
        lowMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_FREQ", lowMidFreqKnob);
        lowMidFreqLabel.setText("", juce::dontSendNotification);
        lowMidFreqLabel.setJustificationType(juce::Justification::centred);
        lowMidFreqLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        lowMidFreqLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(lowMidFreqLabel);

        lowMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                           juce::MathConstants<float>::pi * 2.75f,
                                           true);
        lowMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidGainKnob.setVelocityBasedMode(true);
        lowMidGainKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(lowMidGainKnob);
        lowMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_GAIN", lowMidGainKnob);
        lowMidGainLabel.setText("Gain", juce::dontSendNotification);
        lowMidGainLabel.setJustificationType(juce::Justification::centred);
        lowMidGainLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        lowMidGainLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(lowMidGainLabel);

        lowMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lowMidQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
        lowMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        lowMidQKnob.setVelocityBasedMode(true);
        lowMidQKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(lowMidQKnob);
        lowMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "LOW_MID_Q", lowMidQKnob);
        lowMidQLabel.setText("Q", juce::dontSendNotification);
        lowMidQLabel.setJustificationType(juce::Justification::centred);
        lowMidQLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        lowMidQLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(lowMidQLabel);

        // High Mid Frequency, Gain & Q
        highMidFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highMidFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidFreqKnob.setVelocityBasedMode(true);
        highMidFreqKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(highMidFreqKnob);
        highMidFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_FREQ", highMidFreqKnob);
        highMidFreqLabel.setText("", juce::dontSendNotification);
        highMidFreqLabel.setJustificationType(juce::Justification::centred);
        highMidFreqLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        highMidFreqLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(highMidFreqLabel);

        highMidGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidGainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highMidGainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidGainKnob.setVelocityBasedMode(true);
        highMidGainKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(highMidGainKnob);
        highMidGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_GAIN", highMidGainKnob);
        highMidGainLabel.setText("Gain", juce::dontSendNotification);
        highMidGainLabel.setJustificationType(juce::Justification::centred);
        highMidGainLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        highMidGainLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(highMidGainLabel);

        highMidQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highMidQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                         juce::MathConstants<float>::pi * 2.75f,
                                         true);
        highMidQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highMidQKnob.setVelocityBasedMode(true);
        highMidQKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(highMidQKnob);
        highMidQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_MID_Q", highMidQKnob);
        highMidQLabel.setText("Q", juce::dontSendNotification);
        highMidQLabel.setJustificationType(juce::Justification::centred);
        highMidQLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        highMidQLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(highMidQLabel);

        // High Cut Frequency & Q
        highCutFreqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutFreqKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                            juce::MathConstants<float>::pi * 2.75f,
                                            true);
        highCutFreqKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highCutFreqKnob.setVelocityBasedMode(true);
        highCutFreqKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(highCutFreqKnob);
        highCutFreqAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_FREQ", highCutFreqKnob);
        highCutFreqLabel.setText("", juce::dontSendNotification);
        highCutFreqLabel.setJustificationType(juce::Justification::centred);
        highCutFreqLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        highCutFreqLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(highCutFreqLabel);

        highCutQKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        highCutQKnob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                         juce::MathConstants<float>::pi * 2.75f,
                                         true);
        highCutQKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highCutQKnob.setVelocityBasedMode(true);
        highCutQKnob.setVelocityModeParameters(0.5, 1, 0.1, false); // More sensitive
        addAndMakeVisible(highCutQKnob);
        highCutQAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "HIGH_CUT_Q", highCutQKnob);
        highCutQLabel.setText("Q", juce::dontSendNotification);
        highCutQLabel.setJustificationType(juce::Justification::centred);
        highCutQLabel.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        highCutQLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(highCutQLabel);
    }

    juce::String AudioPluginAudioProcessorEditor::formatFrequency(float freq)
    {
        if (freq >= 1000.0f)
        {
            return juce::String(freq / 1000.0f, 1) + " kHz";
        }
        else
        {
            return juce::String((int)freq) + " Hz";
        }
    }

    juce::String AudioPluginAudioProcessorEditor::formatGain(float gain)
    {
        if (gain > 0.0f)
            return "+" + juce::String(gain, 1) + " dB";
        else if (gain < 0.0f)
            return juce::String(gain, 1) + " dB";
        else
            return "0 dB";
    }

    juce::String AudioPluginAudioProcessorEditor::formatQ(float q)
    {
        // For values >= 10, show 1 decimal place; for smaller values, show 2
        if (q >= 10.0f)
            return juce::String(q, 1);
        else
            return juce::String(q, 2);
    }

    void AudioPluginAudioProcessorEditor::updateEQLabels()
    {
        // Low Cut
        auto lowCutFreq = processorRef.apvts.getRawParameterValue("LOW_CUT_FREQ")->load();
        auto lowCutQ = processorRef.apvts.getRawParameterValue("LOW_CUT_Q")->load();

        if (lowCutFreq > 0.0f)
        {
            lowCutFreqLabel.setText(formatFrequency(lowCutFreq), juce::dontSendNotification);
            lowCutQLabel.setText(formatQ(lowCutQ), juce::dontSendNotification);
        }
        else
        {
            lowCutFreqLabel.setText("", juce::dontSendNotification);
            lowCutQLabel.setText("Q", juce::dontSendNotification);
        }

        // Low Mid
        auto lowMidFreq = processorRef.apvts.getRawParameterValue("LOW_MID_FREQ")->load();
        auto lowMidGain = processorRef.apvts.getRawParameterValue("LOW_MID_GAIN")->load();
        auto lowMidQ = processorRef.apvts.getRawParameterValue("LOW_MID_Q")->load();

        lowMidFreqLabel.setText(formatFrequency(lowMidFreq), juce::dontSendNotification);

        if (juce::approximatelyEqual(lowMidGain, 0.0f))
        {
            lowMidGainLabel.setText("Gain", juce::dontSendNotification);
            lowMidQLabel.setText("Q", juce::dontSendNotification);
        }
        else
        {
            lowMidGainLabel.setText(formatGain(lowMidGain), juce::dontSendNotification);
            lowMidQLabel.setText(formatQ(lowMidQ), juce::dontSendNotification);
        }

        // High Mid
        auto highMidFreq = processorRef.apvts.getRawParameterValue("HIGH_MID_FREQ")->load();
        auto highMidGain = processorRef.apvts.getRawParameterValue("HIGH_MID_GAIN")->load();
        auto highMidQ = processorRef.apvts.getRawParameterValue("HIGH_MID_Q")->load();

        highMidFreqLabel.setText(formatFrequency(highMidFreq), juce::dontSendNotification);

        if (juce::approximatelyEqual(highMidGain, 0.0f))
        {
            highMidGainLabel.setText("Gain", juce::dontSendNotification);
            highMidQLabel.setText("Q", juce::dontSendNotification);
        }
        else
        {
            highMidGainLabel.setText(formatGain(highMidGain), juce::dontSendNotification);
            highMidQLabel.setText(formatQ(highMidQ), juce::dontSendNotification);
        }

        // High Cut
        auto highCutFreq = processorRef.apvts.getRawParameterValue("HIGH_CUT_FREQ")->load();
        auto highCutQ = processorRef.apvts.getRawParameterValue("HIGH_CUT_Q")->load();

        if (highCutFreq < 20000.0f) // Active only when < 20kHz (inverted logic)
        {
            highCutFreqLabel.setText(formatFrequency(highCutFreq), juce::dontSendNotification);
            highCutQLabel.setText(formatQ(highCutQ), juce::dontSendNotification);
        }
        else
        {
            highCutFreqLabel.setText("", juce::dontSendNotification);
            highCutQLabel.setText("Q", juce::dontSendNotification);
        }
    }

    juce::String AudioPluginAudioProcessorEditor::formatPercentage(float normalizedValue)
    {
        int percentage = (int)(normalizedValue * 100.0f);
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

    void AudioPluginAudioProcessorEditor::updateMainControlLabels()
    {
        // Drive knob (0.0 to 1.0 normalized)
        auto driveValue = processorRef.apvts.getRawParameterValue("DRIVE")->load();
        driveLabel.setText(formatPercentage(driveValue), juce::dontSendNotification);

        // Mix knob (0.0 to 1.0 normalized)
        auto mixValue = processorRef.apvts.getRawParameterValue("MIX")->load();
        mixLabel.setText(formatPercentage(mixValue), juce::dontSendNotification);

        // Delay knob (0.0 to 1.0 normalized)
        auto delayValue = processorRef.apvts.getRawParameterValue("DELAY")->load();
        delayLabel.setText(formatPercentage(delayValue), juce::dontSendNotification);

        // Chorus knob (0.0 to 1.0 normalized)
        auto chorusValue = processorRef.apvts.getRawParameterValue("CHORUS")->load();
        chorusLabel.setText(formatPercentage(chorusValue), juce::dontSendNotification);
    }
}
