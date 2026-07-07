#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace synthortion
{
    /** Lock-free dual-channel ring buffer for transferring audio from the audio thread to the UI.

        Maintains two independent stereo captures: pre-DSP (dry input after input gain) and
        post-DSP (wet output after the full effects chain and output gain). Writes are expected
        to happen on the realtime audio thread and are lock-free / allocation-free. Reads happen
        on the message/VBlank thread and may allocate temporary working storage.
    */
    class AudioScopeRingBuffer final
    {
    public:
        static constexpr int kNumChannels = 2;
        static constexpr int kDefaultCapacity = 4096;

        explicit AudioScopeRingBuffer (int capacitySamples = kDefaultCapacity);

        /** Write the first two channels of @p buffer as the pre-DSP capture. */
        void writeInput (const juce::AudioBuffer<float>& buffer) noexcept;

        /** Write the first two channels of @p buffer as the post-DSP capture. */
        void writeOutput (const juce::AudioBuffer<float>& buffer) noexcept;

        /** Read the most recent samples into @p dest, up to dest.getNumSamples().
            Returns the number of samples actually read per channel.
        */
        int readInput (juce::AudioBuffer<float>& dest);
        int readOutput (juce::AudioBuffer<float>& dest);

    private:
        struct ChannelRing
        {
            explicit ChannelRing (int capacitySamples)
                : fifo (capacitySamples),
                  data (static_cast<size_t> (capacitySamples))
            {
            }

            juce::AbstractFifo fifo;
            std::vector<float> data;
        };

        static void writeChannel (ChannelRing& ring, const float* source, int numSamples) noexcept;
        static int readChannel (ChannelRing& ring, float* dest, int maxSamples);

        std::array<ChannelRing, kNumChannels> inputChannels;
        std::array<ChannelRing, kNumChannels> outputChannels;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioScopeRingBuffer)
    };
}
