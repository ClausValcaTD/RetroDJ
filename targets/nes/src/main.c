/* RetroDJ - NES Main Entry Point */
/* GPLv3 - RetroDJ Project */

/*
 * Copyright (C) 2025 RetroDJ Project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should receive a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ui_nes.h"
#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

int main(void) {
    /* Initialize Tracker UI */
    ui_init();

    /* Initialize and activate NES hardware backend */
    rdj_set_backend(&backend_nes_hw);

    /* Initialize 2A03 APU driver for standard NTSC clock 1789773 Hz */
    nes_2a03_driver.init(1789773);

    while (1) {
        nes_vblank_wait();
        ui_update();
        ui_render();
    }

    return 0;
}
