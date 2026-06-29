#include "Synthortion/PanelComponent.h"
#include "Synthortion/AnalogLookAndFeel.h"

namespace synthortion
{

PanelComponent::PanelComponent(const juce::String& title, bool isRecessed)
    : panelTitle(title), recessed(isRecessed)
{
    setOpaque(true);
}

void PanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    auto* lf = dynamic_cast<AnalogLookAndFeel*>(&getLookAndFeel());
    if (lf != nullptr)
    {
        lf->drawPanelBackground(g, bounds, recessed, panelTitle);
    }
    else
    {
        g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));
    }
}

void PanelComponent::resized()
{
}

void PanelComponent::setTitle(const juce::String& newTitle)
{
    panelTitle = newTitle;
    repaint();
}

} // namespace synthortion
