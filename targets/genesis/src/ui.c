/* RetroDJ - Sega Genesis VDP Text-Mode Tracker UI */
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

#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE) && !defined(__m68k__)
#include <stdio.h>
#endif

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
#include <genesis.h>
#endif

static uint8_t current_row = 0;
static uint8_t current_col = 0;

void ui_init(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    VDP_drawText("--- RetroDJ Tracker ---", 8, 1);
    VDP_drawText("CH1  CH2  CH3  CH4  CH5  CH6", 4, 3);
#else
    (void)current_row;
    (void)current_col;
#endif
}

void ui_update(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    uint16_t joy = JOY_readJoypad(JOY_1);
    if (joy & BUTTON_UP) {
        if (current_row > 0) current_row--;
    }
    if (joy & BUTTON_DOWN) {
        if (current_row < 63) current_row++;
    }
    if (joy & BUTTON_LEFT) {
        if (current_col > 0) current_col--;
    }
    if (joy & BUTTON_RIGHT) {
        if (current_col < 5) current_col++;
    }
#endif
}

void ui_render(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    char buf[32];
    for (uint8_t r = 0; r < 16; r++) {
        uint8_t display_row = (current_row + r) % 64;
        sprintf(buf, "%02X | C-4  ---  E-4  ---  G-4  ---", display_row);
        VDP_drawText(buf, 2, 5 + r);
    }
#endif
}
