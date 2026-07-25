cmake_minimum_required(VERSION 3.22)
project(PatoWebSocketVST VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(clap-juce-extensions)

set(COMMON_SOURCES
    Source/Sha1.h
    Source/Base64.h
    Source/PositionData.h
    Source/WebSocketServer.h
    Source/WebSocketServer.cpp
    Source/WebSocketProcessorBase.h
    Source/WebSocketProcessorBase.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

# ── Effect Plugin ──────────────────────────────────────────────
juce_add_plugin(PatoWebSocketEffect
    PRODUCT_NAME "Patos WebSocket VST Effect"
    COMPANY_NAME "Pato"
    PLUGIN_MANUFACTURER_CODE PtWS
    PLUGIN_CODE PWse
    FORMATS VST3 Standalone
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    SOURCES
        Source/EffectProcessor.h
        Source/EffectProcessor.cpp
        ${COMMON_SOURCES}
)

target_compile_definitions(PatoWebSocketEffect
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PatoWebSocketEffect
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
)

if(WIN32)
    target_link_libraries(PatoWebSocketEffect PRIVATE ws2_32)
endif()

clap_juce_extensions_plugin(TARGET PatoWebSocketEffect
    CLAP_ID "com.pato.websocket-effect"
    CLAP_FEATURES "utility" "analyser"
)

# ── Generator Plugin ───────────────────────────────────────────
juce_add_plugin(PatoWebSocketGenerator
    PRODUCT_NAME "Patos WebSocket VST Generator"
    COMPANY_NAME "Pato"
    PLUGIN_MANUFACTURER_CODE PtWS
    PLUGIN_CODE PWge
    FORMATS VST3 Standalone
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    SOURCES
        Source/GeneratorProcessor.h
        Source/GeneratorProcessor.cpp
        ${COMMON_SOURCES}
)

target_compile_definitions(PatoWebSocketGenerator
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PatoWebSocketGenerator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
)

if(WIN32)
    target_link_libraries(PatoWebSocketGenerator PRIVATE ws2_32)
endif()

clap_juce_extensions_plugin(TARGET PatoWebSocketGenerator
    CLAP_ID "com.pato.websocket-generator"
    CLAP_FEATURES "utility" "analyser"
)
