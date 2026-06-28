# BestPractice

BestPractice is a Windows audio practice tool for slowing music down, changing
pitch, looping difficult passages, and reducing centered vocals. This version
uses Qt 6 and the original BestPractice DSP algorithms through a modern CMake
build.

## Features

- WAV and MP3 playback with progressive decoding.
- Independent speed, semitone, and fine-pitch controls.
- WOLA and WSOLA time-stretch modes.
- Optional anti-alias filtering.
- Karaoke vocal suppression with bass and treble pass-through.
- Validated A/B looping.
- Cancellable full-track or loop-region WAV export.
- File drag-and-drop and keyboard playback controls.

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

The generated setup program is `build-qt/BestPractice-1.03-Setup.exe`.

## License

BestPractice is distributed under the GNU General Public License version 2. See
[`license.txt`](license.txt). This project is based on the
[original BestPractice software](https://bestpractice.sourceforge.net/) created
by Robert Moerland.
