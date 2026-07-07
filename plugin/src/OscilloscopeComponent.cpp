#include "Synthortion/OscilloscopeComponent.h"

namespace synthortion
{
    OscilloscopeComponent::OscilloscopeComponent (AudioScopeRingBuffer& rb)
        : ringBuffer (rb)
    {
        setOpaque (true);
        for (auto& h : history)
            h.setSize (2, kWindowSize);

        scratchInput.clear();
        scratchOutput.clear();
        startTimer (1000 / kTimerHz);
    }

    void OscilloscopeComponent::resized()
    {
    }

    void OscilloscopeComponent::setBypassed (bool b) noexcept
    {
        bypassed = b;
    }

    void OscilloscopeComponent::timerCallback()
    {
        ringBuffer.readInput (scratchInput);
        ringBuffer.readOutput (scratchOutput);

        history[static_cast<size_t> (historyIndex)] = scratchOutput;
        historyIndex = (historyIndex + 1) % kHistoryFrames;

        breathPhase += 0.05f;
        if (breathPhase > juce::MathConstants<float>::twoPi)
            breathPhase -= juce::MathConstants<float>::twoPi;

        repaint();
    }

    void OscilloscopeComponent::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colour (0xFFF5F0EB));

        drawScanlines (g);

        if (! bypassed)
        {
            for (int i = 0; i < kHistoryFrames; ++i)
            {
                const int idx = (historyIndex + kHistoryFrames - 1 - i) % kHistoryFrames;
                const float alpha = 0.25f * (kHistoryFrames - i);
                drawTrace (g, history[static_cast<size_t> (idx)], juce::Colour (0xFF7C3AED), alpha);
            }

            drawTrace (g, scratchInput, juce::Colour (0xFF7C3AED).withAlpha (0.35f), 0.35f);
            drawTrace (g, scratchOutput, juce::Colour (0xFF7C3AED), 1.0f);
        }
        else
        {
            const float breath = 0.5f + 0.5f * std::sin (breathPhase);
            g.setColour (juce::Colour (0xFF7C3AED).withAlpha (0.15f * breath));
            const float y = getLocalBounds().toFloat().getCentreY();
            g.drawLine (0.0f, y, static_cast<float> (getWidth()), y, 1.5f);
        }
    }

    void OscilloscopeComponent::drawScanlines (juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.03f));

        for (float y = 0.0f; y < bounds.getHeight(); y += 4.0f)
            g.drawLine (0.0f, y, bounds.getWidth(), y, 1.0f);
    }

    void OscilloscopeComponent::drawTrace (juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                                           juce::Colour colour, float alpha)
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
            const float sample = (buffer.getSample (0, i) + buffer.getSample (1, i)) * 0.5f;
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
}
