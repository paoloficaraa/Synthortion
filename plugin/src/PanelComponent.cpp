#include "Synthortion/PanelComponent.h"

namespace synthortion
{
    namespace
    {
        constexpr float kTitleHeight = 22.0f;
        constexpr float kContentTopPad = 6.0f;
        constexpr float kPlaceholderMotionMargin = 16.0f;
    }
    PanelComponent::PanelComponent (const juce::String& panelTitle, const juce::Colour& backgroundColour)
        : title (panelTitle),
          bgColour (backgroundColour),
          headingFont (juce::FontOptions().withName("BebasNeue").withHeight(kTitleHeight).withStyle("Regular").withKerningFactor(SynthortionLookAndFeel::kTightKerning))
    {
        setOpaque (true);
    }

    void PanelComponent::paint (juce::Graphics& g)
    {
        g.fillAll (bgColour);

        if (auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel()))
        {
            laf->drawPanelBackground (g, getLocalBounds(), false, title, bgColour,
                                      headingFont.getHeight(), headingInset);
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
    }

    void PanelComponent::setHeadingStyle (float fontHeight, float inset) noexcept
    {
        headingFont = juce::Font (juce::FontOptions().withName("BebasNeue").withHeight(fontHeight)
                                  .withStyle("Regular").withKerningFactor(SynthortionLookAndFeel::kTightKerning));
        headingInset = inset;
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

        {
            juce::Rectangle<float> labelArea (panel.getX(), ruleY,
                                              panelW, juce::jmax (1.0f, panelH - ruleY - kPlaceholderMotionMargin));
            g.setColour (textColour);
            g.setFont (headingFont);
            g.drawFittedText ("COMING SOON", labelArea.toNearestInt(),
                              juce::Justification::centred, 1);
        }

        if (glitchOverlay != nullptr)
        {
            const float motionY = panelH - kPlaceholderMotionMargin;
            const juce::Rectangle<int> bandArea (panel.getX(),
                                                 static_cast<int> (motionY),
                                                 panel.getWidth(),
                                                 GlitchOverlay::driftBandHeight());
            glitchOverlay->drawHorizontalBand (g, bandArea);

            const int halfBlock = GlitchOverlay::flickerBlockSize() / 2;
            const int blockSize = GlitchOverlay::flickerBlockSize();
            const juce::Rectangle<int> flickerArea (panel.getCentreX() - halfBlock,
                                                    static_cast<int> (motionY) + blockSize,
                                                    blockSize, blockSize);
            glitchOverlay->drawFlickerBlock (g, flickerArea);
        }
    }
}
