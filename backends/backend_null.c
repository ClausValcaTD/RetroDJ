/* GPLv3 - RetroDJ Project - Null / Logging Backend */

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
#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "../core/rdj_backend.h"

static uint8_t ym_shadow[2][256];
static uint8_t apu_shadow[32];

static void null_init(void) {
    /* Initialize shadow registers */
    for (int p = 0; p < 2; p++) {
        for (int r = 0; r < 256; r++) ym_shadow[p][r] = 0;
    }
    for (int a = 0; a < 32; a++) apu_shadow[a] = 0;
}

static void null_shutdown(void) {
}

static void null_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
    ym_shadow[port & 1][reg] = val;
    printf("[YM2612] P%d R%02X = %02X\n", port, reg, val);
}

static void null_write_apu(uint16_t addr, uint8_t val) {
    if (addr >= 0x4000 && addr <= 0x4015) {
        apu_shadow[addr - 0x4000] = val;
    }
    printf("[2A03] $%04X = %02X\n", addr, val);
}

rdj_backend_t backend_null = {
    .name = "Null Logging Backend",
    .init = null_init,
    .shutdown = null_shutdown,
    .write_ym2612 = null_write_ym2612,
    .write_apu = null_write_apu
};

rdj_backend_t *rdj_active_backend = &backend_null;

void rdj_set_backend(rdj_backend_t *backend) {
    if (backend) {
        if (rdj_active_backend && rdj_active_backend->shutdown) {
            rdj_active_backend->shutdown();
        }
        rdj_active_backend = backend;
        if (rdj_active_backend->init) {
            rdj_active_backend->init();
        }
    } else {
        rdj_active_backend = &backend_null;
    }
}
