# BestPractice

BestPractice is a Windows audio practice tool for slowing music down, changing
pitch, looping difficult passages, and reducing centered vocals. This version
uses Qt 6 and the original BestPractice DSP algorithms through a modern CMake
build.

## Why?

BestPractice was made to give musicians a free practice tool for changing the
speed of recorded music without producing the pitch shift associated with
playing a record at the wrong speed. It can also change pitch without changing
tempo, making it possible to match a differently tuned instrument or move a song
into a more comfortable key.

The original project began because comparable tools commonly cost money, and
its author wanted to prove that a free alternative could be built.

You might use BestPractice to:

- Slow down a difficult solo until every note is clear.
- Transcribe music that moves too quickly at its original tempo.
- Tune a recording to your instrument instead of retuning your instrument.
- Change a song's key to better suit your voice.
- Slow down spoken examples while learning a language.
- Experiment with audio and digital signal processing.
- Discover whether a very serious ballad works at rave speed.

## Features

- WAV and MP3 playback with progressive decoding.
- Independent speed, semitone, and fine-pitch controls.
- WOLA (Weighted Overlap-Add) and WSOLA (Waveform Similarity Overlap-Add) time-stretch modes.
- Optional anti-alias filtering.
- Karaoke vocal suppression with bass and treble pass-through.
- Loop selectable regions of the song.
- Full-track or loop-region WAV and MP3 export.
- File drag-and-drop to load songs.
- Keyboard playback controls.

## Build

Install CMake, Qt 6 with Widgets and Multimedia, and a supported C++17 compiler.
For the Qt MinGW package on Windows:

```powershell
cmake -S . -B build-qt -G Ninja `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64 `
  -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build-qt --target bestpractice_qt
```

The portable DSP core can be built without Qt:

```powershell
cmake -S . -B build
cmake --build build
```

## Test

```powershell
cmake --build build-qt --target bestpractice_core_smoke bestpractice_audio_engine_smoke
```

## Installer

Install NSIS, then build the English-only per-user installer:

```powershell
cmake --build build-qt --target bestpractice_installer
```

The generated setup program is `build-qt/BestPractice-2.00-Setup.exe`.

## License

BestPractice is distributed under the GNU General Public License version 2. See
[`license.txt`](license.txt). This project is based on the
[original BestPractice software](https://bestpractice.sourceforge.net/) created
by Robert Moerland.
