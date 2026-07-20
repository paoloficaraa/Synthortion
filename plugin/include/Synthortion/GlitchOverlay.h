#pragma once

#include <array>
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

        // Test accessors.
        int getGrainFrameIndex() const noexcept { return grainFrameIndex; }
        int getDeadPixelRerollCount() const noexcept { return deadPixelRerollCount; }
        static constexpr int tileSizeForTests() noexcept { return kGrainTextureSize; }

    private:
        static constexpr int kGrainTextureSize = 64;
        static constexpr int kGrainFrames = 8;
        // Fraction of dither tile pixels that are #FFF (the rest are #000).
        static constexpr float kDitherDutyCycle = 0.06f;
        static constexpr int kScanlineSpacing = 3;
        static constexpr int kDeadPixelMin = 8;
        static constexpr int kDeadPixelMax = 12;
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