# JUCE Plugin Boilerplate by Archie

A ready-to-use JUCE audio plugin development boilerplate for Visual Studio Code using CMake.

Built to simplify the setup process and eliminate the configuration headaches that usually come with setting up JUCE plugin development for the first time.

This boilerplate is designed as a reusable foundation for building audio plugins faster.

## Features

* Pre-configured JUCE integration
* CMake build system ready
* Visual Studio Code workflow ready
* Supports:

  * VST3
  * Standalone Application
* Optional assets pipeline (auto-detects assets when added)
* Clean project structure
* Ready for DSP implementation
* Beginner-friendly starter template

---

## Requirements

Before building the project, install:

### Required Software

* Visual Studio Code
* CMake
* Visual Studio Build Tools (Desktop Development with C++)
* Git

### Recommended VS Code Extensions

* C/C++
* CMake Tools
* CMake

---

## Project Structure

```text
JUCE-Plugin-Boilerplate/
│
├── Source/                  # Plugin source files
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
│
├── assets/                  # Optional assets folder
│
├── modules/
│   └── JUCE/                # JUCE framework
│
├── CMakeLists.txt           # Build configuration
├── .gitignore               # Git ignore rules
└── README.md                # Documentation
```

---

## Getting Started

Clone the repository:

```bash
git clone <repository-url>
```

Go into the project folder:

```bash
cd JUCE-Plugin-Boilerplate
```

Open the folder in Visual Studio Code.

---

## Building the Project

### Step 1 — Configure CMake

Open the Command Palette:

```text
Ctrl + Shift + P
```

Run:

```text
CMake: Configure
```

---

### Step 2 — Build

Open the Command Palette:

```text
Ctrl + Shift + P
```

Run:

```text
CMake: Build
```

---

### Step 3 — Run Standalone Application

Open the Command Palette:

```text
Ctrl + Shift + P
```

Run:

```text
CMake: Run Without Debugging
```

If everything is configured correctly, the standalone app should launch successfully.

---

## Plugin Output Paths

After building:

### VST3 Plugin

```text
build/<ProjectName>_artefacts/Debug/VST3/
```

### Standalone Application

```text
build/<ProjectName>_artefacts/Debug/Standalone/
```

---

## Adding Assets (Optional)

This boilerplate supports optional asset embedding.

Place assets inside:

```text
assets/
```

Example:

```text
assets/logo.png
assets/background.png
assets/impulse.wav
```

After adding assets:

Reconfigure CMake:

```text
CMake: Configure
```

Then rebuild.

Assets will automatically be compiled into the plugin binary.

---

## Creating a New Plugin from this Boilerplate

Open `CMakeLists.txt` and update:

```cmake
set(PROJECT_NAME "YourPluginName")
set(PRODUCT_NAME "YourPluginName")
set(COMPANY_NAME "YourCompany")
set(BUNDLE_ID "com.yourcompany.yourplugin")
```

Then rename:

* PluginProcessor files
* PluginEditor files
* Processor class names
* Editor class names

---

## Recommended Development Workflow

1. Build Standalone first
2. Test DSP logic
3. Build VST3 version
4. Test inside your DAW
5. Add parameters
6. Build UI
7. Add assets if needed
8. Final testing

---

## Common Issues

### JUCE folder not found

Make sure the JUCE folder exists here:

```text
modules/JUCE
```

---

### CMake configuration issues

Delete the build folder:

```text
build/
```

Then run:

```text
CMake: Configure
```

again.

---

### Plugin not showing in DAW

* Rescan plugins
* Verify VST3 output path
* Check your DAW plugin folder settings

---

## Built For

This boilerplate can be used as a foundation for:

* Gain Plugins
* EQ Plugins
* Compressors
* Distortion Plugins
* Delay Plugins
* Reverb Plugins
* Utility Plugins
* Synth Plugins

---

## Why This Exists

This boilerplate was created after spending multiple classes debugging setup issues during JUCE plugin development.

The goal is simple:

Set up once. Build faster. Focus on creating.

---

## Credits

Created by Archie.

Built for learning, building, and shipping audio plugins.

---

## License

Free to use, modify, and build upon.
