#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

namespace synthortion
{
    /**
     * Lock-free ring buffer for capturing post-FX mono audio frames without heap allocations.
     * Uses a 16k-sample capacity (16384 samples) backed by juce::AbstractFifo.
     */
    class AudioCaptureFifo
    {
    public:
        static constexpr int kCapacity = 16384;

        AudioCaptureFifo() noexcept
            : fifo (kCapacity)
        {
            buffer.fill (0.0f);
        }

        /** Pushes audio buffer downmixed to mono ((L + R) * 0.5) into the ring buffer. Zero heap allocation. */
        void push (const juce::AudioBuffer<float>& input) noexcept
        {
            const auto numChannels = input.getNumChannels();
            const auto numSamples = input.getNumSamples();
            if (numChannels <= 0 || numSamples <= 0)
                return;

            push (input.getArrayOfReadPointers(), numChannels, numSamples);
        }

        /** Pushes raw channel pointers downmixed to mono into the ring buffer. */
        void push (const float* const* channelData, int numChannels, int numSamples) noexcept
        {
            if (channelData == nullptr || numChannels <= 0 || numSamples <= 0)
                return;

            int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
            fifo.prepareToWrite (numSamples, start1, size1, start2, size2);

            const int totalToWrite = size1 + size2;
            if (totalToWrite <= 0)
                return;

            auto writeMono = [channelData, numChannels] (float* dest, int offset, int count)
            {
                if (numChannels == 1)
                {
                    const auto* src = channelData[0] + offset;
                    for (int i = 0; i < count; ++i)
                        dest[i] = src[i];
                }
                else
                {
                    const auto* left = channelData[0] + offset;
                    const auto* right = channelData[1] + offset;
                    for (int i = 0; i < count; ++i)
                        dest[i] = (left[i] + right[i]) * 0.5f;
                }
            };

            if (size1 > 0)
                writeMono (buffer.data() + start1, 0, size1);

            if (size2 > 0)
                writeMono (buffer.data() + start2, size1, size2);

            fifo.finishedWrite (totalToWrite);
        }

        /**
         * Pulls up to numSamples from the ring buffer into destination.
         * Returns the actual number of samples copied.
         */
        int pop (float* destination, int numSamples) noexcept
        {
            if (destination == nullptr || numSamples <= 0)
                return 0;

            int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
            fifo.prepareToRead (numSamples, start1, size1, start2, size2);

            const int totalToRead = size1 + size2;
            if (totalToRead <= 0)
                return 0;

            if (size1 > 0)
                std::copy_n (buffer.data() + start1, size1, destination);

            if (size2 > 0)
                std::copy_n (buffer.data() + start2, size2, destination + size1);

            fifo.finishedRead (totalToRead);
            return totalToRead;
        }

        /** Returns the number of samples available to be read. */
        int getNumReady() const noexcept
        {
            return fifo.getNumReady();
        }

        /** Returns the remaining space available for writing. */
        int getFreeSpace() const noexcept
        {
            return fifo.getFreeSpace();
        }
        /**
         * Discards up to numSamples from the ring buffer without copying memory or allocating.
         * Returns the number of samples discarded.
         */
        int discard (int numSamples) noexcept
        {
            if (numSamples <= 0)
                return 0;

            int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
            fifo.prepareToRead (numSamples, start1, size1, start2, size2);

            const int totalToDiscard = size1 + size2;
            if (totalToDiscard <= 0)
                return 0;

            fifo.finishedRead (totalToDiscard);
            return totalToDiscard;
        }


        /** Returns the total capacity (16384 samples). */
        int getCapacity() const noexcept
        {
            return kCapacity;
        }

        /** Resets the read/write pointers and clears buffer. */
        void reset() noexcept
        {
            fifo.reset();
            buffer.fill (0.0f);
        }

    private:
        juce::AbstractFifo fifo;
        std::array<float, kCapacity> buffer {};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioCaptureFifo)
    };
}
