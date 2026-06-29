#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{

class PanelComponent : public juce::Component
{
public:
    PanelComponent(const juce::String& title, bool isRecessed = false);

    void paint(juce::Graphics&) override;
    void resized() override;

    void setTitle(const juce::String& newTitle);

private:
    juce::String panelTitle;
    bool recessed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanelComponent)
};

} // namespace synthortion
