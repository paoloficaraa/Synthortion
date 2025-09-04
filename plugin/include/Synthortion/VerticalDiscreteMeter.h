#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "Synthortion/SynthortionLookAndFeel.h"

namespace synthortion
{
    namespace Gui
    {
        class Bulb : public juce::Component
        {
        public:
            Bulb(const juce::Colour &c) : colour(c) {}

            void paint(juce::Graphics &g) override
            {
                const auto delta = 2.f;
                const auto bounds = getLocalBounds().toFloat().reduced(delta);
                const auto side = juce::jmin(bounds.getWidth(), bounds.getHeight());
                const auto bulbFillBounds = juce::Rectangle<float>{bounds.getX(), bounds.getY(), side, side};

                if (isOn)
                {
                    g.setColour(colour);
                    g.fillEllipse(bulbFillBounds);

                    // Glow viola più intenso quando acceso
                    g.setGradientFill(juce::ColourGradient{
                        colour.withAlpha(0.8f), // Glow più intenso (da 0.45f a 0.8f)
                        bulbFillBounds.getCentre(),
                        colour.withAlpha(0.f),
                        {},
                        true});
                    g.fillEllipse(bulbFillBounds.expanded(delta * 1.5f)); // Glow più ampio
                }
                else
                {
                    g.setColour(DARK_GREY.darker(0.3f)); // Pallini spenti più scuri per contrasto
                    g.fillEllipse(bulbFillBounds);
                }

                // Bordo nero
                g.setColour(juce::Colours::black.withAlpha(0.8f));
                g.drawEllipse(bulbFillBounds, 1.f);
            }

            void setState(const bool state)
            {
                if (isOn != state)
                {
                    isOn = state;
                    repaint();
                }
            }

        private:
            bool isOn = false;
            juce::Colour colour{};
        };

        class VerticalDiscreteMeter : public juce::Component, public juce::Timer
        {
        public:
            VerticalDiscreteMeter(std::function<float()> &&valueFunction) : valueSupplier(std::move(valueFunction))
            {
                startTimerHz(30); // 30 FPS per smooth updates
            }

            void paint(juce::Graphics &g) override
            {
                const auto level = juce::jlimit(0.f, 1.f, juce::jmap(valueSupplier(), -60.f, 6.f, 0.f, 1.f));

                // Calcola quanti pallini devono essere accesi
                const int activeBulbs = static_cast<int>(level * totalNumberOfBulbs);

                // Recessed slot background
                auto slot = getLocalBounds().toFloat().reduced(2.0f);
                g.setColour(DARK_GREY);
                g.fillRoundedRectangle(slot, 6.0f);
                g.setColour(BLACK.withAlpha(0.6f));
                g.drawRoundedRectangle(slot, 6.0f, 1.0f);

                for (auto i = 0; i < totalNumberOfBulbs; i++)
                {
                    if (i < activeBulbs)
                        bulbs[i]->setState(true);
                    else
                        bulbs[i]->setState(false);
                }
            }

            void resized() override
            {
                const auto bounds = getLocalBounds().toFloat();

                // Gradiente viola più vivace e visibile
                juce::ColourGradient gradient{
                    juce::Colour(0xFF4A148C), // Viola scuro in basso (più intenso)
                    bounds.getBottomLeft(),
                    juce::Colour(0xFFE040FB), // Viola magenta brillante in alto
                    bounds.getTopLeft(),
                    false};
                gradient.addColour(0.3, juce::Colour(0xFF7B1FA2)); // Viola medio
                gradient.addColour(0.7, juce::Colour(0xFFBA68C8)); // Viola chiaro

                auto bulbBounds = getLocalBounds().reduced(2); // Riduciamo meno per utilizzare meglio lo spazio
                const auto bulbHeight = bulbBounds.getHeight() / totalNumberOfBulbs;
                const auto bulbWidth = juce::jmin(bulbBounds.getWidth(), bulbHeight - 2); // Keep bulbs circular
                bulbs.clear();

                for (auto i = 0; i < totalNumberOfBulbs; i++)
                {
                    // I pallini partono dal basso (indice 0 = verde)
                    const auto colourPosition = static_cast<double>(i) / (totalNumberOfBulbs - 1);
                    auto bulb = std::make_unique<Bulb>(gradient.getColourAtPosition(colourPosition));
                    addAndMakeVisible(bulb.get());

                    // Center the bulb horizontally within its slot
                    auto bulbArea = bulbBounds.removeFromBottom(bulbHeight);
                    auto centeredBulb = juce::Rectangle<int>(0, 0, bulbWidth, bulbWidth)
                                            .withCentre(bulbArea.getCentre());
                    bulb->setBounds(centeredBulb);
                    bulbs.push_back(std::move(bulb));
                }
            }

            void timerCallback() override
            {
                repaint();
            }

        private:
            std::function<float()> valueSupplier;
            std::vector<std::unique_ptr<Bulb>> bulbs;
            const int totalNumberOfBulbs = 12; // Numero di pallini
        };
    }
}
