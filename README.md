# MIDIcurator plugin

The [music-suite](https://github.com/Enkerli/music-suite) MIDIcurator webapp
(`apps/MIDIcurator`) as an aumi MIDI processor on the
[enkerli-juce](https://github.com/Enkerli/enkerli-juce) foundation.

What the plugin adds over the browser app:

- **Host-synced auditioning** — "play" loads the selected clip into the
  lock-free `MidiClipScheduler`, looped; the host's play button is the play
  button (suite doctrine).
- **File-backed clip library** — IndexedDB is unreliable under the
  `juce://` scheme, so the library lives in `library.json` in the app data
  directory (standalone and AUv3 each have their own container for now).
- **Native import/export** — document picker / FileChooser in,
  share sheet / FileChooser out. No blob downloads, no `<input type=file>`
  (both kill or no-op in the plugin WebView; see enkerli-juce TESTING.md).

Browser-only features (hidden in plugin mode): Apple Loops database
(sql.js/WASM), sample-progression loading.

## Build

```sh
git clone --recurse-submodules https://github.com/Enkerli/midicurator-plugin
cd midicurator-plugin
node WebUI/build.mjs       # regenerate the UI bundle (needs music-suite checkout)
cmake -B build-macos -DCMAKE_BUILD_TYPE=Release && cmake --build build-macos -j 8
cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS   # then build/sign in Xcode
```

`WebUI/index.html` is committed, so the plugin builds without node.
Validation ladder: `auval -v aumi Mcur Enke`, pluginval strictness 8,
then real hosts on ≥2 iPads (see enkerli-juce/TESTING.md).
