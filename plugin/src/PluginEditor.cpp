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
        return juce::WebBrowserComponent::Options{}
            .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
#if JUCE_WINDOWS
            .withWinWebView2Options (juce::WebBrowserComponent::Options::WinWebView2{}
                .withUserDataFolder (juce::File::getSpecialLocation (juce::File::SpecialLocationType::tempDirectory).getChildFile ("Synthortion_WebView2")))
#endif
            .withNativeIntegrationEnabled (true)
            .withEventListener ("setParameter", [&processor] (const juce::var& data)
            {
                juce::var parsed = data;
                if (data.isString())
                    parsed = juce::JSON::parse (data.toString());

                if (auto* obj = parsed.getDynamicObject())
                {
                    auto parameterId = obj->hasProperty ("parameterId")
                        ? obj->getProperty ("parameterId").toString()
                        : obj->getProperty ("id").toString();
                    auto value = static_cast<float> (obj->getProperty ("value"));
                    if (auto* param = processor.getAPVTS().getParameter (parameterId))
                    {
                        param->setValueNotifyingHost (value);
                    }
                }
            })
            .withEventListener ("connect", [&editor] (const juce::var&)
            {
                editor.sendAllParameters();
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
    }

    AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
    {
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
            obj->setProperty ("parameterId", parameterID);
            obj->setProperty ("id", parameterID);
            obj->setProperty ("value", val);

            webView->emitEventIfBrowserIsVisible ("parameterChange", juce::var (obj.get()));
            webView->evaluateJavascript (
                "if (window.__SYNTORTION_BRIDGE__ && window.__SYNTORTION_BRIDGE__.onParameterChange) { "
                "window.__SYNTORTION_BRIDGE__.onParameterChange('" + parameterID + "', " + juce::String (val) + "); }"
            );
        }
    }
    void AudioPluginAudioProcessorEditor::sendAllParameters()
    {
        if (webView == nullptr)
            return;

        for (auto* param : processorRef.getParameters())
        {
            if (auto* pWithId = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
            {
                sendParameterChange (pWithId->paramID, pWithId->getValue());
            }
        }
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
