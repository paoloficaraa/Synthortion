#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/BypassSwitch.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    class GlitchOverlay;

    /** Top bar containing the brutalist Block-style bypass toggle. Wraps the
        BypassSwitch ButtonAttachment for PLUGIN_BYPASS; the actual drawing and
        state-transition glitch burst live on the BypassSwitch itself.
    */
    class BypassComponent final : public juce::Component
    {
    public:
        BypassComponent();
        explicit BypassComponent (juce::AudioProcessorValueTreeState& apvts,
                                  const juce::String& paramId = "PLUGIN_BYPASS",
                                  AnimationController* animationController = nullptr);

        void paint (juce::Graphics& g) override;
        void resized() override;

        bool isBypassed() const noexcept;
        juce::Button& getToggleButton() noexcept { return bypassSwitch; }
        BypassSwitch& getBypassSwitch() noexcept { return bypassSwitch; }

        void setGlitchOverlay (GlitchOverlay* overlay) noexcept;

    private:
        void setupButton();

        BypassSwitch bypassSwitch;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassComponent)
    };
}
