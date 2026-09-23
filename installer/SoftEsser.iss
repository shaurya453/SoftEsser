; SoftEsser installer script, for the Inno Setup Compiler (https://jrsoftware.org/isinfo.php) -
; the installer builder JUCE's own docs recommend for packaging Windows plugins:
; https://juce.com/tutorials/tutorial_step_by_step_windows/
;
; Packages the Release VST3 build produced by CMake (see README.md's Building section) into a
; single .exe installer that drops it into the standard per-machine VST3 folder. Run from the
; repo root, after building:
;
;   cmake -B build -A x64
;   cmake --build build --config Release
;   iscc installer\SoftEsser.iss
;
; The installer is written to installer\Output\SoftEsser-Setup-<version>.exe. CI builds this
; automatically on every push - see .github/workflows/build.yml.

#define AppName "SoftEsser"
#define AppVersion "0.0.1"
#define AppPublisher "TheMeloMix"
#define AppURL "https://github.com/shaurya453/SoftEsser"
#define BuildDir "..\build\SoftEsser_artefacts\Release"

[Setup]
; Fixed GUID identifying this application across versions, so upgrades/uninstalls work
; correctly - do not change this once the installer has shipped.
AppId={{7C1E9E2A-1F6B-4C7B-9C77-1B8F3E9A5D02}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
VersionInfoVersion={#AppVersion}
; No app folder to pick - the only thing this installer does is drop the VST3 into the standard
; system location below.
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableFinishedPage=no
CreateAppDir=no
OutputDir=Output
OutputBaseFilename={#AppName}-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; VST3 plugins are 64-bit only on Windows, so the installer only targets 64-bit machines and
; always writes into the 64-bit Common Files\VST3 folder.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Required because Common Files\VST3 needs admin rights to write to.
PrivilegesRequired=admin

[Files]
; Standard per-machine VST3 location that every VST3 host scans by default.
Source: "{#BuildDir}\VST3\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#AppName}.vst3"
