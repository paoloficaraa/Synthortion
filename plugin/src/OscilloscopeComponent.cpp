#include "Synthortion/OscilloscopeComponent.h"

namespace synthortion
{
    namespace
    {
        const juce::Colour kBackground (0xFFF5F0EB);
        const juce::Colour kTraceColour (0xFF7C3AED);
        const juce::Colour kTraceGrey (0xFF6B6570);
        const juce::Colour kScanlineColour = juce::Colours::black.withAlpha (0.05f);
    }

    OscilloscopeComponent::OscilloscopeComponent (AudioScopeRingBuffer& rb,
                                                  AnimationController* animationController)
        : ringBuffer (rb),
          controller (animationController)
    {
        setOpaque (true);

        for (auto& h : history)
            h.setSize (2, kWindowSize);

        scratchInput.clear();
        scratchOutput.clear();

        startAnimation();
    }

    OscilloscopeComponent::~OscilloscopeComponent()
    {
        if (controller != nullptr && animationRunning && currentAnimator.has_value())
            controller->removeAnimator (*currentAnimator);
    }

    void OscilloscopeComponent::resized()
    {
    }

    void OscilloscopeComponent::setBypassed (bool b) noexcept
    {
        bypassed = b;
    }

    void OscilloscopeComponent::startAnimation()
    {
        if (controller == nullptr)
            return;

        currentAnimator = controller->runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (kFrameDurationMs)
                .withEasing (juce::Easings::createLinear())
                .runningInfinitely()
                .withValueChangedCallback ([this] (float)
                                           {
                                               refresh();
                                           }));

        animationRunning = true;
    }

    void OscilloscopeComponent::refresh()
    {
        updateFrame();
        repaint();
    }

    void OscilloscopeComponent::updateFrame()
    {
        ringBuffer.readInput (scratchInput);
        ringBuffer.readOutput (scratchOutput);

        silent = detectSilence (scratchInput) && detectSilence (scratchOutput);

        if (! bypassed)
            pushHistory (scratchOutput);

        breathPhase += kBreathIncrement;
        if (breathPhase > juce::MathConstants<float>::twoPi)
            breathPhase -= juce::MathConstants<float>::twoPi;

        if (bypassed)
        {
            flatlineAmplitude = flatlineAmplitude * kFlatlineDecay;
        }
        else
        {
            flatlineAmplitude = 1.0f;
        }
    }

    void OscilloscopeComponent::paint (juce::Graphics& g)
    {
        g.fillAll (kBackground);

        drawScanlines (g);

        const float mix = controller != nullptr ? controller->getBypassMix() : 0.0f;

        if (silent)
            drawBreathLine (g);

        if (bypassed)
        {
            const float outputAlpha = 0.9f * (1.0f - mix * 0.5f);
            drawTrace (g, scratchOutput, kTraceGrey, outputAlpha, flatlineAmplitude);
        }
        else if (! silent)
        {
            drawGhostTrails (g);
            drawTrace (g, scratchInput, kTraceColour, 0.45f * (1.0f - mix * 0.5f), 1.0f);
            drawTrace (g, scratchOutput, kTraceColour, 1.0f * (1.0f - mix * 0.3f), 1.0f);
        }
    }

    void OscilloscopeComponent::drawScanlines (juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        g.setColour (kScanlineColour);

        for (float y = 0.0f; y < bounds.getHeight(); y += 4.0f)
            g.drawLine (0.0f, y, bounds.getWidth(), y, 1.0f);
    }

    void OscilloscopeComponent::drawTrace (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                                           juce::Colour colour, float alpha, float amplitudeScale)
    {
        const auto bounds = getLocalBounds().toFloat();
        const int numSamples = buffer.getNumSamples();
        if (numSamples < 2)
            return;

        juce::Path path;
        const float midY = bounds.getCentreY();
        const float gain = bounds.getHeight() * 0.35f;
        const float stepX = bounds.getWidth() / static_cast<float> (numSamples - 1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = (buffer.getSample (0, i) + buffer.getSample (1, i)) * 0.5f * amplitudeScale;
            const float x = static_cast<float> (i) * stepX;
            const float y = midY - sample * gain;

            if (i == 0)
                path.startNewSubPath (x, y);
            else
                path.lineTo (x, y);
        }

        g.setColour (colour.withAlpha (alpha));
        g.strokePath (path, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
    }

    void OscilloscopeComponent::drawGhostTrails (juce::Graphics& g)
    {
        for (int i = 0; i < kHistoryFrames; ++i)
        {
            const int idx = (historyIndex + kHistoryFrames - 1 - i) % kHistoryFrames;
            const float alpha = 0.18f * static_cast<float> (kHistoryFrames - i);
            drawTrace (g, history[static_cast<size_t> (idx)], kTraceColour, alpha, 1.0f);
        }
    }

    void OscilloscopeComponent::drawBreathLine (juce::Graphics& g)
    {
        const float breath = 0.5f + 0.5f * std::sin (breathPhase);
        g.setColour (kTraceColour.withAlpha (0.15f * breath));
        const float y = getLocalBounds().toFloat().getCentreY();
        g.drawLine (0.0f, y, static_cast<float> (getWidth()), y, 1.5f);
    }

    bool OscilloscopeComponent::detectSilence (const juce::AudioBuffer<float>& buffer) const noexcept
    {
        if (buffer.getNumSamples() == 0)
            return true;

        double sum = 0.0;
        int count = 0;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                sum += static_cast<double> (data[i]) * static_cast<double> (data[i]);
                ++count;
            }
        }

        if (count == 0)
            return true;

        const float rms = static_cast<float> (std::sqrt (sum / static_cast<double> (count)));
        return rms < kSilenceThreshold;
    }

    void OscilloscopeComponent::pushHistory (const juce::AudioBuffer<float>& buffer)
    {
        auto& dest = history[static_cast<size_t> (historyIndex)];
        const int samplesToCopy = std::min (buffer.getNumSamples(), dest.getNumSamples());

        for (int ch = 0; ch < std::min (buffer.getNumChannels(), dest.getNumChannels()); ++ch)
            dest.copyFrom (ch, 0, buffer, ch, 0, samplesToCopy);

        historyIndex = (historyIndex + 1) % kHistoryFrames;
    }
}
