# SoftEsser

[![Build](https://github.com/shaurya453/SoftEsser/actions/workflows/build.yml/badge.svg)](https://github.com/shaurya453/SoftEsser/actions/workflows/build.yml)

A frequency-selective de-esser plugin built with [JUCE](https://juce.com/). It band-pass filters
the signal around a target frequency, tracks that band's level, and pulls the overall signal down
once it crosses a threshold - useful for taming sibilance or other narrow-band peaks without
affecting the rest of the signal.

Built on [JUCE-Plugin-Boilerplate-by-Archie](https://github.com/archietiger/JUCE-Plugin-Biolerplate-by-Archie).

## Parameters

| Control       | Range          | Description                                                              |
|---------------|----------------|---------------------------------------------------------------------------|
| **Threshold** | -60 to 0 dB    | Level above which gain reduction begins.                                  |
| **Amount**    | 0 to 100 %     | How strongly the level above the threshold is pulled down.                |
| **Frequency** | 4 kHz to 10 kHz| Centre frequency of the band monitored for excess level (e.g. sibilance). |
| **Mix**       | 0 to 100 %     | Blend between the processed (wet) and original (dry) signal.              |
| **Output**    | -12 to +12 dB  | Output level trim, applied after processing.                              |

Hover over any knob for a tooltip explaining what it does. The window is freely resizable and
keeps its proportions (knobs, labels, and fonts all scale together).

## Formats

Builds as **VST3** and a **Standalone** application, for Windows.

## Building

### Requirements

- CMake 3.23.1+
- A C++17 compiler (Visual Studio 2022 Build Tools or newer, on Windows)
- Git

### Steps

```bash
git clone https://github.com/shaurya453/SoftEsser.git
cd SoftEsser
cmake -B build -A x64
cmake --build build --config Release
```

Built binaries land under:

```text
build/SoftEsser_artefacts/Release/VST3/SoftEsser.vst3
build/SoftEsser_artefacts/Release/Standalone/SoftEsser.exe
```

Every push to `main` also builds automatically via GitHub Actions
([workflow](.github/workflows/build.yml)), and the built VST3/Standalone binaries are attached as
downloadable artifacts on each run.

## Project Structure

```text
SoftEsser/
├── Source/
│   ├── PluginProcessor.h/.cpp     # DSP: filtering, envelope follower, gain reduction, mix
│   ├── PluginEditor.h/.cpp        # GUI: layout, resizing, tooltips
│   ├── SoftEsserLookAndFeel.h     # Custom knob styling and font scaling
│   └── assets/bg.png              # Background art
├── modules/JUCE/                  # Trimmed JUCE framework (only the modules this plugin needs)
├── .github/workflows/build.yml    # CI build
└── CMakeLists.txt
```

## License

Free to use, modify, and build upon.
