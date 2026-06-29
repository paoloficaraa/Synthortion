#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace synthortion
{

class BypassComponent : public juce::Component
{
public:
    BypassComponent(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::ToggleButton bypassButton;
    juce::Label bypassLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    juce::Colour getLedColour() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BypassComponent)
};

} // namespace synthortion
