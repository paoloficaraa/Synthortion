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

        // Horizontal Slice glitch burst fired when the bypass Block toggles
        // state: 2-3 hard #FFF horizontal bands shift L/R by 1-3 px during the
        // ~150 ms burst window, then snap back. Drawn over the children of the
        // editor so the slice glitch visibly slides across the rendered UI.
        void triggerBypassSlices();
        void drawBypassSlices (juce::Graphics& g, juce::Rectangle<int> bounds);

        // One-time boot Burst fired on first window show: 100 ms #FFF flash
        // fading over 300 ms, 6 Slice displacements, and dead pixel field.
        void triggerBootBurst();
        void drawBootBurst (juce::Graphics& g, juce::Rectangle<int> bounds, float progress);

        // Rare random-event Flicker burst (issue #33): sporadic glitch
        // bursts (horizontal slice drift + brief dead pixel field) fired
        // at ~10-30s intervals normally. While bypass is engaged the
        // interval shortens, the band count doubles, the burst duration
        // lengthens and the displacement strengthens to visually
        // reinforce the inactive state. Drawn over the editor children so
        // the flicker visibly slides across the rendered UI. The existing
        // bypass Slice burst (triggerBypassSlices) and the dead pixel
        // scatter cadence are preserved unchanged.
        void setBypassAmplified (bool amplified) noexcept;
        bool isBypassAmplified() const noexcept { return bypassAmplified; }
        void drawFlickerBurst (juce::Graphics& g, juce::Rectangle<int> bounds);

        // Test accessors.
        int getGrainFrameIndex() const noexcept { return grainFrameIndex; }
        int getDeadPixelRerollCount() const noexcept { return deadPixelRerollCount; }
        int getFrameTickCount() const noexcept { return frameTickCount; }
        int getDriftBandStep() const noexcept;
        bool isFlickerBlockVisible() const noexcept;
        int getSweepStep() const noexcept;
        float getSweepPosition() const noexcept;
        bool isBypassSliceActive() const noexcept { return bypassSliceActive; }
        bool isBootBurstActive() const noexcept { return bootBurstActive; }
        bool isBootBurstFired() const noexcept { return bootBurstFired; }
        int getBootBurstElapsedTicks() const noexcept { return bootBurstElapsedTicks; }
        float getBootBurstProgress() const noexcept;
        static constexpr int tileSizeForTests() noexcept { return kGrainTextureSize; }
        static constexpr int driftBandHeight() noexcept { return kDriftBandHeight; }
        static constexpr int flickerBlockSize() noexcept { return kFlickerBlockSize; }
        static constexpr int driftBandStepForTests() noexcept { return kDriftBandSteps; }
        static constexpr int driftBandStepTicksForTests() noexcept { return kDriftBandStepTicks; }
        static constexpr int flickerPeriodTicksForTests() noexcept { return kFlickerPeriodTicks; }
        static constexpr int sweepWidthForTests() noexcept { return kSweepWidth; }
        static constexpr int sweepStepsForTests() noexcept { return kSweepSteps; }
        static constexpr int sweepStepTicksForTests() noexcept { return kSweepStepTicks; }
        static constexpr int bypassSliceDurationTicksForTests() noexcept { return kBypassSliceDurationTicks; }
        static constexpr int bypassSliceStepsForTests() noexcept { return kBypassSliceSteps; }
        static constexpr int bypassSliceBandCountForTests() noexcept { return kBypassSliceBands; }
        static constexpr int bootBurstDurationTicksForTests() noexcept { return kBootBurstDurationTicks; }
        static constexpr int bootBurstFlashTicksForTests() noexcept { return kBootBurstFlashTicks; }
        static constexpr int bootBurstBandCountForTests() noexcept { return kBootBurstBands; }
        static constexpr int bootBurstDeadPixelCountForTests() noexcept { return kBootBurstDeadPixels; }
        static constexpr int flickerNormalMinIntervalTicksForTests() noexcept { return kFlickerNormalMinIntervalTicks; }
        static constexpr int flickerNormalMaxIntervalTicksForTests() noexcept { return kFlickerNormalMaxIntervalTicks; }
        static constexpr int flickerBypassMinIntervalTicksForTests() noexcept { return kFlickerBypassMinIntervalTicks; }
        static constexpr int flickerBypassMaxIntervalTicksForTests() noexcept { return kFlickerBypassMaxIntervalTicks; }
        static constexpr int flickerNormalDurationTicksForTests() noexcept { return kFlickerNormalDurationTicks; }
        static constexpr int flickerBypassDurationTicksForTests() noexcept { return kFlickerBypassDurationTicks; }
        static constexpr int flickerNormalBandCountForTests() noexcept { return kFlickerNormalBands; }
        static constexpr int flickerBypassBandCountForTests() noexcept { return kFlickerBypassBands; }
        static constexpr int flickerNormalDeadPixelCountForTests() noexcept { return kFlickerNormalDeadPixels; }
        static constexpr int flickerBypassDeadPixelCountForTests() noexcept { return kFlickerBypassDeadPixels; }
        static constexpr int flickerNormalMaxShiftForTests() noexcept { return kFlickerNormalMaxShift; }
        static constexpr int flickerBypassMaxShiftForTests() noexcept { return kFlickerBypassMaxShift; }
        bool isFlickerBurstActive() const noexcept { return flickerBurstActive; }
        int getFlickerBurstElapsedTicks() const noexcept { return flickerBurstElapsedTicks; }
        int getTicksToNextFlicker() const noexcept { return ticksToNextFlicker; }
        int getFlickerBurstCount() const noexcept { return flickerBurstCount; }

    private:
        struct SliceBand
        {
            float yFrac;
            int thickness;
            int shiftDir;
            int shiftPx;
        };

        static constexpr int kBypassSliceDurationTicks = 9;   // ~150 ms at the 60 Hz timer
        static constexpr int kBypassSliceSteps = 8;            // matched to the Block toggle's N=8 Step easing
        static constexpr int kBypassSliceBands = 3;
        std::array<SliceBand, static_cast<size_t> (kBypassSliceBands)> bypassSliceBands {};
        bool bypassSliceActive = false;
        int bypassSliceElapsedTicks = 0;

        // Boot Burst: 400 ms total, 100 ms flash at 60 Hz (24 / 6 ticks).
        static constexpr int kBootBurstDurationTicks = 24;
        static constexpr int kBootBurstFlashTicks = 6;
        static constexpr int kBootBurstBands = 6;
        static constexpr int kBootBurstDeadPixels = 60;
        std::array<SliceBand, static_cast<size_t> (kBootBurstBands)> bootBurstBands {};
        std::array<juce::Point<float>, static_cast<size_t> (kBootBurstDeadPixels)> bootBurstDeadPixels {};
        bool bootBurstActive = false;
        bool bootBurstFired = false;
        int bootBurstElapsedTicks = 0;

        // Rare Flicker burst system (issue #33). Normal mode fires ~10-30s
        // (600-1800 ticks at 60 Hz); bypass-amplified mode fires ~3-8s
        // (180-480 ticks) with double the bands, double the duration and
        // double the max displacement to reinforce the inactive state.
        static constexpr int kFlickerNormalMinIntervalTicks = 600;    // ~10s at 60 Hz
        static constexpr int kFlickerNormalMaxIntervalTicks = 1800;   // ~30s at 60 Hz
        static constexpr int kFlickerBypassMinIntervalTicks = 180;    // ~3s at 60 Hz
        static constexpr int kFlickerBypassMaxIntervalTicks = 480;    // ~8s at 60 Hz
        static constexpr int kFlickerNormalDurationTicks = 18;        // ~300ms at 60 Hz
        static constexpr int kFlickerBypassDurationTicks = 36;        // ~600ms at 60 Hz
        static constexpr int kFlickerNormalBands = 3;
        static constexpr int kFlickerBypassBands = 6;
        static constexpr int kFlickerNormalDeadPixels = 20;
        static constexpr int kFlickerBypassDeadPixels = 40;
        static constexpr int kFlickerNormalMaxShift = 3;
        static constexpr int kFlickerBypassMaxShift = 6;
        static constexpr int kFlickerMaxBands = kFlickerBypassBands;
        static constexpr int kFlickerMaxDeadPixels = kFlickerBypassDeadPixels;

        std::array<SliceBand, static_cast<size_t> (kFlickerMaxBands)> flickerBands {};
        std::array<juce::Point<float>, static_cast<size_t> (kFlickerMaxDeadPixels)> flickerDeadPixels {};
        bool flickerBurstActive = false;
        int flickerBurstElapsedTicks = 0;
        int ticksToNextFlicker = 0;
        bool bypassAmplified = false;
        int flickerBurstCount = 0;
        juce::Random flickerRandom { 0xF11C4E };

        int scheduleNextFlickerInterval();
        void triggerFlickerBurst();

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