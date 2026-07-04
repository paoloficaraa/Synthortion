#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    /** Transparent top bar containing the bypass toggle and its LED indicator. */
    class BypassComponent final : public juce::Component
    {
    public:
        BypassComponent();
        explicit BypassComponent (juce::AudioProcessorValueTreeState& apvts,
                                  const juce::String& paramId = "PLUGIN_BYPASS");

        void paint (juce::Graphics& g) override;
        void resized() override;

        bool isLedOn() const noexcept;
        juce::ToggleButton& getToggleButton() noexcept { return bypassButton; }

    private:
        void setupButton();

        juce::ToggleButton bypassButton;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassComponent)
    };
}
