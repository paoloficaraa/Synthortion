#include "Synthortion/SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    startTimerHz(30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark background with gradient
    juce::ColourGradient gradient(juce::Colour(0xff0a0f14), bounds.getX(), bounds.getY(),
                                  juce::Colour(0xff1a2332), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRect(bounds);

    // Border
    g.setColour(juce::Colour(0xff3a4556));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);

    // Grid lines
    g.setColour(juce::Colour(0xff2a3441).withAlpha(0.5f));

    // Horizontal grid lines (frequency bands)
    auto height = bounds.getHeight();
    for (int i = 1; i < 8; ++i)
    {
        auto y = bounds.getY() + (height * i) / 8.0f;
        g.drawHorizontalLine((int)y, bounds.getX() + 10, bounds.getRight() - 10);
    }

    // Vertical grid lines (time)
    auto width = bounds.getWidth();
    for (int i = 1; i < 10; ++i)
    {
        auto x = bounds.getX() + (width * i) / 10.0f;
        g.drawVerticalLine((int)x, bounds.getY() + 10, bounds.getBottom() - 10);
    }

    // Frequency labels
    g.setColour(juce::Colour(0xffb8c5d6).withAlpha(0.6f));
    g.setFont(10.0f);

    juce::StringArray freqLabels = {"20Hz", "250Hz", "500Hz", "1kHz", "2kHz", "5kHz", "10kHz", "20kHz"};
    for (int i = 0; i < freqLabels.size(); ++i)
    {
        auto y = bounds.getBottom() - 20 - (height - 40) * i / 7.0f;
        g.drawText(freqLabels[i], bounds.getX() + 5, (int)y - 6, 50, 12,
                   juce::Justification::centredLeft);
    }

    drawFrame(g);
}

void SpectrumAnalyzer::resized()
{
}

void SpectrumAnalyzer::timerCallback()
{
    if (nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }
}

void SpectrumAnalyzer::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            juce::zeromem(fftData, sizeof(fftData));
            memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void SpectrumAnalyzer::drawNextFrameOfSpectrum()
{
    window.multiplyWithWindowingTable(fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    auto mindB = -100.0f;
    auto maxdB = 0.0f;

    for (int i = 0; i < scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
        auto level = juce::jmap(juce::jlimit(mindB, maxdB,
                                             juce::Decibels::gainToDecibels(fftData[fftDataIndex]) -
                                                 juce::Decibels::gainToDecibels((float)fftSize)),
                                mindB, maxdB, 0.0f, 1.0f);
        scopeData[i] = level;
    }
}

void SpectrumAnalyzer::drawFrame(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(15);
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    // Draw spectrum
    juce::Path spectrumPath;
    bool started = false;

    for (int i = 1; i < scopeSize; ++i)
    {
        auto x = bounds.getX() + juce::jmap((float)i, 0.0f, (float)scopeSize, 0.0f, width);
        auto y = bounds.getBottom() - juce::jmap(scopeData[i], 0.0f, 1.0f, 0.0f, height);

        if (!started)
        {
            spectrumPath.startNewSubPath(x, y);
            started = true;
        }
        else
        {
            spectrumPath.lineTo(x, y);
        }
    }

    // Fill under the spectrum with gradient
    juce::Path filledPath = spectrumPath;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    juce::ColourGradient spectrumGradient(juce::Colour(0xffff6b35).withAlpha(0.3f),
                                          bounds.getCentreX(), bounds.getY(),
                                          juce::Colour(0xffff6b35).withAlpha(0.1f),
                                          bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(spectrumGradient);
    g.fillPath(filledPath);

    // Draw spectrum line
    g.setColour(juce::Colour(0xffff6b35));
    g.strokePath(spectrumPath, juce::PathStrokeType(2.0f));
}
