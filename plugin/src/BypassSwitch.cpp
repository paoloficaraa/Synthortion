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

    float BypassSwitch::getLedBrightness() const noexcept
    {
        return juce::jlimit (0.0f, 1.0f, 1.0f - animationProgress);
    }

    float BypassSwitch::getDepressOffset() const noexcept
    {
        return juce::jlimit (0.0f, kDepressPixels, (1.0f - animationProgress) * kDepressPixels);
    }

    juce::Rectangle<float> BypassSwitch::getBodyBounds() const noexcept
    {
        return getLocalBounds().toFloat().reduced (kBezelInset).translated (0.0f, getDepressOffset());
    }

    juce::Rectangle<float> BypassSwitch::getLedBounds() const noexcept
    {
        const auto body = getBodyBounds();
        const float cx = body.getCentreX();
        const float cy = body.getY() + kLedTopOffset;
        return juce::Rectangle<float> (cx - kLedRadius, cy - kLedRadius,
                                       kLedRadius * 2.0f, kLedRadius * 2.0f);
    }

    void BypassSwitch::paintButton (juce::Graphics& g, bool /*isMouseOver*/, bool /*isMouseDown*/)
    {
        const auto* laf = dynamic_cast<const SynthortionLookAndFeel*> (&getLookAndFeel());
        const auto getColour = [laf] (int colourId, juce::Colour defaultColour)
        {
            return laf != nullptr ? laf->findColour (colourId) : defaultColour;
        };

        const auto textColour = getColour (SynthortionLookAndFeel::textColourId, juce::Colour (0xFFFFFFFF));

        const auto body = getBodyBounds();
        const float ledBrightness = getLedBrightness();

        // 1 px outer bezel ring (lighter edge) fills the cap; inner layers overdraw.
        g.setColour (juce::Colour (0xFF6E6E70));
        g.fillRoundedRectangle (body, kButtonCornerRadius);

        // 1 px inner bezel ring (darker edge), 1 px inside the outer ring.
        g.setColour (juce::Colour (0xFF050505));
        g.fillRoundedRectangle (body.reduced (1.0f),
                                juce::jmax (0.0f, kButtonCornerRadius - 1.0f));

        // Vertical metallic gradient cap (charcoal top -> near-black bottom), 1 px inside the inner bezel.
        const auto cap = body.reduced (2.0f);
        const juce::ColourGradient capGrad (
            juce::Colour (0xFF3A3A3C), cap.getTopLeft(),
            juce::Colour (0xFF080808), cap.getBottomLeft(), false);
        g.setGradientFill (capGrad);
        g.fillRoundedRectangle (cap, juce::jmax (0.0f, kButtonCornerRadius - 2.0f));

        // Circular LED aperture riding on the cap.
        const auto led = getLedBounds();
        const auto ledCentre = led.getCentre();
        const float ledRadius = led.getWidth() * 0.5f;

        // Aperture recess (always visible): dark hole with a faint rim.
        g.setColour (juce::Colour (0xFF050505));
        g.fillEllipse (led);
        g.setColour (juce::Colour (0xFF4A4A4C));
        g.drawEllipse (led, 1.0f);

        // LED glow + halo, ramped by brightness (active = bright white + halo, bypassed = dark).
        if (ledBrightness > 0.0f)
        {
            const float haloRadius = ledRadius + 4.0f;
            g.setColour (juce::Colours::white.withAlpha (0.35f * ledBrightness));
            g.fillEllipse (juce::Rectangle<float> (ledCentre.x - haloRadius, ledCentre.y - haloRadius,
                                                   haloRadius * 2.0f, haloRadius * 2.0f));

            g.setColour (juce::Colours::white.withAlpha (ledBrightness));
            g.fillEllipse (led);
        }

        // BYPASS label, centred on the cap.
        g.setColour (textColour);
        g.setFont (juce::FontOptions().withName ("BebasNeue").withHeight (static_cast<float> (kBypassLabelHeight)).withStyle ("Regular").withKerningFactor (SynthortionLookAndFeel::kTightKerning));
        g.drawFittedText ("BYPASS", body.toNearestInt(),
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