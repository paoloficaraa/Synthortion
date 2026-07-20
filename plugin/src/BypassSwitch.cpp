#include "Synthortion/BypassSwitch.h"
#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/GlitchOverlay.h"

namespace synthortion
{
    BypassSwitch::BypassSwitch (AnimationController* animationController)
        : juce::Button ("BYPASS"),
          controller (animationController)
    {
        setOpaque (false);
        setClickingTogglesState (true);
    }

    BypassSwitch::~BypassSwitch()
    {
        if (controller != nullptr && animationRunning && currentAnimator.has_value())
            controller->removeAnimator (*currentAnimator);
    }

    bool BypassSwitch::isBypassed() const noexcept
    {
        return getToggleState();
    }

    void BypassSwitch::paintButton (juce::Graphics& g, bool /*isMouseOver*/, bool /*isMouseDown*/)
    {
        const auto bounds = getLocalBounds().toFloat();
        const bool renderBypassed = animationProgress >= 0.5f;

        const auto* laf = dynamic_cast<const SynthortionLookAndFeel*> (&getLookAndFeel());
        const auto getColour = [laf] (int colourId, juce::Colour defaultColour)
        {
            return laf != nullptr ? laf->findColour (colourId) : defaultColour;
        };

        const auto backgroundColour = getColour (SynthortionLookAndFeel::backgroundColourId, juce::Colour (0xFF000000));
        const auto accentColour = getColour (SynthortionLookAndFeel::accentColourId, juce::Colour (0xFFFFFFFF));

        const auto blockColour = renderBypassed ? backgroundColour : accentColour;
        const auto labelColour = renderBypassed ? accentColour : backgroundColour;

        g.setColour (blockColour);
        g.fillRect (bounds);

        if (renderBypassed)
        {
            g.setColour (accentColour);
            g.drawRect (bounds, 1.0f);
        }

        g.setColour (labelColour);
        g.setFont (juce::FontOptions().withName ("BebasNeue").withHeight (static_cast<float> (kBypassLabelHeight)).withStyle ("Regular"));
        g.drawFittedText ("BYPASS", bounds.toNearestInt(),
                          juce::Justification::centred, 1);
    }

    void BypassSwitch::resized()
    {
        repaint();
    }

    void BypassSwitch::buttonStateChanged()
    {
        Button::buttonStateChanged();
        startAnimation();
    }

    void BypassSwitch::startAnimation()
    {
        const float target = getToggleState() ? 1.0f : 0.0f;

        if (std::abs (target - animationProgress) < 0.001f)
            return;

        if (glitchOverlay != nullptr)
            glitchOverlay->triggerBypassSlices();

        if (controller == nullptr)
        {
            animationProgress = target;
            repaint();
            return;
        }

        if (animationRunning && currentAnimator.has_value())
            controller->removeAnimator (*currentAnimator);

        animationRunning = false;
        animationStart = animationProgress;
        animationTarget = target;

        currentAnimator = controller->runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (kAnimationDurationMs)
                .withEasing ([] (float progress)
                             {
                                 return AnimatedKnob::quantizeStepProgress (progress, BypassSwitch::kBypassSteps);
                             })
                .withValueChangedCallback ([this] (float t)
                                           {
                                               animationProgress = juce::jlimit (
                                                   0.0f, 1.0f, animationStart + (animationTarget - animationStart) * t);
                                               repaint();
                                           })
                .withOnCompleteCallback ([this]
                                         {
                                             animationRunning = false;
                                         }));

        animationRunning = true;
    }
}