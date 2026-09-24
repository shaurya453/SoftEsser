#!/bin/bash
# SoftEsser macOS installer builder.
#
# Packages the Release VST3 build produced by CMake (see README.md's Building section) into a
# single .pkg installer that drops it into the standard per-machine VST3 folder
# (/Library/Audio/Plug-Ins/VST3). Run from the repo root, after building:
#
#   cmake -B build -G Xcode
#   cmake --build build --config Release
#   ./installer/macos/build-pkg.sh
#
# The installer is written to installer/Output/SoftEsser-Setup-<version>.pkg. CI builds this
# automatically on every push - see .github/workflows/build.yml.
#
# Note: this package is NOT code-signed or notarized (no Apple Developer certificate is
# configured in this repo). On a fresh Mac, Gatekeeper will block the installer until the user
# right-clicks it and chooses "Open", or runs:
#   xattr -dr com.apple.quarantine SoftEsser-Setup-<version>.pkg

set -euo pipefail

APP_NAME="SoftEsser"
APP_VERSION="0.0.1"
IDENTIFIER="com.themelomix.SoftEsser.vst3.pkg"
INSTALL_LOCATION="/Library/Audio/Plug-Ins/VST3"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VST3_SRC="$REPO_ROOT/build/SoftEsser_artefacts/Release/VST3/${APP_NAME}.vst3"
WORK_DIR="$REPO_ROOT/installer/macos/.build"
OUTPUT_DIR="$REPO_ROOT/installer/Output"

if [ ! -d "$VST3_SRC" ]; then
    echo "error: VST3 not found at $VST3_SRC - build it first (cmake --build build --config Release)" >&2
    exit 1
fi

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/root${INSTALL_LOCATION}" "$OUTPUT_DIR"

cp -R "$VST3_SRC" "$WORK_DIR/root${INSTALL_LOCATION}/"

pkgbuild \
    --root "$WORK_DIR/root" \
    --identifier "$IDENTIFIER" \
    --version "$APP_VERSION" \
    --install-location "/" \
    "$WORK_DIR/${APP_NAME}-component.pkg"

cat > "$WORK_DIR/distribution.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>${APP_NAME}</title>
    <organization>com.themelomix</organization>
    <domains enable_localSystem="true"/>
    <options customize="never" require-scripts="false" rootVolumeOnly="true"/>
    <volume-check>
        <allowed-os-versions>
            <os-version min="10.13"/>
        </allowed-os-versions>
    </volume-check>
    <choices-outline>
        <line choice="default">
            <line choice="${IDENTIFIER}"/>
        </line>
    </choices-outline>
    <choice id="default"/>
    <choice id="${IDENTIFIER}" visible="false">
        <pkg-ref id="${IDENTIFIER}"/>
    </choice>
    <pkg-ref id="${IDENTIFIER}" version="${APP_VERSION}" onConclusion="none">${APP_NAME}-component.pkg</pkg-ref>
</installer-gui-script>
EOF

productbuild \
    --distribution "$WORK_DIR/distribution.xml" \
    --package-path "$WORK_DIR" \
    "$OUTPUT_DIR/${APP_NAME}-Setup-${APP_VERSION}.pkg"

echo "Wrote $OUTPUT_DIR/${APP_NAME}-Setup-${APP_VERSION}.pkg"
