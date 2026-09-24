# SoftEsser

[![Build](https://github.com/shaurya453/SoftEsser/actions/workflows/build.yml/badge.svg)](https://github.com/shaurya453/SoftEsser/actions/workflows/build.yml)

A split-band de-esser plugin built with [JUCE](https://juce.com/). It splits the signal at a
target frequency into an untouched low band and a high band, tracks the high band's level, and
pulls only that band down once it crosses a threshold - useful for taming sibilance or other
narrow-band peaks without ducking the rest of the signal.

Built on [JUCE-Plugin-Boilerplate-by-Archie](https://github.com/archietiger/JUCE-Plugin-Biolerplate-by-Archie).

## Parameters

| Control       | Range          | Description                                                              |
|---------------|----------------|---------------------------------------------------------------------------|
| **Threshold** | -60 to 0 dB    | Level above which gain reduction begins.                                  |
| **Amount**    | 0 to 100 %     | How strongly the level above the threshold is pulled down.                |
| **Frequency** | 4 kHz to 10 kHz| Split point between the untouched low band and the de-essed high band.    |
| **Q**         | 0.3 to 6.0     | Resonance of the crossover split - higher is a sharper, more surgical split. |
| **Attack**    | 0.1 to 50 ms   | How quickly the de-esser responds once the high band crosses the threshold. |
| **Release**   | 5 to 300 ms    | How quickly the de-esser lets go once the high band falls back below the threshold. |
| **Mix**       | 0 to 100 %     | Blend between the processed (wet) and original (dry) signal.              |
| **Output**    | -12 to +12 dB  | Output level trim, applied after processing.                              |

Hover over any knob for a tooltip explaining what it does. The window is freely resizable and
keeps its proportions (knobs, labels, and fonts all scale together).

## Other controls

- **Listen** - solos the high band to the output, so you can hear exactly what's being de-essed
  while you tune Frequency/Q.
- **Gain reduction meter** - the vertical meter next to the knobs shows how much reduction is
  being applied in real time, with a dB tick scale and a live numeric readout.
- **A / B** - two in-memory snapshots of the current settings for quick before/after comparison.
  Click the inactive slot to recall it (storing the current settings into the slot you're
  leaving first); click the active slot again to re-store the current settings into it.
- **Presets** - save/load named snapshots of all parameters. A `Default` preset plus a few
  starting points (`Vocal - Light`, `Vocal - Aggressive`, `Broadcast`) are created automatically
  on first run, stored under `%APPDATA%/SoftEsser/Presets`.

## Formats

Builds as a **VST3** plugin, for Windows and macOS.

## Building

### Requirements

- CMake 3.23.1+
- A C++17 compiler (Visual Studio 2022 Build Tools or newer, on Windows; Xcode command line
  tools, on macOS)
- Git

### Steps

Windows:

```bash
git clone https://github.com/shaurya453/SoftEsser.git
cd SoftEsser
cmake -B build -A x64
cmake --build build --config Release
```

macOS:

```bash
git clone https://github.com/shaurya453/SoftEsser.git
cd SoftEsser
cmake -B build -G Xcode
cmake --build build --config Release
```

Built binary lands under:

```text
build/SoftEsser_artefacts/Release/VST3/SoftEsser.vst3
```

Every push to `main` also builds automatically via GitHub Actions
([workflow](.github/workflows/build.yml)) on both Windows and macOS, and the built VST3 for each
platform is attached as a downloadable artifact on each run. Every build is also validated with
[pluginval](https://github.com/Tracktion/pluginval) - see
[`docs/pluginval-report.md`](docs/pluginval-report.md) for the latest results and how to run it
locally.

### Building the installer

**Windows** - the VST3 build above is packaged into an installer with
[Inno Setup](https://jrsoftware.org/isinfo.php) - the installer builder [recommended by JUCE's own
docs](https://juce.com/tutorials/tutorial_step_by_step_windows/) - via
[`installer/SoftEsser.iss`](installer/SoftEsser.iss). After building Release above:

```bash
iscc installer\SoftEsser.iss
```

This writes `installer/Output/SoftEsser-Setup-<version>.exe`, which installs the VST3 into the
standard per-machine `Common Files\VST3` location, with an uninstaller. Requires admin rights to
run (it writes to Common Files). CI builds this too on every push and attaches it as the
`SoftEsser-Installer` artifact.

**macOS** - the VST3 build above is packaged into a `.pkg` installer via
[`installer/macos/build-pkg.sh`](installer/macos/build-pkg.sh), using Apple's own `pkgbuild` /
`productbuild` tools. After building Release above:

```bash
./installer/macos/build-pkg.sh
```

This writes `installer/Output/SoftEsser-Setup-<version>.pkg`, which installs the VST3 into the
standard per-machine `/Library/Audio/Plug-Ins/VST3` location. Requires admin rights to run. CI
builds this too on every push and attaches it as the `SoftEsser-Installer-macOS` artifact.

This `.pkg` is **not code-signed or notarized** (that requires a paid Apple Developer account and
signing certificates, which this repo doesn't have configured). macOS Gatekeeper will refuse to
open it with a plain double-click; to install it anyway, either right-click the `.pkg` and choose
**Open**, or run:

```bash
xattr -dr com.apple.quarantine SoftEsser-Setup-<version>.pkg
```

## Project Structure

```text
SoftEsser/
├── Source/
│   ├── PluginProcessor.h/.cpp     # DSP: band split, envelope follower, gain reduction, mix
│   ├── PluginEditor.h/.cpp        # GUI: layout, resizing, tooltips, A/B compare
│   ├── PresetManager.h/.cpp       # Save/load/delete presets, factory presets, A/B snapshots
│   ├── SoftEsserLookAndFeel.h     # Custom knob styling and font scaling
│   ├── GainReductionMeter.h       # Real-time gain reduction meter component
│   └── assets/bg.png              # Background art
├── modules/JUCE/                  # Trimmed JUCE framework (only the modules this plugin needs)
├── installer/
│   ├── SoftEsser.iss              # Windows installer (Inno Setup)
│   └── macos/build-pkg.sh         # macOS installer (.pkg)
├── .github/workflows/build.yml    # CI build (Windows + macOS)
└── CMakeLists.txt
```

## License

Free to use, modify, and build upon.
