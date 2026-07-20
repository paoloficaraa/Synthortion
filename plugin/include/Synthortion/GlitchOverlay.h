#pragma once

#include <array>
#include <juce_gui_basics/juce_gui_basics.h>

namespace synthortion
{
    // GlitchOverlay renders the procedural CRT-glass texture layers that sit on
    // top of the DEADLOCK pure-black substrate. All layers are strictly 1-bit:
    // every drawn pixel is either pure #000 or pure #FFF, never a mid-grey. No
    // alpha blending is used; sparse visual density is achieved through spatial
    // duty-cycle sampling rather than translucent overlays.
    class GlitchOverlay final
    {
    public:
        GlitchOverlay();

        // Drives the per-frame animation cadence. Called once per
        // AudioPluginAudioProcessorEditor::timerCallback (kTimerHz = 60).
        void tick();

        // Draws the current 1-bit dither noise tile stretched over bounds.
        // Drawn in PluginEditor::paint so the grain sits behind child widgets.
        void drawDitherNoise (juce::Graphics& g, juce::Rectangle<int> bounds);

        // Draws horizontal 1-bit scanlines across bounds. Drawn in
        // PluginEditor::paint (before widgets) so the scanline texture is part
        // of the background substrate and does not visually compete with knob
        // arcs, scope traces or meter bars drawn on top of it.
        void drawScanlines (juce::Graphics& g, juce::Rectangle<int> bounds);

        // Draws 8-12 single-pixel #FFF "dead pixel" dust dots scattered over
        // bounds. Positions are re-rolled roughly every 80 ms via tick(). Drawn
        // in PluginEditor::paintOverChildren so the dust sits visually above all
        // child components like CRT-glass debris.
        void drawDeadPixelScatter (juce::Graphics& g, juce::Rectangle<int> bounds);

        // Test accessors.
        int getGrainFrameIndex() const noexcept { return grainFrameIndex; }
        int getDeadPixelRerollCount() const noexcept { return deadPixelRerollCount; }
        static constexpr int tileSizeForTests() noexcept { return kGrainTextureSize; }

    private:
        static constexpr int kGrainTextureSize = 64;
        static constexpr int kGrainFrames = 8;
        // Fraction of pixels per dither tile that are #FFF (the rest are #000).
        // Repurposed from the legacy kGrainAlpha overlay opacity constant into a
        // spatial sampling duty cycle so no withAlpha(mid-grey) call is needed.
        static constexpr float kDitherDutyCycle = 0.06f;
        static constexpr int kScanlineSpacing = 3;       // vertical px between scanlines (2-3 range)
        static constexpr int kScanlineThickness = 1;
        static constexpr int kDeadPixelMin = 8;
        static constexpr int kDeadPixelMax = 12;
        // ~80 ms at kTimerHz = 60 -> 60 ticks/sec -> ~5 ticks per 80 ms.
        static constexpr int kDeadPixelRerollTicks = 5;

        static constexpr juce::uint32 kBlackArgb = 0xFF000000;
        static constexpr juce::uint32 kWhiteArgb = 0xFFFFFFFF;

        std::array<juce::Image, static_cast<size_t> (kGrainFrames)> grainFrames;
        int grainFrameIndex = 0;

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