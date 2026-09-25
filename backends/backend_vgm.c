/* GPLv3 - RetroDJ Project - VGM Stream Writer Backend */

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

#include <stdio.h>
#include <stdint.h>
#include "../core/rdj_backend.h"

/* Minimal VGM Writer Implementation */
static FILE *vgm_file = NULL;

static void vgm_init(void) {
    /* Ready to open file on demand or log VGM output */
    vgm_file = NULL;
}

static void vgm_shutdown(void) {
    if (vgm_file) {
        fclose(vgm_file);
        vgm_file = NULL;
    }
}

static void vgm_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
    /* VGM commands for YM2612: 0x52 (port 0), 0x53 (port 1) */
    uint8_t cmd = (port & 1) ? 0x53 : 0x52;
    if (vgm_file) {
        fputc(cmd, vgm_file);
        fputc(reg, vgm_file);
        fputc(val, vgm_file);
    } else {
        printf("[VGM] CMD %02X Reg %02X Val %02X\n", cmd, reg, val);
    }
}

static void vgm_write_apu(uint16_t addr, uint8_t val) {
    /* VGM command for NES APU: 0xB4 (reg = addr & 0x7F) */
    if (vgm_file) {
        fputc(0xB4, vgm_file);
        fputc((uint8_t)(addr & 0x7F), vgm_file);
        fputc(val, vgm_file);
    } else {
        printf("[VGM] CMD B4 Reg %02X Val %02X\n", (uint8_t)(addr & 0x7F), val);
    }
}

rdj_backend_t backend_vgm = {
    .name = "VGM Writer Backend",
    .init = vgm_init,
    .shutdown = vgm_shutdown,
    .write_ym2612 = vgm_write_ym2612,
    .write_apu = vgm_write_apu
};
