#include "Synthortion/AudioScopeRingBuffer.h"

namespace synthortion
{
    AudioScopeRingBuffer::AudioScopeRingBuffer (int capacitySamples)
        : inputChannels { ChannelRing { capacitySamples }, ChannelRing { capacitySamples } },
          outputChannels { ChannelRing { capacitySamples }, ChannelRing { capacitySamples } }
    {
        jassert (capacitySamples > 0);
    }

    void AudioScopeRingBuffer::writeInput (const juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return;

        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            const int sourceChannel = std::min (ch, buffer.getNumChannels() - 1);
            writeChannel (inputChannels[static_cast<size_t> (ch)], buffer.getReadPointer (sourceChannel), numSamples);
        }
    }

    void AudioScopeRingBuffer::writeOutput (const juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return;

        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            const int sourceChannel = std::min (ch, buffer.getNumChannels() - 1);
            writeChannel (outputChannels[static_cast<size_t> (ch)], buffer.getReadPointer (sourceChannel), numSamples);
        }
    }

    int AudioScopeRingBuffer::readInput (juce::AudioBuffer<float>& dest)
    {
        int minRead = 0;
        for (int ch = 0; ch < kNumChannels; ++ch)
            minRead = std::max (minRead, readChannel (inputChannels[static_cast<size_t> (ch)], dest.getWritePointer (ch), dest.getNumSamples()));
        return minRead;
    }

    int AudioScopeRingBuffer::readOutput (juce::AudioBuffer<float>& dest)
    {
        int minRead = 0;
        for (int ch = 0; ch < kNumChannels; ++ch)
            minRead = std::max (minRead, readChannel (outputChannels[static_cast<size_t> (ch)], dest.getWritePointer (ch), dest.getNumSamples()));
        return minRead;
    }

    void AudioScopeRingBuffer::writeChannel (ChannelRing& ring, const float* source, int numSamples)
    {
        jassert (source != nullptr);
        jassert (numSamples > 0);

        const int totalSize = ring.fifo.getTotalSize();
        const int samplesToWrite = std::min (numSamples, totalSize);
        const int freeSpace = ring.fifo.getFreeSpace();

        // Overwrite the oldest samples if the incoming block does not fit. This keeps the most
        // recent audio available for the UI thread, which is what an oscilloscope/meter needs.
        if (freeSpace < samplesToWrite)
        {
            const int toDiscard = samplesToWrite - freeSpace;
            int discardStart1 = 0, discardSize1 = 0, discardStart2 = 0, discardSize2 = 0;
            ring.fifo.prepareToRead (toDiscard, discardStart1, discardSize1, discardStart2, discardSize2);
            ring.fifo.finishedRead (discardSize1 + discardSize2);
        }

        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        ring.fifo.prepareToWrite (samplesToWrite, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy (source + (numSamples - samplesToWrite),
                       source + (numSamples - samplesToWrite) + size1,
                       ring.data.data() + start1);

        if (size2 > 0)
            std::copy (source + (numSamples - samplesToWrite) + size1,
                       source + numSamples,
                       ring.data.data() + start2);

        ring.fifo.finishedWrite (size1 + size2);
    }

    int AudioScopeRingBuffer::readChannel (ChannelRing& ring, float* dest, int maxSamples)
    {
        jassert (dest != nullptr);
        jassert (maxSamples > 0);

        const int available = ring.fifo.getNumReady();
        const int toRead = std::min (available, maxSamples);

        if (toRead <= 0)
            return 0;

        // To return the most recent window, read everything available into a scratch buffer and
        // copy the tail. Reading on the message thread is not realtime-critical.
        std::vector<float> scratch (static_cast<size_t> (available));

        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        ring.fifo.prepareToRead (available, start1, size1, start2, size2);

        std::copy (ring.data.data() + start1, ring.data.data() + start1 + size1, scratch.data());

        if (size2 > 0)
            std::copy (ring.data.data() + start2, ring.data.data() + start2 + size2, scratch.data() + size1);

        ring.fifo.finishedRead (size1 + size2);

        const int offset = available - toRead;
        std::copy (scratch.data() + offset, scratch.data() + offset + toRead, dest);

        return toRead;
    }
}
