cmake_minimum_required(VERSION 3.22)
project(MIDIcurator VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include("${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
enkerli_resolve_juce()

enkerli_add_midi_effect_plugin(MIDIcurator
    PRODUCT_NAME "MIDIcurator"
    PLUGIN_CODE  Mcur
    DESCRIPTION  "MIDI clip curation: import, tag, transform and audition clips host-synced"
    ICON_BIG     "${CMAKE_SOURCE_DIR}/Assets/icon_1024.png"
)

target_sources(MIDIcurator PRIVATE Source/Plugin.cpp)
if(APPLE)
    # Native save (share sheet) + open (document picker) paths for the
    # WebView UI (ObjC++; both compile empty on macOS).
    target_sources(MIDIcurator PRIVATE
        enkerli-juce/src/FileExport.mm
        enkerli-juce/src/FileImport.mm)
endif()

# The web UI: a single-file bundle generated from the music-suite monorepo
# app (apps/MIDIcurator) by WebUI/build.mjs. The generated file is
# committed so the plugin builds without node; regenerate after app changes.
juce_add_binary_data(MIDIcuratorWebUI
    HEADER_NAME BinaryDataWebUI.h
    NAMESPACE   BinaryData
    SOURCES     WebUI/index.html)

target_link_libraries(MIDIcurator PRIVATE
    MIDIcuratorWebUI
    juce::juce_audio_utils
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
