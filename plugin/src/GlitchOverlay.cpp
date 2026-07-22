#include "Synthortion/GlitchOverlay.h"

namespace synthortion
{
    GlitchOverlay::GlitchOverlay()
    {
        regenerateGrainFrames();
        rerollDeadPixels (lastDeadPixelBounds);
        ticksToNextFlicker = scheduleNextFlickerInterval();
    }

    void GlitchOverlay::tick()
    {
        grainFrameIndex = (grainFrameIndex + 1) % kGrainFrames;
        ++frameTickCount;

        if (++ticksSinceReroll >= kDeadPixelRerollTicks)
        {
            ticksSinceReroll = 0;
            rerollDeadPixels (lastDeadPixelBounds);
            ++deadPixelRerollCount;
        }

        if (bypassSliceActive && ++bypassSliceElapsedTicks >= kBypassSliceDurationTicks)
            bypassSliceActive = false;

        if (bootBurstActive && ++bootBurstElapsedTicks >= kBootBurstDurationTicks)
            bootBurstActive = false;

        if (flickerBurstActive)
        {
            const int duration = bypassAmplified ? kFlickerBypassDurationTicks : kFlickerNormalDurationTicks;
            if (++flickerBurstElapsedTicks >= duration)
            {
                flickerBurstActive = false;
                ticksToNextFlicker = scheduleNextFlickerInterval();
            }
        }
        else if (--ticksToNextFlicker <= 0)
        {
            triggerFlickerBurst();
        }
    }

    void GlitchOverlay::triggerBypassSlices()
    {
        bypassSliceBands[0] = { 0.25f, 2, -1, 2 };
        bypassSliceBands[1] = { 0.50f, 2, +1, 3 };
        bypassSliceBands[2] = { 0.75f, 2, -1, 1 };

        bypassSliceActive = true;
        bypassSliceElapsedTicks = 0;
    }

    void GlitchOverlay::drawBypassSlices (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (! bypassSliceActive || bounds.isEmpty())
            return;

        const float rawProgress = static_cast<float> (bypassSliceElapsedTicks)
                                  / static_cast<float> (kBypassSliceDurationTicks);
        const int stepIndex = juce::jlimit (0, kBypassSliceSteps,
                                             juce::roundToInt (rawProgress * static_cast<float> (kBypassSliceSteps)));
        const float progress = static_cast<float> (stepIndex) / static_cast<float> (kBypassSliceSteps);

        g.setColour (juce::Colour (kWhiteArgb));

        for (const auto& band : bypassSliceBands)
        {
            const int y = bounds.getY() + juce::roundToInt (band.yFrac * static_cast<float> (bounds.getHeight()));
            const int x = bounds.getX() + juce::roundToInt (static_cast<float> (band.shiftPx)
                                                              * static_cast<float> (band.shiftDir)
                                                              * progress);
            g.fillRect (x, y, bounds.getWidth(), band.thickness);
        }
    }

    void GlitchOverlay::triggerBootBurst()
    {
        if (bootBurstFired)
            return;

        bootBurstFired = true;

        for (auto& band : bootBurstBands)
        {
            band.yFrac = random.nextFloat();
            band.thickness = 1 + random.nextInt (3);
            band.shiftDir = (random.nextInt (2) == 0) ? -1 : +1;
            band.shiftPx = 1 + random.nextInt (4);
        }

        for (auto& pixel : bootBurstDeadPixels)
        {
            pixel = juce::Point<float> { random.nextFloat(), random.nextFloat() };
        }

        bootBurstActive = true;
        bootBurstElapsedTicks = 0;
    }

    float GlitchOverlay::getBootBurstProgress() const noexcept
    {
        if (! bootBurstActive)
            return 0.0f;

        return juce::jlimit (0.0f, 1.0f,
                             static_cast<float> (bootBurstElapsedTicks)
                                 / static_cast<float> (kBootBurstDurationTicks));
    }

    void GlitchOverlay::drawBootBurst (juce::Graphics& g, juce::Rectangle<int> bounds, float progress)
    {
        if (! bootBurstActive || bounds.isEmpty())
            return;

        const float clamped = juce::jlimit (0.0f, 1.0f, progress);
        const float flashEnd = static_cast<float> (kBootBurstFlashTicks) / static_cast<float> (kBootBurstDurationTicks);

        if (clamped < flashEnd)
        {
            g.setColour (juce::Colour (kWhiteArgb));
            g.fillRect (bounds);
        }
        else
        {
            const float fadeProgress = (clamped - flashEnd) / (1.0f - flashEnd);
            const int alphaInt = juce::roundToInt ((1.0f - fadeProgress) * 255.0f);
            const auto alpha = static_cast<juce::uint8> (juce::jlimit (0, 255, alphaInt));
            g.setColour (juce::Colour (kWhiteArgb).withAlpha (alpha));
            g.fillRect (bounds);
        }

        g.setColour (juce::Colour (kWhiteArgb));
        for (const auto& band : bootBurstBands)
        {
            const int y = bounds.getY() + juce::roundToInt (band.yFrac * static_cast<float> (bounds.getHeight()));
            const int x = bounds.getX() + juce::roundToInt (static_cast<float> (band.shiftPx)
                                                              * static_cast<float> (band.shiftDir)
                                                              * clamped);
            g.fillRect (x, y, bounds.getWidth(), band.thickness);
        }

        for (const auto& pixel : bootBurstDeadPixels)
        {
            const int x = bounds.getX() + juce::roundToInt (pixel.x * static_cast<float> (bounds.getWidth()));
            const int y = bounds.getY() + juce::roundToInt (pixel.y * static_cast<float> (bounds.getHeight()));
            g.fillRect (x, y, 1, 1);
        }
    }

    int GlitchOverlay::scheduleNextFlickerInterval()
    {
        const int minTicks = bypassAmplified ? kFlickerBypassMinIntervalTicks : kFlickerNormalMinIntervalTicks;
        const int maxTicks = bypassAmplified ? kFlickerBypassMaxIntervalTicks : kFlickerNormalMaxIntervalTicks;
        const int span = juce::jmax (1, maxTicks - minTicks + 1);
        return minTicks + flickerRandom.nextInt (span);
    }

    void GlitchOverlay::triggerFlickerBurst()
    {
        const int bandCount = bypassAmplified ? kFlickerBypassBands : kFlickerNormalBands;
        const int flickerDeadPixelCount = bypassAmplified ? kFlickerBypassDeadPixels : kFlickerNormalDeadPixels;
        const int maxShift = bypassAmplified ? kFlickerBypassMaxShift : kFlickerNormalMaxShift;

        for (int i = 0; i < bandCount; ++i)
        {
            auto& band = flickerBands[static_cast<size_t> (i)];
            band.yFrac = flickerRandom.nextFloat();
            band.thickness = 1 + flickerRandom.nextInt (3);
            band.shiftDir = (flickerRandom.nextInt (2) == 0) ? -1 : +1;
            band.shiftPx = 1 + flickerRandom.nextInt (juce::jmax (1, maxShift));
        }

        for (int i = 0; i < flickerDeadPixelCount; ++i)
            flickerDeadPixels[static_cast<size_t> (i)] =
                juce::Point<float> { flickerRandom.nextFloat(), flickerRandom.nextFloat() };

        flickerBurstActive = true;
        flickerBurstElapsedTicks = 0;
        ++flickerBurstCount;
    }

    void GlitchOverlay::setBypassAmplified (bool amplified) noexcept
    {
        if (bypassAmplified == amplified)
            return;

        bypassAmplified = amplified;

        if (! flickerBurstActive)
            ticksToNextFlicker = scheduleNextFlickerInterval();
    }

    void GlitchOverlay::drawFlickerBurst (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (! flickerBurstActive || bounds.isEmpty())
            return;

        const int duration = bypassAmplified ? kFlickerBypassDurationTicks : kFlickerNormalDurationTicks;
        const float progress = juce::jlimit (0.0f, 1.0f,
                                             static_cast<float> (flickerBurstElapsedTicks)
                                                 / static_cast<float> (duration));
        const int bandCount = bypassAmplified ? kFlickerBypassBands : kFlickerNormalBands;
        const int flickerDeadPixelCount = bypassAmplified ? kFlickerBypassDeadPixels : kFlickerNormalDeadPixels;

        g.setColour (juce::Colour (kWhiteArgb));

        for (int i = 0; i < bandCount; ++i)
        {
            const auto& band = flickerBands[static_cast<size_t> (i)];
            const int y = bounds.getY()
                        + juce::roundToInt (band.yFrac * static_cast<float> (bounds.getHeight()));
            const int x = bounds.getX()
                        + juce::roundToInt (static_cast<float> (band.shiftPx)
                                            * static_cast<float> (band.shiftDir)
                                            * progress);
            g.fillRect (x, y, bounds.getWidth(), band.thickness);
        }

        for (int i = 0; i < flickerDeadPixelCount; ++i)
        {
            const auto& pixel = flickerDeadPixels[static_cast<size_t> (i)];
            const int x = bounds.getX() + juce::roundToInt (pixel.x * static_cast<float> (bounds.getWidth()));
            const int y = bounds.getY() + juce::roundToInt (pixel.y * static_cast<float> (bounds.getHeight()));
            g.fillRect (x, y, 1, 1);
        }
    }

    int GlitchOverlay::getDriftBandStep() const noexcept
    {
        return (frameTickCount / kDriftBandStepTicks) % kDriftBandSteps;
    }

    bool GlitchOverlay::isFlickerBlockVisible() const noexcept
    {
        return (frameTickCount / kFlickerPeriodTicks) % 2 == 0;
    }

    int GlitchOverlay::getSweepStep() const noexcept
    {
        return (frameTickCount / kSweepStepTicks) % kSweepSteps;
    }

    float GlitchOverlay::getSweepPosition() const noexcept
    {
        return static_cast<float> (getSweepStep()) / static_cast<float> (kSweepSteps);
    }

    void GlitchOverlay::drawTriplet (juce::Graphics& g, std::function<void (juce::Point<float>)> strokeAtOffset)
    {
        juce::ignoreUnused (g);
        if (! strokeAtOffset)
            return;

        strokeAtOffset (juce::Point<float> { -1.0f, 0.0f });
        strokeAtOffset (juce::Point<float> {  0.0f, 0.0f });
        strokeAtOffset (juce::Point<float> {  1.0f, 0.0f });
    }

    void GlitchOverlay::drawSweep (juce::Graphics& g, juce::Rectangle<int> bounds, float position)
    {
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        const float clamped = juce::jlimit (0.0f, 1.0f, position);
        const int usable = juce::jmax (0, bounds.getWidth() - kSweepWidth);
        const int x = bounds.getX()
                    + static_cast<int> (std::round (clamped * static_cast<float> (usable)));

        g.setColour (juce::Colour (kWhiteArgb));
        g.fillRect (x, bounds.getY(), kSweepWidth, bounds.getHeight());
    }

    void GlitchOverlay::drawHorizontalBand (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        const int step = getDriftBandStep();
        const int stride = juce::jmax (1, bounds.getWidth() / kDriftBandSteps);
        const int x = bounds.getX() + step * stride;
        const int y = bounds.getY();

        g.setColour (juce::Colour (kWhiteArgb));
        g.fillRect (x, y, stride, kDriftBandHeight);
    }

    void GlitchOverlay::drawFlickerBlock (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (bounds.getWidth() < kFlickerBlockSize || bounds.getHeight() < kFlickerBlockSize)
            return;

        if (! isFlickerBlockVisible())
            return;

        const int x = bounds.getCentreX() - kFlickerBlockSize / 2;
        const int y = bounds.getCentreY() - kFlickerBlockSize / 2;

        g.setColour (juce::Colour (kWhiteArgb));
        g.fillRect (x, y, kFlickerBlockSize, kFlickerBlockSize);
    }

    void GlitchOverlay::drawDitherNoise (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (bounds.isEmpty())
            return;

        const auto& frame = grainFrames[static_cast<size_t> (grainFrameIndex)];

        g.setImageResamplingQuality (juce::Graphics::lowResamplingQuality);
        g.setOpacity (1.0f);
        g.drawImage (frame, bounds.toFloat(), juce::RectanglePlacement::stretchToFit);
        g.setImageResamplingQuality (juce::Graphics::mediumResamplingQuality);
    }

    void GlitchOverlay::drawScanlines (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (bounds.isEmpty())
            return;

        g.setColour (juce::Colour (kWhiteArgb));

        const int xStart = bounds.getX();
        const int width = bounds.getWidth();

        for (int y = bounds.getY(); y < bounds.getBottom(); y += kScanlineSpacing)
            g.drawHorizontalLine (y, static_cast<float> (xStart), static_cast<float> (xStart + width));
    }

    void GlitchOverlay::drawDeadPixelScatter (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (bounds.isEmpty())
            return;

        if (lastDeadPixelBounds != bounds || deadPixelCount == 0)
        {
            lastDeadPixelBounds = bounds;
            rerollDeadPixels (bounds);
        }

        g.setColour (juce::Colour (kWhiteArgb));

        for (int i = 0; i < deadPixelCount; ++i)
        {
            const auto& pos = deadPixels[static_cast<size_t> (i)];
            g.fillRect (pos.x, pos.y, 1, 1);
        }
    }

    void GlitchOverlay::regenerateGrainFrames()
    {
        for (int i = 0; i < kGrainFrames; ++i)
        {
            auto& frame = grainFrames[static_cast<size_t> (i)];
            frame = juce::Image (juce::Image::ARGB, kGrainTextureSize, kGrainTextureSize, true);

            juce::Random frameRandom { i * 137 + 42 };

            for (int y = 0; y < kGrainTextureSize; ++y)
            {
                for (int x = 0; x < kGrainTextureSize; ++x)
                {
                    const bool isWhite = frameRandom.nextFloat() < kDitherDutyCycle;
                    frame.setPixelAt (x, y, juce::Colour (isWhite ? kWhiteArgb : kBlackArgb));
                }
            }
        }
    }

    void GlitchOverlay::rerollDeadPixels (juce::Rectangle<int> bounds)
    {
        if (bounds.isEmpty())
        {
            deadPixelCount = 0;
            return;
        }

        deadPixelCount = kDeadPixelMin
                       + random.nextInt (kDeadPixelMax - kDeadPixelMin + 1);

        for (int i = 0; i < deadPixelCount; ++i)
        {
            const int x = bounds.getX() + random.nextInt (bounds.getWidth());
            const int y = bounds.getY() + random.nextInt (bounds.getHeight());
            deadPixels[static_cast<size_t> (i)] = juce::Point<int> (x, y);
        }
    }
}