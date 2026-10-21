#!/usr/bin/env bash
set -e

# RetroDJ Headless Emulator Screenshot Verification Script
# Generates screenshots/gb_screenshot.png and screenshots/nes_screenshot.png

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CDir="$ROOT_DIR"

mkdir -p "$CDir/screenshots"

echo "=== 1. Building Game Boy target ==="
GBDKDIR="${GBDKDIR:-/opt/gbdk}"
if [ -d "$GBDKDIR" ]; then
    export GBDKDIR="$GBDKDIR"
fi
make -C "$CDir/targets/gameboy/" clean
make -C "$CDir/targets/gameboy/"

echo "=== 2. Building NES target ==="
make -C "$CDir/targets/nes/" clean
make -C "$CDir/targets/nes/"

echo "=== 3. Capturing Game Boy Screenshot (PyBoy) ==="
python3 -c "
import os
from pyboy import PyBoy

gbc_path = os.path.join('$CDir', 'targets/gameboy/out/rom.gbc')
out_path = os.path.join('$CDir', 'screenshots/gb_screenshot.png')

pyboy = PyBoy(gbc_path, window='null')
for _ in range(80):
    pyboy.tick()
pyboy.screen.image.save(out_path)
pyboy.stop()
print('Saved Game Boy screenshot to:', out_path)
"

echo "=== 4. Capturing NES Screenshot (FCEUX + Xvfb) ==="
LUA_SCRIPT=$(mktemp)
cat << 'LUA_EOF' > "$LUA_SCRIPT"
for i = 1, 120 do
    emu.frameadvance()
end
os.execute("import -window root " .. os.getenv("NES_OUT_PATH"))
os.exit()
LUA_EOF

export NES_OUT_PATH="$CDir/screenshots/nes_screenshot.png"
FCEUX_BIN=$(which fceux || echo "/usr/games/fceux")

xvfb-run -a "$FCEUX_BIN" --sound 0 --no-config 1 --loadlua "$LUA_SCRIPT" "$CDir/targets/nes/out/rom.nes"
rm -f "$LUA_SCRIPT"
echo "Saved NES screenshot to: $NES_OUT_PATH"

echo "=== Screenshots capture complete! ==="
