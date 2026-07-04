juce_add_plugin(Synthortion
    COMPANY_NAME "WavesCrate"
    BUNDLE_ID com.wavescrate.synthortion
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE MACP
    PLUGIN_CODE P729
    FORMATS AU VST3
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_add_binary_data(SynthortionResources
    SOURCES
        resources/BebasNeue-Regular.ttf
        resources/Montserrat-VariableFont_wght.ttf
)

target_sources(Synthortion
PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/AnalogLookAndFeel.cpp
    src/WarmDistortion.cpp
    src/PingPongDelay.cpp
    src/BitCrusher.cpp
    src/SynthortionChorus.cpp
)

target_include_directories(Synthortion
PRIVATE
    include
)

target_link_libraries(Synthortion
PRIVATE
    SynthortionResources
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
    gin
    gin_plugin
    gin_dsp
    gin_gui
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(Synthortion
PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
    pkg_check_modules(GTK3 REQUIRED gtk+-x11-3.0)
    pkg_check_modules(CURL REQUIRED libcurl)

    target_include_directories(Synthortion PRIVATE ${WEBKIT2GTK_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
    target_link_libraries(Synthortion PRIVATE ${WEBKIT2GTK_LIBRARIES} ${GTK3_LIBRARIES} ${CURL_LIBRARIES})
endif()

juce_add_console_app(SynthortionTests
    PRODUCT_NAME "Synthortion Tests"
)

juce_generate_juce_header(SynthortionTests)

target_sources(SynthortionTests
PRIVATE
    src/AnalogLookAndFeel.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/WarmDistortion.cpp
    src/PingPongDelay.cpp
    src/BitCrusher.cpp
    src/SynthortionChorus.cpp
    tests/AudioPluginTest.cpp
)

target_include_directories(SynthortionTests
PRIVATE
    include
)

target_compile_definitions(SynthortionTests
PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(SynthortionTests
PRIVATE
    SynthortionResources
    gin
    gin_plugin
    gin_dsp
    gin_gui
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_dsp
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
