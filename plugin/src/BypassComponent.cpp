#include "Synthortion/BypassComponent.h"

namespace synthortion
{

namespace
{
    const juce::Colour LED_ON(0xFF00F5D4);
    const juce::Colour LED_OFF(0xFF1A1A1A);
    const juce::Colour CREAM(0xFFE0E0E0);
}

BypassComponent::BypassComponent(juce::AudioProcessorValueTreeState& apvts)
{
    bypassLabel.setText("BYPASS", juce::dontSendNotification);
    bypassLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bypassLabel);

    bypassButton.setColour(juce::ToggleButton::textColourId, CREAM);
    bypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF7C3AED));
    addAndMakeVisible(bypassButton);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "PLUGIN_BYPASS", bypassButton);
}

void BypassComponent::paint(juce::Graphics& g)
{
    auto ledBounds = juce::Rectangle<int>(getWidth() / 2 - 32, getHeight() / 2 - 4, 8, 8);
    auto ledColour = getLedColour();

    g.setColour(ledColour);
    g.fillEllipse(ledBounds.toFloat());

    if (bypassButton.getToggleState())
    {
        g.setColour(ledColour.withAlpha(0.3f));
        g.fillEllipse(ledBounds.expanded(2).toFloat());
    }
}

void BypassComponent::resized()
{
    auto bounds = getLocalBounds();

    auto labelBounds = bounds.removeFromLeft(60);
    bypassLabel.setBounds(labelBounds);

    auto btnBounds = bounds.removeFromLeft(40);
    bypassButton.setBounds(btnBounds);
}

juce::Colour BypassComponent::getLedColour() const
{
    return bypassButton.getToggleState() ? LED_ON : LED_OFF;
}

} // namespace synthortion
