#pragma once

#include <cmath>
#include <JuceHeader.h>
#include "Synthortion/AnimatedKnob.h"
#include "Synthortion/AnimationController.h"
#include "Synthortion/AudioScopeRingBuffer.h"
#include "Synthortion/SynthortionLookAndFeel.h"
#include "Synthortion/BypassComponent.h"
#include "Synthortion/MeterComponent.h"
#include "Synthortion/OscilloscopeComponent.h"
#include "Synthortion/PanelComponent.h"
#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class SynthortionLookAndFeelTests : public juce::UnitTest
{
public:
    SynthortionLookAndFeelTests()
        : juce::UnitTest("SynthortionLookAndFeel", "Synthortion")
    {
    }

    void runTest() override
    {
        const juce::MessageManagerLock mmLock;

        testBebasNeueFontRouting();
        testMontserratFontRouting();
        testTypographyScale();
    }

private:
    void testBebasNeueFontRouting()
    {
        beginTest("BebasNeue font routing returns a non-null typeface");

        SynthortionLookAndFeel lookAndFeel;
        auto typeface = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue"));

        expect(typeface != nullptr, "Expected non-null BebasNeue typeface");
    }

    void testMontserratFontRouting()
    {
        beginTest("Default sans-serif font routing returns Montserrat typeface");

        SynthortionLookAndFeel lookAndFeel;
        auto montserrat = lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat"));
        auto defaultSans = lookAndFeel.getTypefaceForFont(
            juce::FontOptions().withName(juce::Font::getDefaultSansSerifFontName()));

        expect(montserrat != nullptr, "Expected non-null Montserrat typeface");
        expect(defaultSans == montserrat, "Expected default sans-serif to resolve to Montserrat");
    }

    void testTypographyScale()
    {
        beginTest("Typography scale fonts are pre-computed and resolve correctly");

        SynthortionLookAndFeel lookAndFeel;

        auto heading = lookAndFeel.getSectionHeadingFont();
        expect (juce::roundToInt(heading.getHeight()) == 18, "Section heading should be 18px");
        expect (lookAndFeel.getTypefaceForFont(heading) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("BebasNeue")),
                "Section heading should resolve to the BebasNeue typeface");

        auto paramLabel = lookAndFeel.getParameterLabelFont();
        expect (juce::roundToInt(paramLabel.getHeight()) == 13, "Parameter label should be 13px");
        expect (paramLabel.getTypefaceStyle() == "Medium", "Parameter label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(paramLabel) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter label should resolve to the Montserrat typeface");

        auto paramValue = lookAndFeel.getParameterValueFont();
        expect (juce::roundToInt(paramValue.getHeight()) == 12, "Parameter value should be 12px");
        expect (paramValue.getTypefaceStyle() == "Regular", "Parameter value should be Regular weight");
        expect (lookAndFeel.getTypefaceForFont(paramValue) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Parameter value should resolve to the Montserrat typeface");

        auto bypass = lookAndFeel.getBypassLabelFont();
        expect (juce::roundToInt(bypass.getHeight()) == 13, "Bypass label should be 13px");
        expect (bypass.getTypefaceStyle() == "Medium", "Bypass label should be Medium weight");
        expect (lookAndFeel.getTypefaceForFont(bypass) == lookAndFeel.getTypefaceForFont(juce::FontOptions().withName("Montserrat")),
                "Bypass label should resolve to the Montserrat typeface");
    }
};

static SynthortionLookAndFeelTests synthortionLookAndFeelTests;

namespace synthortion
{
    class AudioPluginTests final : public juce::UnitTest
    {
    public:
        AudioPluginTests()
            : juce::UnitTest ("Synthortion component tests", "Synthortion")
        {
        }

        void runTest() override
        {
            testPanelComponentRendersBackgroundColour();
            testBypassComponentLedTogglesOnClick();
            testBypassSwitchSpringOvershoot();
            testBypassSwitchParameterAttachment();
            testPanelComponentTitleFont();
            testEditorIsOpaque();
            testPanelComponentIsOpaque();
            testBypassComponentIsNotOpaque();
            testCopperColorPalette();
            testEditorSizeIs720x440();
            testEditorContainsOscilloscope();
            testEditorContainsMeters();
            testDistortionPanelIsLargest();
            testAnimationControllerCreatesAnimator();
            testAudioScopeRingBufferTransfersSamples();
            testOscilloscopeReadsInputBuffer();
            testOscilloscopeReadsOutputBuffer();
            testOscilloscopeDetectsSilence();
            testOscilloscopeDetectsSignal();
            testOscilloscopeBypassFlattensOutput();
            testOscilloscopeBoundsInEditor();
            testAnimatedKnobIsASlider();
            testAnimatedKnobHasRotaryStyle();
            testEditorContainsEightAnimatedKnobs();
            testAnimatedKnobBindsToParameter();
            testInputMeterIsOnLeftSideBar();
            testOutputMeterIsOnRightSideBar();
            testInputGainKnobBindsToParameter();
            testOutputGainKnobBindsToParameter();
            testMeterCalculatesRMS();
            testMeterPeakHoldJumpsToPeak();
        }

    private:
        void testPanelComponentRendersBackgroundColour()
        {
            beginTest ("PanelComponent renders an opaque background colour");

            SynthortionLookAndFeel lookAndFeel;

            const juce::Colour bgColour (0xFF123456);
            PanelComponent panel ("DISTORTION", bgColour);
            panel.setSize (100, 100);
            panel.setLookAndFeel (&lookAndFeel);

            const auto snapshot = panel.createComponentSnapshot (panel.getLocalBounds());

            expect (snapshot.getPixelAt (50, 50) == bgColour,
                    "PanelComponent centre pixel should match the background colour");
            expect (snapshot.getPixelAt (0, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 0).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (0, 99).getAlpha() == 0xFF);
            expect (snapshot.getPixelAt (99, 99).getAlpha() == 0xFF);
        }

        void testBypassComponentLedTogglesOnClick()
        {
            beginTest ("BypassComponent LED toggles on programmatic click");

            BypassComponent bypass;
            bypass.setSize (200, 24);

            expect (! bypass.isLedOn());

            bypass.getToggleButton().triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (bypass.isLedOn());
            expect (bypass.getToggleButton().getToggleState());
        }

        void testBypassSwitchSpringOvershoot()
        {
            beginTest ("BypassSwitch spring easing overshoots the target");

            auto spring = juce::Easings::createSpring (
                juce::SpringEasingOptions().withFrequency (3.5f).withAttenuation (3.0f));

            bool overshoots = false;

            for (float t = 0.0f; t <= 1.0f; t += 0.01f)
            {
                if (spring (t) > 1.05f)
                {
                    overshoots = true;
                    break;
                }
            }

            expect (overshoots, "Spring easing should visibly overshoot the target value");
        }

        void testBypassSwitchParameterAttachment()
        {
            beginTest ("BypassSwitch is attached to PLUGIN_BYPASS");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto& bypassButton = editor.getBypassComponent().getToggleButton();

            expect (! bypassButton.getToggleState());

            processor.getAPVTS().getParameter ("PLUGIN_BYPASS")->setValueNotifyingHost (1.0f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (bypassButton.getToggleState(), "Button should follow PLUGIN_BYPASS parameter turning on");

            bypassButton.triggerClick();
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (processor.getAPVTS().getRawParameterValue ("PLUGIN_BYPASS")->load() < 0.5f,
                    "PLUGIN_BYPASS parameter should follow button click turning off");
        }

        void testPanelComponentTitleFont()
        {
            beginTest ("PanelComponent title font is BebasNeue 18px");

            PanelComponent panel ("DISTORTION", juce::Colours::black);
            auto font = panel.getTitleFont();

            expect (juce::roundToInt(font.getHeight()) == 18, "Panel title should be 18px");
            expect (font.getTypefaceName().containsIgnoreCase("Bebas"), "Panel title should use BebasNeue");
        }

        void testEditorIsOpaque()
        {
            beginTest ("Plugin editor is opaque");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.isOpaque(), "Editor should be opaque for rendering performance");
        }

        void testPanelComponentIsOpaque()
        {
            beginTest ("PanelComponent is opaque");

            PanelComponent panel ("DISTORTION", juce::Colours::black);

            expect (panel.isOpaque(), "PanelComponent wrapper should be opaque for rendering performance");
        }

        void testBypassComponentIsNotOpaque()
        {
            beginTest ("BypassComponent stays non-opaque for LED glow");

            BypassComponent bypass;

            expect (! bypass.isOpaque(), "BypassComponent should remain non-opaque to preserve semi-transparent LED glow");
        }

        void testCopperColorPalette()
        {
            beginTest ("Color palette uses saturated violet and warm magenta");

            SynthortionLookAndFeel lookAndFeel;

            const auto copper = lookAndFeel.findColour (SynthortionLookAndFeel::copperAccentColourId);
            const auto copperBright = lookAndFeel.findColour (SynthortionLookAndFeel::copperBrightColourId);

            expect (copper == juce::Colour (0xFF7C3AED), "COPPER should be #7C3AED");
            expect (copperBright == juce::Colour (0xFFFF2D78), "COPPER_BRIGHT should be #FF2D78");
        }

        void testEditorSizeIs720x440()
        {
            beginTest ("Plugin editor dimensions are 720x440");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            expect (editor.getWidth() == 720, "Editor width should be 720");
            expect (editor.getHeight() == 440, "Editor height should be 440");
        }

        void testEditorContainsOscilloscope()
        {
            beginTest ("Plugin editor contains an OscilloscopeComponent");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int oscilloscopeCount = 0;
            for (auto* child : editor.getChildren())
                if (dynamic_cast<OscilloscopeComponent*> (child) != nullptr)
                    ++oscilloscopeCount;

            expect (oscilloscopeCount == 1, "Editor should contain exactly one OscilloscopeComponent");
        }

        void testEditorContainsMeters()
        {
            beginTest ("Plugin editor contains two MeterComponents");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int meterCount = 0;
            for (auto* child : editor.getChildren())
                if (dynamic_cast<MeterComponent*> (child) != nullptr)
                    ++meterCount;

            expect (meterCount == 2, "Editor should contain exactly two MeterComponents");
        }

        void testDistortionPanelIsLargest()
        {
            beginTest ("Distortion panel is the largest central panel");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            int distortionArea = 0;
            int chorusArea = 0;
            int delayArea = 0;
            int comingSoonArea = 0;

            for (auto* child : editor.getChildren())
            {
                if (auto* panel = dynamic_cast<PanelComponent*> (child))
                {
                    const int area = panel->getWidth() * panel->getHeight();
                    const auto title = panel->getTitle();

                    if (title == "DISTORTION")
                        distortionArea = area;
                    else if (title == "CHORUS")
                        chorusArea = area;
                    else if (title == "DELAY")
                        delayArea = area;
                    else if (title == "COMING SOON")
                        comingSoonArea = area;
                }
            }

            expect (distortionArea > chorusArea, "Distortion panel should be larger than Chorus");
            expect (distortionArea > delayArea, "Distortion panel should be larger than Delay");
            expect (distortionArea > comingSoonArea, "Distortion panel should be larger than Coming Soon");
        }

        void testAnimationControllerCreatesAnimator()
        {
            beginTest ("AnimationController creates a ValueAnimator with the expected duration");

            juce::Component dummy;
            AnimationController controller (&dummy);

            auto animator = controller.runAnimator (
                juce::ValueAnimatorBuilder()
                    .withValueChangedCallback ([] (float) {})
                    .withDurationMs (150)
                    .withEasing (juce::Easings::createLinear()));

            expect (std::abs (animator.getDurationMs() - 150.0) < 1.0e-6, "Animator duration should match builder");

            juce::ignoreUnused (controller);
        }

        void testAudioScopeRingBufferTransfersSamples()
        {
            beginTest ("AudioScopeRingBuffer transfers stereo samples");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            writeBuffer.setSample (1, 20, -0.25f);

            buffer.writeInput (writeBuffer);

            juce::AudioBuffer<float> readBuffer (2, 128);
            readBuffer.clear();
            const int read = buffer.readInput (readBuffer);

            expect (read == 64, "Should read back the written sample count");
            expect (std::abs (readBuffer.getSample (0, read - 64 + 10) - 0.5f) < 1.0e-6f, "Channel 0 sample should match");
            expect (std::abs (readBuffer.getSample (1, read - 64 + 20) - -0.25f) < 1.0e-6f, "Channel 1 sample should match");
        }

        void testOscilloscopeReadsInputBuffer()
        {
            beginTest ("OscilloscopeComponent reads the input ring buffer");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            writeBuffer.setSample (1, 20, -0.25f);

            buffer.writeInput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            const auto& input = scope.getCurrentInputBuffer();
            expect (input.getNumChannels() == 2, "Oscilloscope should read a stereo input buffer");
            expect (input.getNumSamples() == OscilloscopeComponent::kWindowSize, "Oscilloscope input buffer should match the window size");
            expect (std::abs (input.getSample (0, 10) - 0.5f) < 1.0e-6f, "Input channel 0 sample should match");
            expect (std::abs (input.getSample (1, 20) - -0.25f) < 1.0e-6f, "Input channel 1 sample should match");
        }

        void testOscilloscopeReadsOutputBuffer()
        {
            beginTest ("OscilloscopeComponent reads the output ring buffer");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 5, 0.75f);
            writeBuffer.setSample (1, 15, -0.5f);

            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            const auto& output = scope.getCurrentOutputBuffer();
            expect (output.getNumChannels() == 2, "Oscilloscope should read a stereo output buffer");
            expect (output.getNumSamples() == OscilloscopeComponent::kWindowSize, "Oscilloscope output buffer should match the window size");
            expect (std::abs (output.getSample (0, 5) - 0.75f) < 1.0e-6f, "Output channel 0 sample should match");
            expect (std::abs (output.getSample (1, 15) - -0.5f) < 1.0e-6f, "Output channel 1 sample should match");
        }

        void testOscilloscopeDetectsSilence()
        {
            beginTest ("OscilloscopeComponent reports silence for zero buffers");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            buffer.writeInput (writeBuffer);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (scope.isSilent(), "Oscilloscope should report silence for zero buffers");
        }

        void testOscilloscopeDetectsSignal()
        {
            beginTest ("OscilloscopeComponent reports non-silence for active buffers");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.5f);
            buffer.writeInput (writeBuffer);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (! scope.isSilent(), "Oscilloscope should report non-silence for active buffers");
        }

        void testOscilloscopeBypassFlattensOutput()
        {
            beginTest ("OscilloscopeComponent enters bypassed flatline state");

            AudioScopeRingBuffer buffer (512);
            juce::AudioBuffer<float> writeBuffer (2, 64);
            writeBuffer.clear();
            writeBuffer.setSample (0, 10, 0.8f);
            buffer.writeOutput (writeBuffer);

            OscilloscopeComponent scope (buffer, nullptr);
            scope.refresh();

            expect (! scope.isBypassed(), "Oscilloscope should start un-bypassed");

            scope.setBypassed (true);
            scope.refresh();

            expect (scope.isBypassed(), "Oscilloscope should report bypassed after setBypassed(true)");
        }

        void testOscilloscopeBoundsInEditor()
        {
            beginTest ("OscilloscopeComponent occupies the expected Top Bar area");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            for (auto* child : editor.getChildren())
            {
                if (auto* scope = dynamic_cast<OscilloscopeComponent*> (child))
                {
                    expect (scope->getWidth() >= 560, "Oscilloscope should be roughly 580px wide");
                    expect (scope->getHeight() >= 80, "Oscilloscope should be roughly 90px tall");
                    expect (scope->getX() >= 120, "Oscilloscope should sit to the right of the bypass switch");
                    return;
                }
            }

            expect (false, "Editor should contain an OscilloscopeComponent to check bounds");
        }

        int countAnimatedKnobsRecursive (juce::Component& parent)
        {
            int count = 0;

            for (auto* child : parent.getChildren())
            {
                if (dynamic_cast<AnimatedKnob*> (child) != nullptr)
                    ++count;

                count += countAnimatedKnobsRecursive (*child);
            }

            return count;
        }

        void testAnimatedKnobIsASlider()
        {
            beginTest ("AnimatedKnob inherits from juce::Slider");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            juce::ignoreUnused (knob);

            expect (std::is_base_of_v<juce::Slider, AnimatedKnob>,
                    "AnimatedKnob should inherit from juce::Slider so attachments work");
        }

        void testAnimatedKnobHasRotaryStyle()
        {
            beginTest ("AnimatedKnob uses a rotary style with no text box");

            juce::Component dummy;
            AnimationController controller (&dummy);
            AnimatedKnob knob (controller);

            expect (knob.getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag,
                    "AnimatedKnob should use RotaryHorizontalVerticalDrag");
            expect (knob.getTextBoxPosition() == juce::Slider::NoTextBox,
                    "AnimatedKnob should hide its text box");
        }

        void testEditorContainsEightAnimatedKnobs()
        {
            beginTest ("Plugin editor contains eight AnimatedKnob instances");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            const int knobCount = countAnimatedKnobsRecursive (editor);
            expect (knobCount == 8, "Editor should contain exactly eight AnimatedKnobs");
        }

        juce::AudioProcessorParameter* findParameterById (juce::AudioProcessor& processor,
                                                           const juce::String& paramId)
        {
            for (auto* param : processor.getParameters())
            {
                if (auto* paramWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
                {
                    if (paramWithId->getParameterID() == paramId)
                        return param;
                }
            }

            return nullptr;
        }

        void collectKnobsAtValue (juce::Component& parent, float value, int& count)
        {
            for (auto* child : parent.getChildren())
            {
                if (auto* knob = dynamic_cast<AnimatedKnob*> (child))
                {
                    if (std::abs (static_cast<float> (knob->getValue()) - value) < 1.0e-6f)
                        ++count;
                }

                collectKnobsAtValue (*child, value, count);
            }
        }

        void testAnimatedKnobBindsToParameter()
        {
            beginTest ("AnimatedKnob binds to an APVTS parameter via SliderAttachment");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* colorParameter = findParameterById (processor, "COLOR");
            jassert (colorParameter != nullptr);
            colorParameter->setValueNotifyingHost (0.75f);

            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            int knobsAtTarget = 0;
            for (auto* child : editor.getChildren())
                collectKnobsAtValue (*child, colorParameter->getValue(), knobsAtTarget);

            expect (knobsAtTarget > 0,
                    "At least one AnimatedKnob in the editor should follow the COLOR parameter");
        }

        void testInputMeterIsOnLeftSideBar()
        {
            beginTest ("Input meter sits in the left side bar above the input gain knob");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            constexpr int kRackEarWidth = 15;
            constexpr int kSideBarWidth = 55;

            auto& meter = editor.getInputMeter();
            auto& knob = editor.getInputGainKnob();

            expect (meter.getX() >= kRackEarWidth,
                    "Input meter should start after the left rack ear");
            expect (meter.getRight() <= kRackEarWidth + kSideBarWidth,
                    "Input meter should fit inside the left side bar");
            expect (meter.getBottom() <= knob.getY(),
                    "Input meter should sit above the input gain knob");
        }

        void testOutputMeterIsOnRightSideBar()
        {
            beginTest ("Output meter sits in the right side bar above the output gain knob");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            constexpr int kRackEarWidth = 15;
            constexpr int kSideBarWidth = 55;

            auto& meter = editor.getOutputMeter();
            auto& knob = editor.getOutputGainKnob();

            expect (meter.getX() >= editor.getWidth() - kRackEarWidth - kSideBarWidth,
                    "Output meter should start inside the right side bar");
            expect (meter.getRight() <= editor.getWidth() - kRackEarWidth,
                    "Output meter should end before the right rack ear");
            expect (meter.getBottom() <= knob.getY(),
                    "Output meter should sit above the output gain knob");
        }

        void testInputGainKnobBindsToParameter()
        {
            beginTest ("Input gain knob follows the INPUT_GAIN parameter");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* inputGain = findParameterById (processor, "INPUT_GAIN");
            jassert (inputGain != nullptr);

            const float targetDb = -12.0f;
            const float normalized = processor.getAPVTS().getParameterRange ("INPUT_GAIN").convertTo0to1 (targetDb);
            inputGain->setValueNotifyingHost (normalized);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (std::abs (static_cast<float> (editor.getInputGainKnob().getValue()) - targetDb) < 0.1f,
                    "Input gain knob should reflect -12 dB");
        }

        void testOutputGainKnobBindsToParameter()
        {
            beginTest ("Output gain knob follows the OUTPUT_GAIN parameter");

            AudioPluginAudioProcessor processor;
            AudioPluginAudioProcessorEditor editor (processor);

            auto* outputGain = findParameterById (processor, "OUTPUT_GAIN");
            jassert (outputGain != nullptr);

            const float targetDb = -36.0f;
            const float normalized = processor.getAPVTS().getParameterRange ("OUTPUT_GAIN").convertTo0to1 (targetDb);
            outputGain->setValueNotifyingHost (normalized);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

            expect (std::abs (static_cast<float> (editor.getOutputGainKnob().getValue()) - targetDb) < 0.1f,
                    "Output gain knob should reflect -36 dB");
        }

        void testMeterCalculatesRMS()
        {
            beginTest ("MeterComponent calculates RMS from a stereo buffer");

            juce::Component dummy;
            AnimationController controller (&dummy);
            MeterComponent meter (controller);
            meter.setSize (40, 200);

            juce::AudioBuffer<float> buffer (2, 64);
            buffer.clear();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                    buffer.setSample (ch, i, 0.5f);

            meter.updateFromBuffer (buffer);

            const float expectedDb = juce::Decibels::gainToDecibels (0.5f);
            expect (std::abs (meter.getRmsDb() - expectedDb) < 0.1f,
                    "RMS should be -6 dB for a 0.5 amplitude signal");
            expect (std::abs (meter.getPeakDb() - expectedDb) < 0.1f,
                    "Peak should match the constant amplitude");
        }

        void testMeterPeakHoldJumpsToPeak()
        {
            beginTest ("MeterComponent peak hold jumps to a transient peak and starts decay animation");

            juce::Component dummy;
            AnimationController controller (&dummy);
            MeterComponent meter (controller);
            meter.setSize (40, 200);

            juce::AudioBuffer<float> buffer (2, 32);
            buffer.clear();
            buffer.setSample (0, 4, 0.8f);
            buffer.setSample (1, 20, -0.8f);

            meter.updateFromBuffer (buffer);

            const float expectedDb = juce::Decibels::gainToDecibels (0.8f);
            expect (std::abs (meter.getAnimatedPeakDb() - expectedDb) < 0.1f,
                    "Animated peak hold should jump to the transient peak");
            expect (meter.isPeakHoldAnimating(),
                    "A decay animator should be running after a new peak");
        }
    };

    static AudioPluginTests synthortionTests;
}
