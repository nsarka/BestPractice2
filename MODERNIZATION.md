# BestPractice Modernization

This repository still contains the original Borland C++Builder 6 project files so
the legacy build remains available. The modern build starts by extracting the
portable audio-processing code into a CMake target that can be compiled with free
Windows toolchains.

## Current Free Toolchain Path

- CMake for project generation.
- Visual Studio Community/Build Tools or MinGW-w64/LLVM for the C++ compiler.
- Qt 6, wxWidgets, or another free native UI toolkit for the eventual VCL form
  replacement.
- NSIS can remain the installer technology because the project already has an
  installer script and NSIS is free software.

## What Builds First

The `bestpractice_core` CMake target contains the DSP and reader-independent
audio classes:

- `CWSOLA`
- `IIRButter`
- `SecondOrderSection`
- `TKaraokeProcessor`
- `TQMFBank`
- `TResampler`
- `TReader`

The VCL forms, `.dfm` files, Borland project file, and Borland-only Pascal helper
units are intentionally excluded from this first target.

## Modern Forms

The `qt/` directory contains a Qt 6 Widgets replacement for the VCL forms:

- `MainWindow` replaces `Main.dfm`.
- `AboutDialog` replaces `AboutBoxX.dfm`.
- `LanguageDialog` replaces `LangForm.dfm`.
- `ProgressDialog` replaces `ProgressForm.dfm`.

The Qt target is named `bestpractice_qt`. It is optional: CMake will skip it when
Qt 6 Widgets is not installed, while still building the portable core.

### Playback Status

The Qt application now uses Qt Multimedia for decoding/output and the original
BestPractice DSP classes for processing:

- WAV and MP3 file selection and decoding.
- Play and pause.
- Seek bar and elapsed-time display.
- Independent playback speed from 20% to 200%.
- Coarse and fine pitch shifting through WOLA/WSOLA plus resampling.
- Selectable WOLA, fast WSOLA, and high-quality WSOLA modes.
- Optional anti-alias filtering for upward pitch shifts.
- Karaoke vocal suppression with bass and treble pass-through controls.
- Output volume.
- A/B loop points and continuous looping.
- Saving the processed result as a 16-bit stereo WAV file.
- Playback completion and decode/output error messages.

Playback processing is now streamed through a bounded `QIODevice` into
`QAudioSink`. Changing speed, pitch, quality, filtering, or karaoke settings
restarts only the next unplayed DSP block at the current source position. It no
longer renders the full song before an adjustment becomes audible. Full-track
processing is reserved for WAV export.

Audio-CD discovery and extraction remain on the legacy-backend migration list.

## Configure And Build

```powershell
cmake -S . -B build
cmake --build build
```

If CMake cannot find a compiler, install one free Windows C++ toolchain and run
the commands again from a shell where that compiler is on `PATH`.

To build the modern forms, install Qt 6 with the Widgets module and reconfigure:

```powershell
cmake -S . -B build -DBESTPRACTICE_BUILD_QT_FORMS=ON -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64
cmake --build build --target bestpractice_qt
```

## Porting Strategy

1. Keep the legacy Borland project as a reference while moving portable code into
   CMake targets.
2. Replace the VCL UI with a new free toolkit UI. Qt 6 is the strongest fit for a
   native desktop app with designer tooling and mature multimedia APIs.
3. Replace WinMM/ACM playback and MP3 decoding with a maintained audio stack.
   Good candidates are Qt Multimedia, PortAudio plus libsndfile/minimp3, or
   another small decode/playback layer.
4. Convert `.dfm` forms into explicit UI layouts instead of trying to compile VCL
   resources directly.
5. Add focused tests around time stretching, resampling, filters, and loop
   boundary behavior before changing the playback engine.
