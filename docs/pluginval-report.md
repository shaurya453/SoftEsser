# pluginval Report

**Plugin:** SoftEsser v0.0.1 (VST3, Windows x64)
**Build:** commit `24a6a51` (CI run [35127050948](https://github.com/shaurya453/SoftEsser/actions/runs/35127050948))
**pluginval:** v1.0.4, strictness level 5, `--skip-gui-tests`, seed `0xda6998`
**Date:** 2026-09-22
**Result:** ✅ **SUCCESS** (exit code 0)

## What was tested

pluginval ran its full suite across all combinations of sample rate (44.1/48/96 kHz) and
block size (64/128/256/512/1024 samples):

| Test group               | Result | Notes                                                          |
|---------------------------|:------:|-----------------------------------------------------------------|
| Scan for plugins           | ✅ Pass | 1 plugin found: `TheMeloMix: SoftEsser v0.0.1`                  |
| Open plugin (cold)         | ✅ Pass | |
| Open plugin (warm)         | ✅ Pass | |
| Plugin info                 | ✅ Pass | Latency 0, tail length 0, no double-precision support declared |
| Plugin programs             | ✅ Pass | 0 programs (expected - presets are handled outside the VST2/3 "programs" concept, see `PresetManager`) |
| Audio processing            | ✅ Pass | All 15 sample-rate/block-size combinations                      |
| Plugin state (save/load)    | ✅ Pass | |
| Automation                  | ✅ Pass | All 15 combinations, with automation sub-blocks of 32 samples   |
| Automatable parameters      | ✅ Pass | |
| Basic bus / layout tests    | ✅ Pass | Main bus 2-in/2-out confirmed; mono/stereo/surround layouts enumerated correctly |

No warnings or errors were reported at any point in the run.

## How to reproduce locally

```bash
# 1. Download pluginval (Windows build)
curl -sL -o pluginval.zip https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip
unzip pluginval.zip

# 2. Build the plugin (see README.md's Building section), then run:
./pluginval.exe --strictness-level 5 --skip-gui-tests --timeout-ms 60000 \
  --output-dir pluginval-logs \
  --validate "build/SoftEsser_artefacts/Release/VST3/SoftEsser.vst3"
```

Exit code `0` means every test passed; pluginval also writes a timestamped `.txt` log into
`--output-dir` with the same content shown above.

## CI integration

Every push now runs this automatically - see the `Validate with pluginval` step in
[`.github/workflows/build.yml`](../.github/workflows/build.yml). It runs right after the plugin
is built, fails the workflow if pluginval reports any error, and uploads the log file as a
`pluginval-logs` artifact (kept even if validation fails, so a failure can be diagnosed from the
Actions UI without re-running locally).

`--skip-gui-tests` is used because CI runners don't reliably support opening editor windows
headlessly; this only skips tests that need to create a visible GUI window, not the DSP/state/
automation tests above. `--strictness-level 5` is pluginval's own recommended minimum for host
compatibility - it can be raised later (up to 10) for deeper fuzzing at the cost of longer runs.
