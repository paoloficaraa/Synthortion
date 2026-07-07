#pragma once

#include "Synthortion/AnimationController.h"
#include "Synthortion/BypassSwitch.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Transparent top bar containing the spring-animated bypass toggle and its LED indicator. */
    class BypassComponent final : public juce::Component
    {
    public:
        BypassComponent();
        explicit BypassComponent (juce::AudioProcessorValueTreeState& apvts,
                                  const juce::String& paramId = "PLUGIN_BYPASS",
                                  AnimationController* animationController = nullptr);

        void paint (juce::Graphics& g) override;
        void resized() override;

        bool isLedOn() const noexcept;
        juce::Button& getToggleButton() noexcept { return bypassSwitch; }

    private:
        void setupButton (AnimationController* animationController);

        BypassSwitch bypassSwitch;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassComponent)
    };
}
