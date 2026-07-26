#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"

namespace synthortion
{
    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p),
          processorRef (p)
    {
        setOpaque (true);

        titleLabel.setText ("Synthortion", juce::dontSendNotification);
        titleLabel.setFont (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 22.0f, juce::Font::bold));
        titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
        titleLabel.setJustificationType (juce::Justification::left);
        addAndMakeVisible (titleLabel);

        comingSoonLabel.setText ("COMING SOON", juce::dontSendNotification);
        comingSoonLabel.setFont (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::bold));
        comingSoonLabel.setColour (juce::Label::textColourId, juce::Colours::white);
        comingSoonLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (comingSoonLabel);

        auto setupTitleLabel = [this] (juce::Label& label, const juce::String& text)
        {
            label.setText (text, juce::dontSendNotification);
            label.setFont (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 11.0f, juce::Font::plain));
            label.setColour (juce::Label::textColourId, juce::Colours::white);
            label.setJustificationType (juce::Justification::centred);
            addAndMakeVisible (label);
        };

        setupTitleLabel (inputGainTitleLabel, "INPUT");
        setupTitleLabel (outputGainTitleLabel, "OUTPUT");
        setupTitleLabel (colorTitleLabel, "COLOR");
        setupTitleLabel (bitCrushTitleLabel, "BIT CRUSH");
        setupTitleLabel (chorusMixTitleLabel, "CHORUS MIX");
        setupTitleLabel (delayTimeTitleLabel, "TIME");
        setupTitleLabel (delayFeedbackTitleLabel, "FDBK");
        setupTitleLabel (delayMixTitleLabel, "MIX");

        auto setupPercentageSlider = [this] (juce::Slider& slider)
        {
            slider.setSliderStyle (juce::Slider::Rotary);
            slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
            addAndMakeVisible (slider);
        };

        auto setupDbSlider = [this] (juce::Slider& slider)
        {
            slider.setSliderStyle (juce::Slider::Rotary);
            slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
            addAndMakeVisible (slider);
        };

        auto setupMsSlider = [this] (juce::Slider& slider)
        {
            slider.setSliderStyle (juce::Slider::Rotary);
            slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
            addAndMakeVisible (slider);
        };

        setupDbSlider (inputGainSlider);
        setupDbSlider (outputGainSlider);
        setupPercentageSlider (colorSlider);
        setupPercentageSlider (bitCrushSlider);
        setupPercentageSlider (chorusMixSlider);
        setupMsSlider (delayTimeSlider);
        setupPercentageSlider (delayFeedbackSlider);
        setupPercentageSlider (delayMixSlider);

        bypassButton.setButtonText ("BYPASS");
        addAndMakeVisible (bypassButton);

        auto& apvts = processorRef.apvts;

        auto setRangeFromNormalisable = [] (juce::Slider& slider, const juce::NormalisableRange<float>& r)
        {
            slider.setRange (static_cast<double> (r.start),
                             static_cast<double> (r.end),
                             static_cast<double> (r.interval));
        };

        juce::NormalisableRange<float> gainRange (-60.0f, 12.0f, 0.1f);
        gainRange.setSkewForCentre (0.0f);

        juce::NormalisableRange<float> unitRange (0.0f, 1.0f, 0.01f);
        juce::NormalisableRange<float> feedbackRange (0.0f, 0.95f, 0.01f);
        juce::NormalisableRange<float> msRange (1.0f, 2000.0f, 1.0f);

        setRangeFromNormalisable (inputGainSlider, gainRange);
        inputGainSlider.setValue (0.0, juce::dontSendNotification);
        setRangeFromNormalisable (outputGainSlider, gainRange);
        outputGainSlider.setValue (0.0, juce::dontSendNotification);
        setRangeFromNormalisable (colorSlider, unitRange);
        colorSlider.setValue (0.0, juce::dontSendNotification);
        setRangeFromNormalisable (bitCrushSlider, unitRange);
        bitCrushSlider.setValue (0.0, juce::dontSendNotification);
        setRangeFromNormalisable (chorusMixSlider, unitRange);
        chorusMixSlider.setValue (0.0, juce::dontSendNotification);
        setRangeFromNormalisable (delayTimeSlider, msRange);
        delayTimeSlider.setValue (250.0, juce::dontSendNotification);
        setRangeFromNormalisable (delayFeedbackSlider, feedbackRange);
        delayFeedbackSlider.setValue (0.4, juce::dontSendNotification);
        setRangeFromNormalisable (delayMixSlider, unitRange);
        delayMixSlider.setValue (0.0, juce::dontSendNotification);

        inputGainSlider.setTextValueSuffix (" dB");
        inputGainSlider.setNumDecimalPlacesToDisplay (1);
        outputGainSlider.setTextValueSuffix (" dB");
        outputGainSlider.setNumDecimalPlacesToDisplay (1);
        delayTimeSlider.setTextValueSuffix (" ms");
        delayTimeSlider.setNumDecimalPlacesToDisplay (0);

        inputGainAttachment = std::make_unique<SliderAttachment> (apvts, "INPUT_GAIN", inputGainSlider);
        outputGainAttachment = std::make_unique<SliderAttachment> (apvts, "OUTPUT_GAIN", outputGainSlider);
        colorAttachment = std::make_unique<SliderAttachment> (apvts, "COLOR", colorSlider);
        bitCrushAttachment = std::make_unique<SliderAttachment> (apvts, "BITCRUSH", bitCrushSlider);
        chorusMixAttachment = std::make_unique<SliderAttachment> (apvts, "CHORUS_MIX", chorusMixSlider);
        delayTimeAttachment = std::make_unique<SliderAttachment> (apvts, "DELAY_TIME", delayTimeSlider);
        delayFeedbackAttachment = std::make_unique<SliderAttachment> (apvts, "DELAY_FEEDBACK", delayFeedbackSlider);
        delayMixAttachment = std::make_unique<SliderAttachment> (apvts, "DELAY_MIX", delayMixSlider);
        bypassAttachment = std::make_unique<ButtonAttachment> (apvts, "PLUGIN_BYPASS", bypassButton);

        setResizable (false, false);
        setSize (kWindowWidth, kWindowHeight);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() = default;

    void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::darkgrey);
        g.setColour (juce::Colours::black);
        g.drawHorizontalLine (kSeparatorY1, 0.0f, static_cast<float> (kWindowWidth));
        g.drawHorizontalLine (kSeparatorY2, static_cast<float> (kSidebarWidth), static_cast<float> (kWindowWidth));
        g.drawHorizontalLine (kSeparatorY3, static_cast<float> (kSidebarWidth), static_cast<float> (kWindowWidth));
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();

        // Header (y 0..60)
        auto headerArea = bounds.removeFromTop (kHeaderHeight);
        titleLabel.setBounds (headerArea.removeFromLeft (kSidebarWidth * 3).reduced (12, 12));
        bypassButton.setBounds (headerArea.removeFromRight (kSidebarWidth).reduced (12, 18));

        // Sidebar (left 120 px)
        auto sidebarLeft = bounds.removeFromLeft (kSidebarWidth);
        auto inputGainArea = sidebarLeft.removeFromTop (sidebarLeft.getHeight() / 2);
        inputGainTitleLabel.setBounds (inputGainArea.removeFromTop (16));
        inputGainSlider.setBounds (inputGainArea.reduced (10));
        auto outputGainArea = sidebarLeft;
        outputGainTitleLabel.setBounds (outputGainArea.removeFromTop (16));
        outputGainSlider.setBounds (outputGainArea.reduced (10));

        // Bottom main area
        auto mainArea = bounds;

        // Distortion: x 140..420, y 80..240
        auto distortionArea = juce::Rectangle<int> (140, kSeparatorY1 + 20, 280, kSeparatorY2 - kSeparatorY1 - 40);
        auto distLeftHalf = distortionArea.removeFromLeft (distortionArea.getWidth() / 2);
        colorTitleLabel.setBounds (distLeftHalf.removeFromTop (16));
        colorSlider.setBounds (distLeftHalf.reduced (10));
        bitCrushTitleLabel.setBounds (distortionArea.removeFromTop (16));
        bitCrushSlider.setBounds (distortionArea.reduced (10));

        // Chorus: x 140..420, y 260..420
        auto chorusArea = juce::Rectangle<int> (140, kSeparatorY2 + 20, 280, kSeparatorY3 - kSeparatorY2 - 40);
        chorusMixTitleLabel.setBounds (chorusArea.removeFromTop (16));
        chorusMixSlider.setBounds (chorusArea.reduced (10));

        // Delay: x 440..660, y 80..240
        auto delayArea = juce::Rectangle<int> (440, kSeparatorY1 + 20, 220, kSeparatorY2 - kSeparatorY1 - 40);
        auto delayOneThird = delayArea.getWidth() / 3;
        auto d1 = delayArea.removeFromLeft (delayOneThird);
        delayTimeTitleLabel.setBounds (d1.removeFromTop (16));
        delayTimeSlider.setBounds (d1.reduced (6));
        auto d2 = delayArea.removeFromLeft (delayOneThird);
        delayFeedbackTitleLabel.setBounds (d2.removeFromTop (16));
        delayFeedbackSlider.setBounds (d2.reduced (6));
        delayMixTitleLabel.setBounds (delayArea.removeFromTop (16));
        delayMixSlider.setBounds (delayArea.reduced (6));

        // Coming Soon: x 680..780, y 80..240
        comingSoonLabel.setBounds (680, kSeparatorY1 + 20, 100, kSeparatorY2 - kSeparatorY1 - 40);
    }

    juce::String AudioPluginAudioProcessorEditor::formatDB (float dbValue)
    {
        if (dbValue > 0.0f)
            return "+" + juce::String (dbValue, 1) + " dB";

        if (dbValue < 0.0f)
            return juce::String (dbValue, 1) + " dB";

        return "0 dB";
    }

    juce::String AudioPluginAudioProcessorEditor::formatPercentage (float normalizedValue)
    {
        const int percentage = static_cast<int> (normalizedValue * 100.0f);
        return juce::String (percentage) + "%";
    }

    juce::String AudioPluginAudioProcessorEditor::formatMilliseconds (float msValue)
    {
        return juce::String (static_cast<int> (msValue)) + " ms";
    }
}
