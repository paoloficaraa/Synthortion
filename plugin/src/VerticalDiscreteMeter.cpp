#include "Synthortion/VerticalDiscreteMeter.h"
#include "Synthortion/SynthortionLookAndFeel.h"

namespace synthortion
{
    namespace Gui
    {
        // ============================================================================
        // Bulb Implementation
        // ============================================================================

        Bulb::Bulb(const juce::Colour &c) : colour(c)
        {
        }

        void Bulb::paint(juce::Graphics &g)
        {
            const auto bounds = getLocalBounds().toFloat().reduced(kBulbReduction);
            const auto side = juce::jmin(bounds.getWidth(), bounds.getHeight());
            const auto bulbFillBounds = juce::Rectangle<float>{bounds.getX(), bounds.getY(), side, side};

            if (isOn)
            {
                g.setColour(colour);
                g.fillEllipse(bulbFillBounds);

                juce::ColourGradient glowGradient{
                    colour.withAlpha(kOnAlpha),
                    bulbFillBounds.getCentre(),
                    colour.withAlpha(0.0f),
                    {},
                    true};
                g.setGradientFill(glowGradient);
                g.fillEllipse(bulbFillBounds.expanded(kBulbReduction * kGlowExpansion));
            }
            else
            {
                g.setColour(DARK_GREY.darker(kOffDarkenFactor));
                g.fillEllipse(bulbFillBounds);
            }

            g.setColour(juce::Colours::black.withAlpha(kBorderAlpha));
            g.drawEllipse(bulbFillBounds, kBulbBorderWidth);
        }

        void Bulb::setState(const bool state)
        {
            if (isOn != state)
            {
                isOn = state;
                repaint();
            }
        }

        // ============================================================================
        // VerticalDiscreteMeter Implementation
        // ============================================================================

        VerticalDiscreteMeter::VerticalDiscreteMeter(std::function<float()> valueFunction)
            : valueSupplier(std::move(valueFunction))
        {
            startTimerHz(kTimerHz);
        }

        float VerticalDiscreteMeter::getCurrentLevel() const
        {
            const float dbValue = valueSupplier();
            return juce::jlimit(0.0f, 1.0f, juce::jmap(dbValue, kMinDbLevel, kMaxDbLevel, 0.0f, 1.0f));
        }

        void VerticalDiscreteMeter::paint(juce::Graphics &g)
        {
            const int activeBulbs = static_cast<int>(cachedLevel * totalNumberOfBulbs);

            const auto slot = getLocalBounds().toFloat().reduced(kSlotReduction);
            g.setColour(DARK_GREY);
            g.fillRoundedRectangle(slot, kSlotCornerRadius);
            g.setColour(BLACK.withAlpha(kSlotBorderAlpha));
            g.drawRoundedRectangle(slot, kSlotCornerRadius, kSlotBorderWidth);

            for (int i = 0; i < totalNumberOfBulbs; ++i)
            {
                bulbs[i]->setState(i < activeBulbs);
            }
        }

        void VerticalDiscreteMeter::createBulbs()
        {
            const auto bounds = getLocalBounds().toFloat();

            juce::ColourGradient gradient{
                juce::Colour(kGradientBottomArgb),
                bounds.getBottomLeft(),
                juce::Colour(kGradientTopArgb),
                bounds.getTopLeft(),
                false};
            gradient.addColour(kGradientStop1, juce::Colour(kGradientMid1Argb));
            gradient.addColour(kGradientStop2, juce::Colour(kGradientMid2Argb));

            auto bulbBounds = getLocalBounds().reduced(kBulbAreaReduction);
            const auto bulbHeight = bulbBounds.getHeight() / totalNumberOfBulbs;
            const auto bulbWidth = juce::jmin(bulbBounds.getWidth(), bulbHeight - kBulbSpacing);
            
            bulbs.clear();
            bulbs.reserve(totalNumberOfBulbs);

            for (int i = 0; i < totalNumberOfBulbs; ++i)
            {
                const auto colourPosition = static_cast<double>(i) / static_cast<double>(totalNumberOfBulbs - 1);
                auto bulb = std::make_unique<Bulb>(gradient.getColourAtPosition(colourPosition));
                addAndMakeVisible(bulb.get());

                const auto bulbArea = bulbBounds.removeFromBottom(bulbHeight);
                const auto centeredBulb = juce::Rectangle<int>(0, 0, bulbWidth, bulbWidth)
                                            .withCentre(bulbArea.getCentre());
                bulb->setBounds(centeredBulb);
                bulbs.push_back(std::move(bulb));
            }
        }

        void VerticalDiscreteMeter::resized()
        {
            createBulbs();
        }

        void VerticalDiscreteMeter::timerCallback()
        {
            const float newLevel = getCurrentLevel();
            
            constexpr float kChangeThreshold = 0.01f;
            if (std::abs(newLevel - cachedLevel) > kChangeThreshold)
            {
                cachedLevel = newLevel;
                repaint();
            }
        }
    }
}
