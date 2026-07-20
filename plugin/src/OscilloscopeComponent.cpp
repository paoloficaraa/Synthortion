#include "Synthortion/OscilloscopeComponent.h"

namespace synthortion
{
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
        g.fillAll (juce::Colour (kSubstrateArgb));

        if (silent)
        {
            drawBreathLine (g);
            return;
        }

        if (! bypassed)
        {
            drawGhostTrails (g);
            drawTraceTriplet (g, scratchInput, true, 1.0f, 1.0f);
            drawTraceTriplet (g, scratchOutput, false, 1.0f, 1.0f);
        }
        else
        {
            drawTraceTriplet (g, scratchOutput, false, flatlineAmplitude, 1.0f);
        }

        if (glitchOverlay != nullptr)
            glitchOverlay->drawSweep (g, getLocalBounds(), glitchOverlay->getSweepPosition());
    }

    void OscilloscopeComponent::drawTraceTriplet (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                                                   bool oddRows, float amplitudeScale, float alpha)
    {
        if (glitchOverlay != nullptr)
        {
            glitchOverlay->drawTriplet (g, [this, &g, &buffer, oddRows, amplitudeScale, alpha]
                                               (juce::Point<float> offset)
                                               {
                                                   drawPixelatedTrace (g, buffer, oddRows, offset.x,
                                                                       amplitudeScale, alpha);
                                               });
        }
        else
        {
            drawPixelatedTrace (g, buffer, oddRows, 0.0f, amplitudeScale, alpha);
        }
    }

    void OscilloscopeComponent::drawPixelatedTrace (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                                                     bool oddRows, float xOffset, float amplitudeScale, float alpha)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples < 2)
            return;

        const int width = getWidth();
        const int height = getHeight();
        if (width <= 0 || height <= 0)
            return;

        const int midY = height / 2;
        const float gain = static_cast<float> (height) * 0.35f;
        const float stepX = static_cast<float> (width) / static_cast<float> (numSamples - 1);
        const int xOff = static_cast<int> (std::round (xOffset));
        const int parity = oddRows ? 1 : 0;

        g.setColour (juce::Colour (kTraceArgb).withAlpha (alpha));

        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = (buffer.getSample (0, i) + buffer.getSample (1, i)) * 0.5f * amplitudeScale;
            const int x = static_cast<int> (std::round (static_cast<float> (i) * stepX)) + xOff;
            const int waveY = static_cast<int> (std::round (static_cast<float> (midY) - sample * gain));
            const int yTop = juce::jmin (midY, waveY);
            const int yBottom = juce::jmax (midY, waveY);

            for (int y = yTop; y <= yBottom; ++y)
            {
                if (y < 0 || y >= height)
                    continue;

                if (y % 2 == parity)
                    g.fillRect (x, y, 1, 1);
            }
        }
    }

    void OscilloscopeComponent::drawGhostTrails (juce::Graphics& g)
    {
        for (int i = 0; i < kHistoryFrames; ++i)
        {
            const int idx = (historyIndex + kHistoryFrames - 1 - i) % kHistoryFrames;
            drawPixelatedTrace (g, history[static_cast<size_t> (idx)], false,
                                0.0f, 1.0f, kGhostAlphas[static_cast<size_t> (i)]);
        }
    }

    void OscilloscopeComponent::drawBreathLine (juce::Graphics& g)
    {
        const bool on = static_cast<int> (breathPhase) % 2 == 0;
        if (! on)
            return;

        g.setColour (juce::Colour (kTraceArgb));
        const int y = getHeight() / 2;
        g.drawHorizontalLine (y, 0.0f, static_cast<float> (getWidth()));
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
