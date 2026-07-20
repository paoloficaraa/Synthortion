#include "Synthortion/PanelComponent.h"

namespace synthortion
{
    namespace
    {
        // Live pixels between the panel rule and the COMING SOON label, and
        // between the label and the glitch motion row, expressed in CSS px.
        constexpr float kContentTopPad = 6.0f;
        constexpr float kPlaceholderMotionMargin = 16.0f;
    }
    PanelComponent::PanelComponent (const juce::String& panelTitle, const juce::Colour& backgroundColour)
        : title (panelTitle),
          bgColour (backgroundColour),
          headingFont (juce::FontOptions().withName("BebasNeue").withHeight(22.0f).withStyle("Regular"))
    {
        setOpaque (true);
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
                auto labelArea = getLocalBounds().toFloat().removeFromTop (22.0f).reduced (8.0f, 0.0f);
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

    void PanelComponent::drawPlaceholderContent (juce::Graphics& g)
    {
        // Layout below the panel title row (0..22 px) and its 1 px rule:
        // ── COMING SOON label centred over the content,
        // ── 2 px-tall drift band drifting across the panel width,
        // ── 4x4 px flicker block beneath the band.
        const auto panel = getLocalBounds();
        const float ruleY = 22.0f + 1.0f + kContentTopPad;
        const auto panelW = static_cast<float> (panel.getWidth());
        const auto panelH = static_cast<float> (panel.getHeight());

        auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel());
        const auto textColour = laf != nullptr
                                  ? laf->findColour (SynthortionLookAndFeel::textColourId)
                                  : juce::Colour (0xFFFFFFFF);

        // Single-pass BebasNeue 22 pt #FFF COMING SOON label centred below the rule.
        {
            juce::Rectangle<float> labelArea (panel.getX(), ruleY,
                                              panelW, juce::jmax (1.0f, panelH - ruleY - kPlaceholderMotionMargin));
            g.setColour (textColour);
            g.setFont (headingFont);
            g.drawFittedText ("COMING SOON", labelArea.toNearestInt(),
                              juce::Justification::centred, 1);
        }

        // Drift band + flicker block read their per-frame state from
        // GlitchOverlay, advanced transitively by PluginEditor::timerCallback.
        // No overlay attached (unit tests) -> only the centred label renders.
        if (glitchOverlay != nullptr)
        {
            const float motionY = panelH - kPlaceholderMotionMargin;
            const juce::Rectangle<int> bandArea (panel.getX(),
                                                 static_cast<int> (motionY),
                                                 panel.getWidth(), 2);
            glitchOverlay->drawHorizontalBand (g, bandArea);

            const juce::Rectangle<int> flickerArea (panel.getCentreX() - 2,
                                                    static_cast<int> (motionY) + 4,
                                                    4, 4);
            glitchOverlay->drawFlickerBlock (g, flickerArea);
        }
    }
}
