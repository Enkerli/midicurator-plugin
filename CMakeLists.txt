cmake_minimum_required(VERSION 3.22)
project(MIDIcurator VERSION 0.1.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# enkerli-juce is a git submodule — a plain `git clone` leaves it empty and
# the include below then fails with a cryptic "Unknown CMake command".
# Initialize it automatically so fresh clones just work.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        message(STATUS "enkerli-juce submodule is empty — running git submodule update --init")
        execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update --init enkerli-juce
                        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                        RESULT_VARIABLE _submod_rc)
    endif()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
        message(FATAL_ERROR "enkerli-juce submodule missing. Run: git submodule update --init")
    endif()
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
enkerli_resolve_juce()

enkerli_add_midi_effect_plugin(MIDIcurator
    PRODUCT_NAME "MIDIcurator"
    PLUGIN_CODE  Mcur
    VERSION      "${PROJECT_VERSION}"
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
