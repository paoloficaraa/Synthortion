#include "Synthortion/MeterComponent.h"

#include <array>
#include <cmath>

namespace synthortion
{
    namespace
    {
        const juce::Colour kWhite (0xFFFFFFFF);
        const juce::Colour kBlack (0xFF000000);

        const std::array<float, 3> kReferenceTickDb { -6.0f, -12.0f, -24.0f };
    }

    MeterComponent::MeterComponent (AnimationController& controller)
        : animationController (controller)
    {
        setOpaque (true);
    }

    MeterComponent::~MeterComponent()
    {
        if (decayAnimator.has_value() && ! decayAnimator->isComplete())
            animationController.removeAnimator (*decayAnimator);
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

        if (bypassed)
        {
            if (animatedPeakDb > kMinDb)
                startPeakDecay (animatedPeakDb, kBypassDecayMs);
        }
        else if (peakDb > animatedPeakDb)
        {
            animatedPeakDb = peakDb;
            startPeakDecay (animatedPeakDb, kNormalDecayMs);
        }

        repaint();
    }

    void MeterComponent::startPeakDecay (float fromDb, float durationMs)
    {
        if (decayAnimator.has_value())
        {
            animationController.removeAnimator (*decayAnimator);
            decayAnimator.reset();
        }

        decayAnimator = animationController.runAnimator (
            juce::ValueAnimatorBuilder()
                .withDurationMs (static_cast<double> (durationMs))
                .withEasing ([](float progress)
                             {
                                 return quantizeStepProgress (progress, kSegmentCount);
                             })
                .withValueChangedCallback (
                    [this, fromDb](float progress)
                    {
                        animatedPeakDb = fromDb + (kMinDb - fromDb) * progress;
                        repaint();
                    }));
    }

    float MeterComponent::quantizeStepProgress (float progress, int steps) noexcept
    {
        if (steps <= 0)
            return progress;

        const float clamped = juce::jlimit (0.0f, 1.0f, progress);
        const int stepIndex = juce::jlimit (0, steps, juce::roundToInt (clamped * static_cast<float> (steps)));
        return static_cast<float> (stepIndex) / static_cast<float> (steps);
    }

    int MeterComponent::dbToSegmentIndex (float db) const noexcept
    {
        const float t = juce::jlimit (0.0f, 1.0f, (db - kMinDb) / (kMaxDb - kMinDb));
        const float scaled = t * static_cast<float> (kSegmentCount);
        int index = static_cast<int> (std::ceil (scaled - 1.0e-5f));
        return juce::jlimit (0, kSegmentCount, index);
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
        const auto bounds = getLocalBounds();
        g.fillAll (kBlack);

        const int barW = bounds.getWidth() / 2;
        const int barX = bounds.getCentreX() - barW / 2;
        const int barY = bounds.getY();
        const int barH = bounds.getHeight();
        const int barBottom = barY + barH;

        const float segmentHeightF = static_cast<float> (barH) / static_cast<float> (kSegmentCount);

        const int litCount = bypassed ? 0 : dbToSegmentIndex (rmsDb);

        for (int i = 0; i < kSegmentCount; ++i)
        {
            const int segTop = juce::roundToInt (static_cast<float> (barBottom) - static_cast<float> (i + 1) * segmentHeightF);
            const int segBottom = juce::roundToInt (static_cast<float> (barBottom) - static_cast<float> (i) * segmentHeightF);
            g.setColour (i < litCount ? kWhite : kBlack);
            g.fillRect (barX, segTop, barW, segBottom - segTop);
        }

        const int peakSeg = dbToSegmentIndex (animatedPeakDb);
        if (peakSeg > 0)
        {
            const int markerY = juce::roundToInt (static_cast<float> (barBottom) - static_cast<float> (peakSeg) * segmentHeightF);
            g.setColour (kWhite);
            g.fillRect (barX, markerY, barW, 1);
        }

        g.setColour (kWhite);
        g.drawRect (barX, barY, barW, barH, 1);

        g.setColour (kBlack);
        for (float db : kReferenceTickDb)
        {
            const int tickY = juce::roundToInt (static_cast<float> (barBottom) - levelToHeight (db));
            g.fillRect (barX, tickY, 1, 1);
            g.fillRect (barX + barW - 1, tickY, 1, 1);
        }
    }
}
