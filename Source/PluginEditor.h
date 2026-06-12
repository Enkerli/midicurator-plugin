#pragma once
#include "PluginProcessor.h"
#include "../enkerli-juce/src/EnkerliWebView.h"
#include "../enkerli-juce/src/FileExport.h"
#include "../enkerli-juce/src/FileImport.h"
#include "../enkerli-juce/src/RuntimeInfo.h"
#include "BinaryDataWebUI.h"

class MIDIcuratorEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit MIDIcuratorEditor (MIDIcuratorProcessor& p)
        : juce::AudioProcessorEditor (p),
          proc (p),
          web (
              { { "/index.html", { BinaryData::index_html, BinaryData::index_htmlSize, "text/html; charset=utf-8" } } },
              {
                  { "uiReady", [this] (const juce::var&) { pageReady = true; sendSavedState(); } },
                  { "enkerliSetClip", [this] (const juce::var& v) { applyClip (v); } },
                  { "enkerliClearClip", [this] (const juce::var&) { proc.scheduler.clear(); } },
                  { "enkerliState", [this] (const juce::var& v) { proc.storeUiState (juce::JSON::toString (v)); } },
                  // Library: file-backed (IndexedDB is unreliable under juce://)
                  { "enkerliLoadLibrary", [this] (const juce::var&) { sendLibrary(); } },
                  { "enkerliStoreLibrary", [this] (const juce::var& v)
                      { MIDIcuratorProcessor::storeLibrary (v.getProperty ("json", "").toString()); } },
                  // WKWebView can't download/upload (TESTING.md) — native paths
                  { "enkerliSaveFile", [this] (const juce::var& v) { saveFile (v); } },
                  { "enkerliOpenFile", [this] (const juce::var& v) { openFile (v); } },
              })
    {
        addAndMakeVisible (web);
        web.start();
        setSize (1100, 760);
        setResizable (true, true);
        startTimerHz (10);
    }

    void resized() override { web.setBounds (getLocalBounds()); }

private:
    void sendSavedState()
    {
        const auto json = proc.loadUiState();
        if (json.isEmpty())
            return;
        const auto state = juce::JSON::parse (json);
        if (! state.isVoid())
            web.emit ("state", state);
    }

    void sendLibrary()
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty ("json", MIDIcuratorProcessor::loadLibrary());
        web.emit ("library", juce::var (obj));
    }

    void saveFile (const juce::var& v)
    {
        const auto name = v.getProperty ("name", "export.bin").toString()
                              .replaceCharacters ("/\\:", "---");
        juce::MemoryOutputStream decoded;
        if (! juce::Base64::convertFromBase64 (decoded, v.getProperty ("b64", "").toString()))
            return;
        enkerli::exportBytes (*this, name, decoded.getMemoryBlock());
    }

    void openFile (const juce::var& v)
    {
        const auto patterns = v.getProperty ("patterns", "*").toString();
        juce::Component::SafePointer<MIDIcuratorEditor> safe (this);
        enkerli::importFile (*this, patterns,
            [safe] (const juce::String& name, const juce::MemoryBlock& bytes)
            {
                if (safe == nullptr)
                    return;
                auto* obj = new juce::DynamicObject();
                obj->setProperty ("name", name);
                obj->setProperty ("b64", juce::Base64::toBase64 (bytes.getData(), bytes.getSize()));
                safe->web.emit ("fileOpened", juce::var (obj));
            });
    }

    void applyClip (const juce::var& v)
    {
        enkerli::MidiClipScheduler::Clip clip;
        clip.lengthBeats = static_cast<double> (v.getProperty ("lengthBeats", 0.0));
        clip.loop = static_cast<bool> (v.getProperty ("loop", true));
        if (auto* arr = v.getProperty ("notes", juce::var()).getArray())
        {
            for (const auto& n : *arr)
            {
                enkerli::ClipNote note;
                note.startBeat = static_cast<double> (n.getProperty ("startBeat", 0.0));
                note.lengthBeats = static_cast<double> (n.getProperty ("lengthBeats", 1.0));
                note.pitch = static_cast<int> (n.getProperty ("pitch", 60));
                note.velocity = static_cast<int> (n.getProperty ("velocity", 96));
                note.channel = juce::jlimit (1, 16, static_cast<int> (n.getProperty ("channel", 1)));
                clip.notes.push_back (note);
            }
        }
        // Strict host sync: the host's play button is the play button.
        proc.scheduler.setClip (std::move (clip));
    }

    void timerCallback() override
    {
        if (! pageReady)
            return;
        auto* obj = new juce::DynamicObject();
        obj->setProperty ("bpm", proc.transport.getBpm());
        obj->setProperty ("playing", proc.transport.isPlaying());
        obj->setProperty ("beat", proc.scheduler.getClipBeat());
        web.emit ("transport", juce::var (obj));

        if (++runtimeTick % 20 == 0) // every ~2 s at 10 Hz
            web.emit ("runtime", enkerli::RuntimeInfo::snapshot (proc));
    }

    MIDIcuratorProcessor& proc;
    enkerli::BridgedWebView web;
    bool pageReady = false;
    int runtimeTick = 0;
};

inline juce::AudioProcessorEditor* MIDIcuratorProcessor::createEditor()
{
    return new MIDIcuratorEditor (*this);
}
