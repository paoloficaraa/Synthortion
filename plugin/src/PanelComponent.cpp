#include "Synthortion/PanelComponent.h"

namespace synthortion
{
    PanelComponent::PanelComponent (const juce::String& panelTitle, const juce::Colour& backgroundColour)
        : title (panelTitle),
          bgColour (backgroundColour),
          headingFont (juce::FontOptions().withName("BebasNeue").withHeight(18.0f).withStyle("Regular"))
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
            g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 6.0f);

            if (title.isNotEmpty())
            {
                auto labelArea = getLocalBounds().toFloat().removeFromTop (22.0f).reduced (8.0f, 0.0f);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
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
        auto bounds = getLocalBounds().toFloat().reduced (8.0f);
        bounds.removeFromTop (22.0f);

        auto* laf = dynamic_cast<SynthortionLookAndFeel*> (&getLookAndFeel());
        const auto textDark = laf != nullptr
                                  ? laf->findColour (SynthortionLookAndFeel::creamTextColourId)
                                  : juce::Colour (0xFF2E2A33);
        const auto cream = laf != nullptr
                               ? laf->findColour (SynthortionLookAndFeel::backgroundColourId)
                               : juce::Colour (0xFFF5F0EB);

        const auto placeholderFont = headingFont.withHeight (16.0f);
        const auto textArea = bounds.toNearestInt();

        // Embossed shadow
        g.setColour (textDark.withAlpha (0.2f));
        g.setFont (placeholderFont);
        g.drawFittedText ("COMING SOON", textArea.translated (1, 1), juce::Justification::centred, 1);

        // Embossed highlight
        g.setColour (cream.brighter (0.25f).withAlpha (0.45f));
        g.drawFittedText ("COMING SOON", textArea.translated (-1, -1), juce::Justification::centred, 1);

        // Main label
        g.setColour (textDark.withAlpha (0.55f));
        g.drawFittedText ("COMING SOON", textArea, juce::Justification::centred, 1);

        // Stylised ellipsis icon
        g.setColour (textDark.withAlpha (0.35f));
        const float dotY = bounds.getCentreY() + 18.0f;
        for (int i = -1; i <= 1; ++i)
        {
            const float x = bounds.getCentreX() + static_cast<float> (i) * 10.0f;
            g.fillEllipse (x - 2.0f, dotY - 2.0f, 4.0f, 4.0f);
        }
    }
}
