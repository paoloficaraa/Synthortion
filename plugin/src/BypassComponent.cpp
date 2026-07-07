#include "Synthortion/BypassComponent.h"

namespace synthortion
{
    BypassComponent::BypassComponent()
        : bypassSwitch (nullptr)
    {
        setupButton (nullptr);
    }

    BypassComponent::BypassComponent (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& paramId,
                                      AnimationController* animationController)
        : bypassSwitch (animationController)
    {
        setupButton (animationController);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, bypassSwitch);
    }

    void BypassComponent::setupButton (AnimationController* /*animationController*/)
    {
        setOpaque (false);
        bypassSwitch.setOpaque (false);
        addAndMakeVisible (bypassSwitch);
    }

    void BypassComponent::paint (juce::Graphics& /*g*/)
    {
    }

    void BypassComponent::resized()
    {
        bypassSwitch.setBounds (getLocalBounds());
    }

    bool BypassComponent::isLedOn() const noexcept
    {
        return bypassSwitch.isLedOn();
    }
}
