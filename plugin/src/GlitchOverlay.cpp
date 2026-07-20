#include "Synthortion/GlitchOverlay.h"

namespace synthortion
{
    GlitchOverlay::GlitchOverlay()
    {
        regenerateGrainFrames();
        rerollDeadPixels (lastDeadPixelBounds);
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
    }

    int GlitchOverlay::getDriftBandStep() const noexcept
    {
        return (frameTickCount / kDriftBandStepTicks) % kDriftBandSteps;
    }

    bool GlitchOverlay::isFlickerBlockVisible() const noexcept
    {
        return (frameTickCount / kFlickerPeriodTicks) % 2 == 0;
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