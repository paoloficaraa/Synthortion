#include "Synthortion/PluginProcessor.h"
#include "Synthortion/PluginEditor.h"
#include <optional>

namespace synthortion
{
    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
        : AudioProcessorEditor(&p),
          processorRef(p),
          driveRelay("DRIVE"),
          mixRelay("MIX"),
          saturationTypeRelay("SATURATION_TYPE"),
          webView{juce::WebBrowserComponent::Options{}
                      .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                      .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
                                                  .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory))
                                                  .withBackgroundColour(juce::Colours::transparentBlack))
                      .withNativeIntegrationEnabled()
                      .withResourceProvider([this](const auto &url)
                                            { return getResource(url); })
                      .withOptionsFrom(driveRelay)
                      .withOptionsFrom(mixRelay)
                      .withOptionsFrom(saturationTypeRelay)},
          driveAttachment{*processorRef.apvts.getParameter("DRIVE"), driveRelay, nullptr},
          mixAttachment{*processorRef.apvts.getParameter("MIX"), mixRelay, nullptr},
          saturationTypeAttachment{*processorRef.apvts.getParameter("SATURATION_TYPE"), saturationTypeRelay, nullptr}
    {
        DBG("AudioPluginAudioProcessorEditor constructor called");

        addAndMakeVisible(webView);

        // Navigate to localhost in debug mode
#if JUCE_DEBUG
        webView.goToURL("http://localhost:5173");
        DBG("WebView navigating to localhost:5173");
#else
        webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif

        setResizable(false, false);
        setSize(1024, 650);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        webView.setBounds(getLocalBounds());
    }

    auto AudioPluginAudioProcessorEditor::getResource(const juce::String &url) const -> std::optional<Resource>
    {
        DBG("Resource requested: " + url);

        // For development, return empty to allow localhost navigation
        return std::nullopt;
    }
}
