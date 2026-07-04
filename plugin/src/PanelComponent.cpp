#include "Synthortion/PanelComponent.h"

namespace synthortion
{
    PanelComponent::PanelComponent (const juce::String& panelTitle, const juce::Colour& backgroundColour)
        : title (panelTitle), bgColour (backgroundColour)
    {
        setOpaque (true);
    }

    void PanelComponent::paint (juce::Graphics& g)
    {
        g.fillAll (bgColour);

        if (auto* laf = dynamic_cast<AnalogLookAndFeel*> (&getLookAndFeel()))
        {
            laf->drawPanelBackground (g, getLocalBounds(), false, title, bgColour);
        }
        else
        {
            g.setColour (bgColour);
            g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 6.0f);

            if (title.isNotEmpty())
            {
                auto labelArea = getLocalBounds().toFloat().removeFromTop (16.0f).reduced (8.0f, 0.0f);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                g.setFont (juce::Font (juce::FontOptions (9.0f).withStyle ("Bold")));
                g.drawFittedText (title, labelArea.toNearestInt(), juce::Justification::centredLeft, 1);
            }
        }
    }

    void PanelComponent::resized()
    {
    }
}
