#!/bin/bash
# OpenKO client (KnightOnLine) on macOS via WoWSilicon Wine + DXVK (KosmicKrisp Metal Vulkan)
# Kullanim:  ./run-mac-wine-dxvk.sh
set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
CLIENT_EXE="$PROJECT_ROOT/assets/Client/KnightOnLine.exe"

A="/Applications/WoWSilicon.app"
WINE="$A/Contents/Resources/Wine/bin/wine"
DXVK="$A/Contents/Resources/WoWSilicon-swift_WoWSiliconSwift.bundle/Patching/dxvk-kosmickrisp/d3d9.dll"

export WINEPREFIX="${WINEPREFIX:-/Users/berkgulher/.openko-wine}"
export WINEDLLOVERRIDES="mscoree=d;mshtml=d;d3d9=n"
export WINEDEBUG=-all
export DYLD_LIBRARY_PATH="$A/Contents/Resources/Wine/lib/external"
export VK_ICD_FILENAMES="$A/Contents/Resources/Wine/lib/vulkan/icd.d/KosmicKrisp_icd.json"
export DXVK_HUD="${DXVK_HUD:-fps}"

# DXVK d3d9'u 32-bit syswow64'e kur (yoksa)
if [ -f "$DXVK" ]; then
	cp -f "$DXVK" "$WINEPREFIX/drive_c/windows/syswow64/d3d9.dll"
fi

cd "$PROJECT_ROOT/assets/Client"
echo "[run] cikis dizini: $(pwd)"
exec "$WINE" "$CLIENT_EXE" "$@"
