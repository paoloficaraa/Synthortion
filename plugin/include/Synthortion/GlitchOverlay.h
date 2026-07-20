#pragma once

#include <array>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    // Renders procedural CRT-glass texture layers over a pure-black substrate.
    // All layers are strictly 1-bit (pure #000 or #FFF) with no alpha blending;
    // visual density is achieved through spatial duty-cycle sampling.
    class GlitchOverlay final
    {
    public:
        GlitchOverlay();

        void tick();

        void drawDitherNoise (juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawScanlines (juce::Graphics& g, juce::Rectangle<int> bounds);
        void drawDeadPixelScatter (juce::Graphics& g, juce::Rectangle<int> bounds);

        // 2 px-tall #FFF horizontal band drifting L->R across `bounds` in
        // kDriftBandSteps hard positions (~0.5 Hz cycle at a 60 Hz tick).
        void drawHorizontalBand (juce::Graphics& g, juce::Rectangle<int> bounds);

        // 4x4 px #FFF block flickering on/off at a 500 ms cadence (one cycle
        // per second at a 60 Hz tick) centred on `bounds`; no-op when hidden.
        void drawFlickerBlock (juce::Graphics& g, juce::Rectangle<int> bounds);

        // RGB-split Triplet: invokes @p strokeAtOffset three times at horizontal
        // offsets -1, 0, +1 px so the caller's stroke renders as three parallel
        // lines (the strict palette forbids real RGB, so all three are #FFF).
        void drawTriplet (juce::Graphics& g, std::function<void (juce::Point<float>)> strokeAtOffset);

        // Scan-charge Sweep: a kSweepWidth-px vertical #FFF band at the L->R
        // position @p position in [0, 1] across `bounds`. The position is
        // advanced per tick via getSweepPosition().
        void drawSweep (juce::Graphics& g, juce::Rectangle<int> bounds, float position);

        // Test accessors.
        int getGrainFrameIndex() const noexcept { return grainFrameIndex; }
        int getDeadPixelRerollCount() const noexcept { return deadPixelRerollCount; }
        int getFrameTickCount() const noexcept { return frameTickCount; }
        int getDriftBandStep() const noexcept;
        bool isFlickerBlockVisible() const noexcept;
        int getSweepStep() const noexcept;
        float getSweepPosition() const noexcept;
        static constexpr int tileSizeForTests() noexcept { return kGrainTextureSize; }
        static constexpr int driftBandHeight() noexcept { return kDriftBandHeight; }
        static constexpr int flickerBlockSize() noexcept { return kFlickerBlockSize; }
        static constexpr int driftBandStepForTests() noexcept { return kDriftBandSteps; }
        static constexpr int driftBandStepTicksForTests() noexcept { return kDriftBandStepTicks; }
        static constexpr int flickerPeriodTicksForTests() noexcept { return kFlickerPeriodTicks; }
        static constexpr int sweepWidthForTests() noexcept { return kSweepWidth; }
        static constexpr int sweepStepsForTests() noexcept { return kSweepSteps; }
        static constexpr int sweepStepTicksForTests() noexcept { return kSweepStepTicks; }

    private:
        static constexpr int kGrainTextureSize = 64;
        static constexpr int kGrainFrames = 8;
        // Fraction of dither tile pixels that are #FFF (the rest are #000).
        static constexpr float kDitherDutyCycle = 0.06f;
        static constexpr int kScanlineSpacing = 3;
        static constexpr int kDeadPixelMin = 8;
        static constexpr int kDeadPixelMax = 12;
        static constexpr int kDeadPixelRerollTicks = 5;

        // Placeholder glitch motion constants (Slice C). Tick cadence follows
        // the editor's 60 Hz timer, so each tick is ~16.7 ms.
        static constexpr int kDriftBandHeight = 2;
        static constexpr int kDriftBandSteps = 16;
        static constexpr int kDriftBandStepTicks = 8;   // 16 * 8 = 128 ticks ~= 0.5 Hz cycle
        static constexpr int kFlickerBlockSize = 4;
        static constexpr int kFlickerPeriodTicks = 30;  // 500 ms on / 500 ms off

        // Scan-charge Sweep constants (Slice F). Same 16-step / 8-tick cadence
        // as the drift band => ~0.5 Hz L->R wipe at a 60 Hz tick.
        static constexpr int kSweepWidth = 4;
        static constexpr int kSweepSteps = 16;
        static constexpr int kSweepStepTicks = 8;

        static constexpr juce::uint32 kBlackArgb = 0xFF000000;
        static constexpr juce::uint32 kWhiteArgb = 0xFFFFFFFF;

        std::array<juce::Image, static_cast<size_t> (kGrainFrames)> grainFrames;
        int grainFrameIndex = 0;
        int frameTickCount = 0;

        std::array<juce::Point<int>, static_cast<size_t> (kDeadPixelMax)> deadPixels;
        int deadPixelCount = kDeadPixelMin;
        int ticksSinceReroll = 0;
        int deadPixelRerollCount = 0;
        juce::Rectangle<int> lastDeadPixelBounds;

        juce::Random random { 0x5EED1D };

        void regenerateGrainFrames();
        void rerollDeadPixels (juce::Rectangle<int> bounds);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchOverlay)
    };
}