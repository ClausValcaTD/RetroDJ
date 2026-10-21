/* GPLv3 - RetroDJ Project - Game Boy Real Hardware Backend */

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

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    #include <genesis.h>
#elif defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    #include <gb/gb.h>
    #include <stdint.h>
    #include <stddef.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "core/rdj_backend.h"

#if !defined(__PORT_sm83) && !defined(__TARGET_gb) && !defined(GBDK)
static uint8_t gb_apu_shadow[0x40];
#endif

static void gb_hw_write_apu(uint16_t addr, uint8_t val) {
#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    *((volatile uint8_t*)addr) = val;
#else
    if (addr >= 0xFF10 && addr <= 0xFF3F) {
        gb_apu_shadow[addr - 0xFF10] = val;
    }
#endif
}

static void gb_hw_init(void) {
    /* Master enable APU: Bit 7 = 1 in NR52 ($FF26) */
    gb_hw_write_apu(0xFF26, 0x80);
    /* Enable all output channels to left and right terminals in NR51 ($FF25) */
    gb_hw_write_apu(0xFF25, 0xFF);
    /* Set max stereo master volume in NR50 ($FF24) */
    gb_hw_write_apu(0xFF24, 0x77);
}

static void gb_hw_shutdown(void) {
    /* Disable APU master enable */
    gb_hw_write_apu(0xFF26, 0x00);
}

static void gb_hw_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
    (void)port;
    (void)reg;
    (void)val;
}

static void gb_hw_wait_samples(uint16_t samples) {
    (void)samples;
}

static void gb_hw_wait_frame(void) {
#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    wait_vbl_done();
#endif
}

static const char gb_backend_name[] = "Game Boy Real Hardware Backend";

rdj_backend_t backend_gb_hw = {
    gb_backend_name,
    gb_hw_init,
    gb_hw_shutdown,
    gb_hw_write_ym2612,
    gb_hw_write_apu,
    gb_hw_wait_samples,
    gb_hw_wait_frame
};

#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
rdj_backend_t *rdj_active_backend = &backend_gb_hw;

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
        rdj_active_backend = &backend_gb_hw;
    }
}
#endif
