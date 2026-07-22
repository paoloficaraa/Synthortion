#include "Synthortion/PanelComponent.h"

namespace synthortion
{
    namespace
    {
        constexpr float kTitleHeight = 22.0f;
        constexpr float kContentTopPad = 6.0f;
        constexpr float kPlaceholderBottomPad = 16.0f;
    }
    PanelComponent::PanelComponent (const juce::String& panelTitle, const juce::Colour& backgroundColour)
        : title (panelTitle),
          bgColour (backgroundColour),
          headingFont (juce::FontOptions().withName("BebasNeue").withHeight(kTitleHeight).withStyle("Regular").withKerningFactor(SynthortionLookAndFeel::kTightKerning))
    {
        setOpaque (true);
    }

    PanelComponent::~PanelComponent()
    {
        stopTimer();
    }

    void PanelComponent::paint (juce::Graphics& g)
    {
        g.fillAll (bgColour);

        if (auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel()))
        {
            laf->drawPanelBackground (g, getLocalBounds(), false, title, bgColour);
        }
        else
        {
            g.setColour (bgColour);
            g.fillRect (getLocalBounds().toFloat());

            g.setColour (juce::Colours::white);
            g.drawRect (getLocalBounds().toFloat(), 1.0f);

            if (title.isNotEmpty())
            {
                auto labelArea = getLocalBounds().toFloat().removeFromTop (kTitleHeight).reduced (8.0f, 0.0f);
                g.setColour (juce::Colours::white);
                g.setFont (headingFont);
                g.drawFittedText (title, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
            }
        }

        if (isComingSoonPlaceholder)
            drawPlaceholderContent (g);
    }

    void PanelComponent::resized()
    {
    }

    void PanelComponent::setPlaceholder (bool isPlaceholder) noexcept
    {
        isComingSoonPlaceholder = isPlaceholder;

        if (isComingSoonPlaceholder)
            startTimer (1000 / kCursorBlinkTimerHz);
        else
            stopTimer();
    }

    void PanelComponent::timerCallback()
    {
        advanceBlinkTick (1);
        repaint();
    }

    void PanelComponent::advanceBlinkTick (int ticks) noexcept
    {
        if (ticks <= 0)
            return;

        cursorBlinkTick = (cursorBlinkTick + ticks) % kCursorBlinkPeriodTicks;
    }

    float PanelComponent::getCursorUnderlineAlpha() const noexcept
    {
        const float phase = static_cast<float> (cursorBlinkTick)
                            / static_cast<float> (kCursorBlinkPeriodTicks);
        return phase < 0.5f ? phase * 2.0f : 2.0f - phase * 2.0f;
    }

    void PanelComponent::drawPlaceholderContent (juce::Graphics& g)
    {
        const auto panel = getLocalBounds();
        const float ruleY = kTitleHeight + 1.0f + kContentTopPad;
        const auto panelW = static_cast<float> (panel.getWidth());
        const auto panelH = static_cast<float> (panel.getHeight());

        auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel());
        const auto textColour = laf != nullptr
                                  ? laf->findColour (SynthortionLookAndFeel::textColourId)
                                  : juce::Colour (0xFFFFFFFF);

        const float bodyHeight = juce::jmax (1.0f, panelH - ruleY - kPlaceholderBottomPad);
        const float bodyCentreY = ruleY + bodyHeight * 0.5f;

        {
            juce::Rectangle<float> labelArea (panel.getX(),
                                               bodyCentreY - kTitleHeight * 0.5f,
                                               panelW,
                                               kTitleHeight);
            g.setColour (textColour);
            g.setFont (headingFont);
            g.drawFittedText ("COMING SOON", labelArea.toNearestInt(),
                              juce::Justification::centred, 1);
        }

        const float alpha = getCursorUnderlineAlpha();
        if (alpha > 0.0f)
        {
            const int cursorX = juce::roundToInt ((panelW - kCursorWidth) * 0.5f);
            const int cursorY = juce::roundToInt (bodyCentreY + kTitleHeight * 0.5f + kCursorGap);
            g.setColour (textColour.withAlpha (alpha));
            g.fillRect (cursorX, cursorY, juce::roundToInt (kCursorWidth), juce::roundToInt (kCursorHeight));
        }
    }
}
