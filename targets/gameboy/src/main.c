/* RetroDJ - Game Boy Main Entry Point */
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

#include "ui_gb.h"
#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

extern rdj_driver_t gb_sound_driver;
extern rdj_backend_t backend_gb_hw;

int main(void) {
    /* Initialize and activate Game Boy hardware backend */
    rdj_set_backend(&backend_gb_hw);

    /* Initialize Game Boy sound driver */
    gb_sound_driver.init(0);

    /* Initialize Tracker UI */
    ui_init();

    while (1) {
        wait_vbl_done();
        ui_update();
        ui_render();
    }

    return 0;
}
