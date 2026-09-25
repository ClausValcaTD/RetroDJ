/* RetroDJ - Sega Genesis Main Entry Point */
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

#include "ui.h"
#include "../../../drivers/ym2612/ym2612_rdj.c"

#ifdef RDJ_GENESIS_HARDWARE
#include <genesis.h>
#endif

int main(void) {
    /* Initialize YM2612 driver for Genesis master clock ~7.67 MHz */
    ym2612_rdj_init(7670454);

    ui_init();

    while (1) {
        ui_update();
        ui_render();

#ifdef RDJ_GENESIS_HARDWARE
        SYS_doVBlankProcess();
#endif
    }

    return 0;
}
