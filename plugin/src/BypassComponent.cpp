#include "Synthortion/BypassComponent.h"
#include "Synthortion/GlitchOverlay.h"

namespace synthortion
{
    BypassComponent::BypassComponent()
        : bypassSwitch (nullptr)
    {
        setupButton();
    }

    BypassComponent::BypassComponent (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& paramId,
                                      AnimationController* animationController)
        : bypassSwitch (animationController)
    {
        setupButton();
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, bypassSwitch);
    }

    void BypassComponent::setupButton()
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

    bool BypassComponent::isBypassed() const noexcept
    {
        return bypassSwitch.isBypassed();
    }

    void BypassComponent::setGlitchOverlay (GlitchOverlay* overlay) noexcept
    {
        bypassSwitch.setGlitchOverlay (overlay);
    }
}
