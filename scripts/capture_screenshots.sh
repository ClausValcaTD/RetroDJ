#!/usr/bin/env bash
set -e

mkdir -p screenshots
CUR_DIR="$(pwd)"

echo "Capturing NES screenshot using fceux under Xvfb..."
cat << LUAEOF > /tmp/nes_capture.lua
for i = 1, 30 do
    emu.frameadvance()
end
local f = io.open("${CUR_DIR}/screenshots/nes_screenshot.ppm", "w")
f:write("P3\n256 224\n255\n")
for y = 0, 223 do
    for x = 0, 255 do
        local r, g, b = emu.getscreenpixel(x, y)
        f:write(string.format("%d %d %d ", r or 0, g or 0, b or 0))
    end
    f:write("\n")
end
f:close()
LUAEOF

xvfb-run -a /usr/games/fceux --sound 0 --loadlua /tmp/nes_capture.lua targets/nes/out/rom.nes >/dev/null 2>&1 &
FCEUX_PID=$!
sleep 5
kill -9 $FCEUX_PID 2>/dev/null || true

python3 -c "
from PIL import Image
img = Image.open('screenshots/nes_screenshot.ppm')
img.save('screenshots/nes_screenshot.png')
"
rm -f screenshots/nes_screenshot.ppm /tmp/nes_capture.lua

echo "Capturing Game Boy screenshot using PyBoy..."
python3 - << 'PYEOF'
from pyboy import PyBoy
from PIL import ImageOps

pyboy = PyBoy('targets/gameboy/out/rom.gbc', window="null")
for _ in range(60):
    pyboy.tick()

img = pyboy.screen.image
img_contrast = ImageOps.autocontrast(img.convert('RGB'))
img_contrast.save('screenshots/gb_screenshot.png')
pyboy.stop()
print("PyBoy Game Boy screenshot saved successfully.")
PYEOF

echo "Screenshot capture complete."
