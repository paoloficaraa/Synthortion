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

        setupKnob(driveKnob);
        addAndMakeVisible(driveKnob);
        driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "COLOR", driveKnob);
        driveLabel.setText("", juce::dontSendNotification);
        driveLabel.setJustificationType(juce::Justification::centred);
        driveLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
        driveLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(driveLabel);

        setupKnob(inputGainKnob);
        addAndMakeVisible(inputGainKnob);
        inputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "INPUT_GAIN", inputGainKnob);

        setupKnob(outputGainKnob);
        addAndMakeVisible(outputGainKnob);
        outputGainAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "OUTPUT_GAIN", outputGainKnob);

        setupKnobWithLabel(bitCrushKnob, bitCrushTitleLabel, bitCrushLabel, "BITCRUSH", "BITCRUSH", bitCrushAttachment);
        setupKnobWithLabel(delayTimeKnob, delayTimeTitleLabel, delayTimeLabel, "DELAY TIME", "DELAY_TIME", delayTimeAttachment);
        setupKnobWithLabel(delayMixKnob, delayMixTitleLabel, delayMixLabel, "DELAY MIX", "DELAY_MIX", delayMixAttachment);
        setupKnobWithLabel(delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel, "DELAY FB", "DELAY_FEEDBACK", delayFeedbackAttachment);
        setupKnobWithLabel(chorusMixKnob, chorusMixTitleLabel, chorusMixLabel, "CHORUS MIX", "CHORUS_MIX", chorusMixAttachment);

        // Preset Selector (commented out - to be implemented later)
        // presetSelector.addItemList({"User", "Clean Tape", "Lofi Chaos", "Ambient Wash", "Aggressive Crunch"}, 1);
        // addAndMakeVisible(presetSelector);
        // presetAttachment = std::make_unique<ComboBoxAttachment>(processorRef.apvts, "PRESET", presetSelector);
        // presetLabel.setText("PRESET", juce::dontSendNotification);
        // presetLabel.setJustificationType(juce::Justification::centred);
        // addAndMakeVisible(presetLabel);

        // Effects title labels (static titles above knobs)
        bitCrushTitleLabel.setText("BITCRUSH", juce::dontSendNotification);
        bitCrushTitleLabel.setJustificationType(juce::Justification::centred);
        bitCrushTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        bitCrushTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(bitCrushTitleLabel);

        delayTimeTitleLabel.setText("DELAY TIME", juce::dontSendNotification);
        delayTimeTitleLabel.setJustificationType(juce::Justification::centred);
        delayTimeTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        delayTimeTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(delayTimeTitleLabel);

        delayMixTitleLabel.setText("DELAY MIX", juce::dontSendNotification);
        delayMixTitleLabel.setJustificationType(juce::Justification::centred);
        delayMixTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        delayMixTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(delayMixTitleLabel);

        delayFeedbackTitleLabel.setText("DELAY FB", juce::dontSendNotification);
        delayFeedbackTitleLabel.setJustificationType(juce::Justification::centred);
        delayFeedbackTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        delayFeedbackTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(delayFeedbackTitleLabel);

        chorusMixTitleLabel.setText("CHORUS MIX", juce::dontSendNotification);
        chorusMixTitleLabel.setJustificationType(juce::Justification::centred);
        chorusMixTitleLabel.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        chorusMixTitleLabel.setColour(juce::Label::textColourId, LIGHT_GREY);
        addAndMakeVisible(chorusMixTitleLabel);

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

        // Connect EQ reference for curve visualization
        spectrumAnalyzer.setEQReference(&processorRef.getEQ());

        // Connect spectrum analyzer to audio processor
        // IMPORTANT: This creates a callback that captures 'this'.
        // The callback MUST be cleared in the destructor to prevent crashes.
        processorRef.setSpectrumAnalyzerCallback([this](const float* data, int numSamples)
                                                 {
                                                     for (int i = 0; i < numSamples; ++i)
                                                         spectrumAnalyzer.pushNextSampleIntoFifo(data[i]);
                                                 });

        setResizable(false, false);
        setSize(kWindowWidth, kWindowHeight);

        startTimerHz(kTimerHz);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        // CRITICAL: Clear the spectrum analyzer callback to prevent crashes
        // when the editor is destroyed while audio is still processing
        processorRef.setSpectrumAnalyzerCallback(nullptr);

        setLookAndFeel(nullptr);
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

        auto leftTop = leftSection.removeFromTop(160.f); // Ridotto da 224 a 160
        auto leftBottom = leftSection;

        auto spectrumSection = rightSection.removeFromTop(245.f);
        auto eqSection = rightSection;

        // Panels per meter
        lookAndFeel.drawSectionPanel(g, inputMeterArea.reduced(2.f), 4.f);
        lookAndFeel.drawFrameLabel(g, inputMeterArea.reduced(4.f).removeFromTop(13.f), "IN");

        lookAndFeel.drawSectionPanel(g, outputMeterArea.reduced(2.f), 4.f);
        lookAndFeel.drawFrameLabel(g, outputMeterArea.reduced(4.f).removeFromTop(13.f), "OUT");

        // Color section (master effects mix knob)
        lookAndFeel.drawSectionPanel(g, leftTop.reduced(4.f), 7.f);
        lookAndFeel.drawFrameLabel(g, leftTop.reduced(7.f).removeFromTop(14.f).withWidth(70.f), "COLOR");

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

        // Update EQ bypass state in spectrum analyzer
        auto eqBypass = processorRef.apvts.getRawParameterValue("EQ_BYPASS")->load() > 0.5f;
        spectrumAnalyzer.setEQBypass(eqBypass);
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
        auto leftTop = leftSection.removeFromTop(160); // Ridotto da 224 a 160
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

        // COLOR AREA (master effects mix - large knob)
        {
            const int colorKnobSize = 70; // Ridotto da 84
            const int colorLabelH = 12;
            const int labelGap = 2;

            auto colorCenterX = leftTop.getCentreX();
            auto colorCenterY = leftTop.getCentreY();

            // Color knob centrato
            auto colorKnobBounds = juce::Rectangle<int>(colorCenterX - colorKnobSize / 2, colorCenterY - colorKnobSize / 2, colorKnobSize, colorKnobSize);
            driveKnob.setBounds(colorKnobBounds);

            // Etichetta percentuale sotto il knob
            auto colorLabelBounds = juce::Rectangle<int>(colorCenterX - 40, colorKnobBounds.getBottom() + labelGap, 80, colorLabelH);
            driveLabel.setBounds(colorLabelBounds);
        }

        // EFFECTS SECTION - Layout 3 righe: BitCrush / Delay Time+Delay Mix / Delay Feedback+Chorus
        {
            const int knobSize = 46;
            const int titleH = 10;
            const int valueH = 10;
            const int labelGap = 1;
            const int rowGap = 8; // Spaziatura uniforme tra le righe
            const int labelWidth = 70;
            const int topMargin = 22; // Margine superiore per evitare sovrapposizione con "EFFECTS"

            auto effectsArea = leftBottom.reduced(10, 12);
            effectsArea.removeFromTop(topMargin); // Sposta tutto più in basso

            // Calcola lo spazio disponibile e dividilo equamente tra le 3 righe
            auto totalContentHeight = titleH + labelGap + knobSize + labelGap + valueH;
            auto availableHeight = effectsArea.getHeight();
            auto totalGaps = rowGap * 2; // 2 gap tra 3 righe
            auto totalRowsHeight = totalContentHeight * 3;
            auto extraSpace = availableHeight - totalRowsHeight - totalGaps;

            // Distribuisci lo spazio extra equamente
            auto rowHeight = totalContentHeight + (extraSpace / 3);

            auto row1 = effectsArea.removeFromTop(rowHeight);
            effectsArea.removeFromTop(rowGap);
            auto row2 = effectsArea.removeFromTop(rowHeight);
            effectsArea.removeFromTop(rowGap);
            auto row3 = effectsArea; // Usa tutto lo spazio rimanente

            // Helper per posizionare un knob con title e value label
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

            // ROW 1: BitCrush (Centrato)
            {
                placeKnob(row1, bitCrushKnob, bitCrushTitleLabel, bitCrushLabel);
            }

            // ROW 2: Delay Time, Delay Feedback
            {
                int halfWidth = row2.getWidth() / 2;

                auto delayTimeArea = row2.removeFromLeft(halfWidth);
                auto delayFeedbackArea = row2;

                placeKnob(delayTimeArea, delayTimeKnob, delayTimeTitleLabel, delayTimeLabel);
                placeKnob(delayFeedbackArea, delayFeedbackKnob, delayFeedbackTitleLabel, delayFeedbackLabel);
            }

            // ROW 3: Delay Mix, Chorus Mix
            {
                int halfWidth = row3.getWidth() / 2;

                auto delayMixArea = row3.removeFromLeft(halfWidth);
                auto chorusMixArea = row3;

                placeKnob(delayMixArea, delayMixKnob, delayMixTitleLabel, delayMixLabel);
                placeKnob(chorusMixArea, chorusMixKnob, chorusMixTitleLabel, chorusMixLabel);
            }
        } // SPECTRUM SECTION (right top) - più grande per colmare spazio vuoto
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

            // EQ Bypass Button - positioned in top right of EQ section
            int bypassWidth = 80;
            int bypassHeight = 20;
            int bypassX = eqSection.getRight() - bypassWidth - 10;
            int bypassY = eqSection.getY() + 5;
            eqBypassButton.setBounds(bypassX, bypassY, bypassWidth, bypassHeight);
        }
    }
    void AudioPluginAudioProcessorEditor::setupEQControls()
    {
        auto setupEQKnob = [this](juce::Slider& knob, juce::Label& label, 
                                  const juce::String& paramId, const juce::String& defaultText,
                                  std::unique_ptr<SliderAttachment>& attachment)
        {
            setupKnob(knob);
            addAndMakeVisible(knob);
            attachment = std::make_unique<SliderAttachment>(processorRef.apvts, paramId, knob);
            
            label.setText(defaultText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
            label.setColour(juce::Label::textColourId, LIGHT_GREY);
            addAndMakeVisible(label);
        };

        setupEQKnob(lowCutFreqKnob, lowCutFreqLabel, "LOW_CUT_FREQ", "", lowCutFreqAttachment);
        setupEQKnob(lowCutQKnob, lowCutQLabel, "LOW_CUT_Q", "Q", lowCutQAttachment);

        setupEQKnob(lowMidFreqKnob, lowMidFreqLabel, "LOW_MID_FREQ", "", lowMidFreqAttachment);
        setupEQKnob(lowMidGainKnob, lowMidGainLabel, "LOW_MID_GAIN", "Gain", lowMidGainAttachment);
        setupEQKnob(lowMidQKnob, lowMidQLabel, "LOW_MID_Q", "Q", lowMidQAttachment);

        setupEQKnob(highMidFreqKnob, highMidFreqLabel, "HIGH_MID_FREQ", "", highMidFreqAttachment);
        setupEQKnob(highMidGainKnob, highMidGainLabel, "HIGH_MID_GAIN", "Gain", highMidGainAttachment);
        setupEQKnob(highMidQKnob, highMidQLabel, "HIGH_MID_Q", "Q", highMidQAttachment);

        setupEQKnob(highCutFreqKnob, highCutFreqLabel, "HIGH_CUT_FREQ", "", highCutFreqAttachment);
        setupEQKnob(highCutQKnob, highCutQLabel, "HIGH_CUT_Q", "Q", highCutQAttachment);

        eqBypassButton.setButtonText("EQ BYPASS");
        eqBypassButton.setColour(juce::ToggleButton::textColourId, LIGHT_GREY);
        eqBypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::orange);
        addAndMakeVisible(eqBypassButton);
        eqBypassAttachment = std::make_unique<ButtonAttachment>(processorRef.apvts, "EQ_BYPASS", eqBypassButton);
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
            lowMidGainLabel.setText(formatDB(lowMidGain), juce::dontSendNotification);
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
            highMidGainLabel.setText(formatDB(highMidGain), juce::dontSendNotification);
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

    juce::String AudioPluginAudioProcessorEditor::formatMilliseconds(float ms)
    {
        if (ms >= 1000.0f)
            return juce::String(ms / 1000.0f, 2) + " s";
        else
            return juce::String((int)ms) + " ms";
    }

    void AudioPluginAudioProcessorEditor::updateMainControlLabels()
    {
        auto colorValue = processorRef.apvts.getRawParameterValue("COLOR")->load();
        driveLabel.setText(formatPercentage(colorValue), juce::dontSendNotification);

        // BitCrush knob
        auto bitCrushValue = processorRef.apvts.getRawParameterValue("BITCRUSH")->load();
        bitCrushLabel.setText(formatPercentage(bitCrushValue), juce::dontSendNotification);

        // Delay Time knob (1-2000 ms)
        auto delayTimeValue = processorRef.apvts.getRawParameterValue("DELAY_TIME")->load();
        delayTimeLabel.setText(formatMilliseconds(delayTimeValue), juce::dontSendNotification);

        // Delay Mix knob (0.0 to 1.0 normalized)
        auto delayMixValue = processorRef.apvts.getRawParameterValue("DELAY_MIX")->load();
        delayMixLabel.setText(formatPercentage(delayMixValue), juce::dontSendNotification);

        // Delay Feedback knob (0.0 to 0.9 normalized)
        auto delayFeedbackValue = processorRef.apvts.getRawParameterValue("DELAY_FEEDBACK")->load();
        delayFeedbackLabel.setText(juce::String(delayFeedbackValue * 100), juce::dontSendNotification);

        // Chorus Mix knob (0.0 to 1.0 normalized)
        auto chorusMixValue = processorRef.apvts.getRawParameterValue("CHORUS_MIX")->load();
        chorusMixLabel.setText(formatPercentage(chorusMixValue), juce::dontSendNotification);
    }
}
