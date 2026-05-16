#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
#include <memory>

namespace synthortion
{
    namespace Gui
    {
        class Bulb : public juce::Component
        {
        public:
            explicit Bulb(const juce::Colour &c);

            void paint(juce::Graphics &g) override;
            void setState(bool state);

        private:
            static constexpr float kBulbReduction = 2.0f;
            static constexpr float kBulbBorderWidth = 1.0f;
            static constexpr float kGlowExpansion = 1.5f;
            static constexpr float kOnAlpha = 0.8f;
            static constexpr float kBorderAlpha = 0.8f;
            static constexpr float kOffDarkenFactor = 0.3f;

            bool isOn = false;
            juce::Colour colour{};

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Bulb)
        };

        class VerticalDiscreteMeter : public juce::Component, public juce::Timer
        {
        public:
            explicit VerticalDiscreteMeter(std::function<float()> valueFunction);

            void paint(juce::Graphics &g) override;
            void resized() override;
            void timerCallback() override;

        private:
            static constexpr int kDefaultNumBulbs = 12;
            static constexpr int kTimerHz = 30;
            static constexpr float kMinDbLevel = -60.0f;
            static constexpr float kMaxDbLevel = 6.0f;
            static constexpr float kSlotReduction = 2.0f;
            static constexpr float kSlotCornerRadius = 6.0f;
            static constexpr float kSlotBorderWidth = 1.0f;
            static constexpr float kSlotBorderAlpha = 0.6f;
            static constexpr int kBulbAreaReduction = 2;
            static constexpr int kBulbSpacing = 2;

            static constexpr float kGradientStop1 = 0.3f;
            static constexpr float kGradientStop2 = 0.7f;

            static constexpr juce::uint32 kGradientBottomArgb = 0xFF4A148C;
            static constexpr juce::uint32 kGradientMid1Argb = 0xFF7B1FA2;
            static constexpr juce::uint32 kGradientMid2Argb = 0xFFBA68C8;
            static constexpr juce::uint32 kGradientTopArgb = 0xFFE040FB;

            void createBulbs();
            float getCurrentLevel() const;

            std::function<float()> valueSupplier;
            std::vector<std::unique_ptr<Bulb>> bulbs;
            const int totalNumberOfBulbs = kDefaultNumBulbs;

            float cachedLevel = 0.0f;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalDiscreteMeter)
        };
    }
}
