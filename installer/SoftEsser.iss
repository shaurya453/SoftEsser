; SoftEsser installer script, for the Inno Setup Compiler (https://jrsoftware.org/isinfo.php) -
; the installer builder JUCE's own docs recommend for packaging Windows plugins:
; https://juce.com/tutorials/tutorial_step_by_step_windows/
;
; Packages the Release VST3 and Standalone builds produced by CMake (see README.md's Building
; section) into a single signed-ready .exe installer. Run from the repo root, after building:
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
DefaultDirName={autopf}\{#AppPublisher}\{#AppName}
DisableDirPage=yes
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir=Output
OutputBaseFilename={#AppName}-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; VST3 plugins are 64-bit only on Windows, so the installer only targets 64-bit machines and
; always writes into the 64-bit Common Files\VST3 folder.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Required because the VST3/Program Files locations below need admin rights to write to.
PrivilegesRequired=admin
UninstallDisplayIcon={app}\{#AppName}.exe

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plug-in"; Types: full custom; Flags: fixed
Name: "standalone"; Description: "Standalone application"; Types: full custom

[Files]
; Standard per-machine VST3 location that every VST3 host scans by default.
Source: "{#BuildDir}\VST3\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
Source: "{#BuildDir}\Standalone\{#AppName}.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: standalone

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Components: standalone
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Tasks: desktopicon; Components: standalone

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional shortcuts:"; Components: standalone; Flags: unchecked

[Run]
Filename: "{app}\{#AppName}.exe"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent; Components: standalone
