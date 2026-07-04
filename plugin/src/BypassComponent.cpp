#include "Synthortion/BypassComponent.h"

namespace synthortion
{
    BypassComponent::BypassComponent()
    {
        setupButton();
    }

    BypassComponent::BypassComponent (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        setupButton();
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, bypassButton);
    }

    void BypassComponent::setupButton()
    {
        setOpaque (false);

        bypassButton.setButtonText ("BYPASS");
        bypassButton.setOpaque (false);
        bypassButton.setClickingTogglesState (true);
        bypassButton.onClick = [this] { repaint(); };

        addAndMakeVisible (bypassButton);
    }

    void BypassComponent::paint (juce::Graphics& /*g*/)
    {
    }

    void BypassComponent::resized()
    {
        bypassButton.setBounds (getLocalBounds());
    }

    bool BypassComponent::isLedOn() const noexcept
    {
        return bypassButton.getToggleState();
    }
}
