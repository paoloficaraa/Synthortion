#include "Synthortion/MeterComponent.h"

namespace synthortion
{
    MeterComponent::MeterComponent()
    {
        setOpaque (true);
        startTimer (1000 / kTimerHz);
    }

    void MeterComponent::resized()
    {
    }

    void MeterComponent::setBypassed (bool b) noexcept
    {
        bypassed = b;
    }

    void MeterComponent::updateFromBuffer (const juce::AudioBuffer<float>& buffer)
    {
        const float rms = computeRMS (buffer);
        rmsDb = juce::Decibels::gainToDecibels (rms, kMinDb);

        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto minMax = buffer.findMinMax (ch, 0, buffer.getNumSamples());
            peak = std::max (peak, std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd())));
        }
        peakDb = juce::Decibels::gainToDecibels (peak, kMinDb);

        if (peakDb > peakHoldDb)
            peakHoldDb = peakDb;
    }

    void MeterComponent::timerCallback()
    {
        if (peakHoldDb > kMinDb)
            peakHoldDb -= kReleaseRate;

        if (bypassed)
        {
            rmsDb = kMinDb;
            peakDb = kMinDb;
            if (peakHoldDb > kMinDb)
                peakHoldDb -= kReleaseRate * 2.0f;
        }

        repaint();
    }

    float MeterComponent::computeRMS (const juce::AudioBuffer<float>& buffer) const
    {
        if (buffer.getNumSamples() == 0)
            return 0.0f;

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

        return static_cast<float> (std::sqrt (sum / static_cast<double> (count)));
    }

    float MeterComponent::levelToHeight (float db) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const float t = juce::jlimit (0.0f, 1.0f, (db - kMinDb) / (kMaxDb - kMinDb));
        return t * bounds.getHeight();
    }

    void MeterComponent::paint (juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();

        g.fillAll (juce::Colour (0xFFF5F0EB));

        const float barWidth = bounds.getWidth() * 0.5f;
        const float barX = bounds.getCentreX() - barWidth * 0.5f;

        // Background bar
        g.setColour (juce::Colour (0xFFE5E0DA));
        g.fillRoundedRectangle (barX, bounds.getY(), barWidth, bounds.getHeight(), 4.0f);

        // RMS bar
        const float rmsH = levelToHeight (rmsDb);
        juce::ColourGradient grad (
            juce::Colour (0xFF7C3AED), barX, bounds.getBottom(),
            juce::Colour (0xFFC4B5FD), barX, bounds.getY(),
            false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (barX, bounds.getBottom() - rmsH, barWidth, rmsH, 4.0f);

        // Peak hold marker
        const float peakH = levelToHeight (peakHoldDb);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        const float markerY = bounds.getBottom() - peakH;
        g.fillRect (barX - 2.0f, markerY, barWidth + 4.0f, 2.0f);

        // Reference ticks
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        for (float db : { -6.0f, -12.0f, -24.0f })
        {
            const float y = bounds.getBottom() - levelToHeight (db);
            g.drawLine (barX - 3.0f, y, barX + barWidth + 3.0f, y, 1.0f);
        }
    }
}
