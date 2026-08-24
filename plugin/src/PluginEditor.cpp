#include "Synthortion/PluginEditor.h"
#include "Synthortion/PluginProcessor.h"
#include <unordered_map>

namespace synthortion
{
    static const char* getMimeForExtension (const juce::String& extension)
    {
        static const std::unordered_map<juce::String, const char*> mimeMap =
        {
            { { "htm"   }, "text/html"                },
            { { "html"  }, "text/html"                },
            { { "txt"   }, "text/plain"               },
            { { "jpg"   }, "image/jpeg"               },
            { { "jpeg"  }, "image/jpeg"               },
            { { "svg"   }, "image/svg+xml"            },
            { { "ico"   }, "image/x-icon"             },
            { { "json"  }, "application/json"         },
            { { "png"   }, "image/png"                },
            { { "css"   }, "text/css"                 },
            { { "js"    }, "text/javascript"          },
            { { "mjs"   }, "text/javascript"          },
            { { "woff"  }, "font/woff"                },
            { { "woff2" }, "font/woff2"               },
            { { "ttf"   }, "font/ttf"                 }
        };

        auto ext = extension.toLowerCase();
        if (const auto it = mimeMap.find (ext); it != mimeMap.end())
            return it->second;

        return "application/octet-stream";
    }

    juce::File AudioPluginAudioProcessorEditor::getDistDirectory()
    {
        const juce::File searchPaths[] =
        {
            juce::File::getCurrentWorkingDirectory().getChildFile ("ui/dist"),
            juce::File::getSpecialLocation (juce::File::currentApplicationFile).getChildFile ("../ui/dist"),
            juce::File::getSpecialLocation (juce::File::currentApplicationFile).getChildFile ("../Resources/dist"),
            juce::File::getSpecialLocation (juce::File::currentApplicationFile).getChildFile ("../Resources"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getChildFile ("ui/dist"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getChildFile ("../Resources/dist"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getChildFile ("../Resources"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getParentDirectory().getChildFile ("ui/dist"),
            juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("ui/dist"),
            juce::File ("C:/Program Files/Common Files/VST3/Synthortion.vst3/Contents/Resources/dist"),
            juce::File ("C:/Program Files/Common Files/VST3/Synthortion.vst3/Contents/Resources"),
            juce::File ("C:/Users/paolo/Synthortion/ui/dist")
        };

        for (const auto& path : searchPaths)
        {
            if (path.isDirectory() && path.getChildFile ("index.html").existsAsFile())
                return path;
        }

        return {};
    }

    std::optional<juce::WebBrowserComponent::Resource> AudioPluginAudioProcessorEditor::getResource (const juce::String& url)
    {
        auto distDir = getDistDirectory();
        if (! distDir.exists())
            return std::nullopt;

        auto relativePath = (url == "/" || url.isEmpty()) ? juce::String ("index.html")
                                                          : url.fromFirstOccurrenceOf ("/", false, false);

        // Remove any query string or hash
        if (relativePath.contains ("?"))
            relativePath = relativePath.upToFirstOccurrenceOf ("?", false, false);
        if (relativePath.contains ("#"))
            relativePath = relativePath.upToFirstOccurrenceOf ("#", false, false);

        auto file = distDir.getChildFile (relativePath);
        if (! file.existsAsFile())
            file = distDir.getChildFile ("index.html");

        if (! file.existsAsFile())
            return std::nullopt;

        juce::MemoryBlock block;
        if (! file.loadFileAsData (block))
            return std::nullopt;

        std::vector<std::byte> data ((const std::byte*) block.getData(),
                                     (const std::byte*) block.getData() + block.getSize());

        auto ext = file.getFileExtension().fromFirstOccurrenceOf (".", false, false);
        auto mime = juce::String (getMimeForExtension (ext));

        return juce::WebBrowserComponent::Resource { std::move (data), std::move (mime) };
    }

    static juce::WebBrowserComponent::Options createBrowserOptions (AudioPluginAudioProcessor& processor, AudioPluginAudioProcessorEditor& editor)
    {
        juce::ignoreUnused (processor);
        return juce::WebBrowserComponent::Options{}
            .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
#if JUCE_WINDOWS
            .withWinWebView2Options (juce::WebBrowserComponent::Options::WinWebView2{}
                .withUserDataFolder (juce::File::getSpecialLocation (juce::File::SpecialLocationType::tempDirectory).getChildFile ("Synthortion_WebView2")))
#endif
            .withNativeIntegrationEnabled (true)
            .withEventListener ("setParameter", [&editor] (const juce::var& data)
            {
                editor.handleSetParameter(data);
            })
            .withEventListener ("connect", [&editor] (const juce::var&)
            {
                editor.handleConnect();
            })
            .withResourceProvider ([&editor] (const juce::String& url)
            {
                return editor.getResource (url);
            });
    }

    AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
        : AudioProcessorEditor (&p),
          processorRef (p)
    {
        webView = std::make_unique<juce::WebBrowserComponent> (createBrowserOptions (processorRef, *this));

#if defined(SYNTHORTION_DEV_SERVER_URL)
        webView->goToURL (SYNTHORTION_DEV_SERVER_URL);
#else
        auto distDir = getDistDirectory();
        if (distDir.exists() && distDir.getChildFile ("index.html").exists())
        {
            webView->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());
        }
        else
        {
            webView->goToURL ("http://localhost:5173");
        }
#endif

        addAndMakeVisible (webView.get());

        for (auto* param : processorRef.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                processorRef.getAPVTS().addParameterListener (pWithId->paramID, this);
            }
        }

        setOpaque (true);
        setResizable (true, true);
        setResizeLimits (kMinWidth, kMinHeight, kMaxWidth, kMaxHeight);
        if (auto* boundsConstrainer = getConstrainer())
            boundsConstrainer->setFixedAspectRatio (kAspectRatio);
        setSize (kDefaultWidth, kDefaultHeight);
        const double sr = processorRef.getSampleRate() > 0.0 ? processorRef.getSampleRate() : 48000.0;
        spectrumAnalyzer.prepare (sr, 60.0f);
        analysisBuffer.fill (0.0f);
        startTimerHz (60);
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
        stopTimer();
        for (auto* param : processorRef.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                processorRef.getAPVTS().removeParameterListener (pWithId->paramID, this);
            }
        }
    }
    void AudioPluginAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
    {
        juce::Component::SafePointer<AudioPluginAudioProcessorEditor> safeThis (this);
        juce::MessageManager::callAsync ([safeThis, parameterID, newValue]()
        {
            if (safeThis == nullptr || safeThis->webView == nullptr)
                return;

            safeThis->sendParameterChange (parameterID, newValue);
        });
    }

    void AudioPluginAudioProcessorEditor::sendParameterChange (const juce::String& parameterID, float newValue)
    {
        juce::ignoreUnused (newValue);
        if (webView == nullptr)
            return;

        if (auto* param = processorRef.getAPVTS().getParameter (parameterID))
        {
            const float val = param->getValue();

            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            obj->setProperty ("id", parameterID);
            obj->setProperty ("value", val);

            webView->emitEventIfBrowserIsVisible ("parameterChange", juce::var (obj.get()));
        }
    }
    void AudioPluginAudioProcessorEditor::sendAllParameters()
    {
        // Replaced by init handshake, but kept for compatibility if needed.
        // Wait, issue says "eliminate ... string building" and use init.
        // Let's implement buildInitPayload and handleConnect instead.
    }

    juce::var AudioPluginAudioProcessorEditor::buildInitPayload()
    {
        juce::DynamicObject::Ptr payload = new juce::DynamicObject();
        payload->setProperty ("schemaVersion", 1);

        juce::Array<juce::var> parameters;
        for (auto* param : processorRef.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                juce::DynamicObject::Ptr paramObj = new juce::DynamicObject();
                paramObj->setProperty ("id", pWithId->paramID);
                paramObj->setProperty ("value", pWithId->getValue());
                paramObj->setProperty ("name", pWithId->getName(100));
                
                if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
                {
                    paramObj->setProperty ("min", floatParam->range.start);
                    paramObj->setProperty ("max", floatParam->range.end);
                }
                else if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
                {
                    paramObj->setProperty ("min", 0.0);
                    paramObj->setProperty ("max", static_cast<double>(choiceParam->choices.size() - 1));
                }
                else
                {
                    paramObj->setProperty ("min", 0.0);
                    paramObj->setProperty ("max", 1.0);
                }
                paramObj->setProperty ("defaultValue", pWithId->getDefaultValue());

                parameters.add (juce::var (paramObj.get()));
            }
        }
        payload->setProperty ("parameters", parameters);

        // Add uiPreferences
        juce::DynamicObject::Ptr uiPrefsObj = new juce::DynamicObject();
        auto uiPrefsTree = processorRef.getAPVTS().state.getChildWithName (UIPreferences::kNodeName);
        if (uiPrefsTree.isValid())
        {
            uiPrefsObj->setProperty (UIPreferences::kUiScale, uiPrefsTree.getProperty (UIPreferences::kUiScale, UIPreferences::kDefaultUiScale));
            uiPrefsObj->setProperty (UIPreferences::kSpectrumDecay, uiPrefsTree.getProperty (UIPreferences::kSpectrumDecay, UIPreferences::kDefaultSpectrumDecay));
            uiPrefsObj->setProperty (UIPreferences::kSkipBootSequence, uiPrefsTree.getProperty (UIPreferences::kSkipBootSequence, UIPreferences::kDefaultSkipBootSequence));
        }
        payload->setProperty ("uiPreferences", juce::var (uiPrefsObj.get()));

        return juce::var (payload.get());
    }

    void AudioPluginAudioProcessorEditor::handleConnect()
    {
        if (webView == nullptr) return;
        webView->emitEventIfBrowserIsVisible ("init", buildInitPayload());
    }

    void AudioPluginAudioProcessorEditor::handleSetParameter (const juce::var& data)
    {
        juce::var parsed = data;
        if (data.isString())
            parsed = juce::JSON::parse (data.toString());

        if (auto* obj = parsed.getDynamicObject())
        {
            auto parameterId = obj->hasProperty ("id")
                ? obj->getProperty ("id").toString()
                : obj->getProperty ("parameterId").toString();
                
            if (obj->hasProperty ("value"))
            {
                auto value = static_cast<float> (obj->getProperty ("value"));
                if (std::isfinite (value))
                {
                    value = juce::jlimit (0.0f, 1.0f, value);
                    if (auto* param = processorRef.getAPVTS().getParameter (parameterId))
                    {
                        param->setValueNotifyingHost (value);
                    }
                }
            }
        }
    }
    void AudioPluginAudioProcessorEditor::timerCallback()
    {
        auto& fifo = processorRef.getAudioFifo();
        const int ready = fifo.getNumReady();

        if (ready > 0)
        {
            if (ready >= SpectrumAnalyzer::kFftSize)
            {
                if (ready > SpectrumAnalyzer::kFftSize)
                {
                    fifo.discard (ready - SpectrumAnalyzer::kFftSize);
                }

                fifo.pop (analysisBuffer.data(), SpectrumAnalyzer::kFftSize);
            }
            else
            {
                std::copy (analysisBuffer.begin() + ready, analysisBuffer.end(), analysisBuffer.begin());
                fifo.pop (analysisBuffer.data() + (SpectrumAnalyzer::kFftSize - ready), ready);
            }

            const auto& magnitudes = spectrumAnalyzer.process (analysisBuffer.data(), SpectrumAnalyzer::kFftSize);
            sendSpectrumFrame (magnitudes);
        }
        else
        {
            const auto& magnitudes = spectrumAnalyzer.process (nullptr, 0);
            sendSpectrumFrame (magnitudes);
        }
        sendMeterFrame(processorRef.getMeterPeaks());
    }

    // emitBridgeEvent removed


    juce::var AudioPluginAudioProcessorEditor::buildSpectrumPayload (const std::array<float, SpectrumAnalyzer::kNumBands>& magnitudes)
    {
        juce::Array<juce::var> varArray;
        varArray.ensureStorageAllocated (SpectrumAnalyzer::kNumBands);
        for (float m : magnitudes) varArray.add (m);
        return juce::var(varArray);
    }

    void AudioPluginAudioProcessorEditor::sendSpectrumFrame (const std::array<float, SpectrumAnalyzer::kNumBands>& magnitudes)
    {
        if (webView != nullptr)
            webView->emitEventIfBrowserIsVisible("spectrumFrame", buildSpectrumPayload(magnitudes));
    }

    juce::var AudioPluginAudioProcessorEditor::buildMeterPayload(AudioPluginAudioProcessor::MeterPeaks peaks)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("input", peaks.input);
        obj->setProperty("output", peaks.output);
        return juce::var(obj.get());
    }

    void AudioPluginAudioProcessorEditor::sendMeterFrame(AudioPluginAudioProcessor::MeterPeaks peaks)
    {
        if (webView != nullptr)
            webView->emitEventIfBrowserIsVisible("meterFrame", buildMeterPayload(peaks));
    }

    void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::black);
    }

    void AudioPluginAudioProcessorEditor::resized()
    {
        if (webView != nullptr)
            webView->setBounds (getLocalBounds());
    }
}
