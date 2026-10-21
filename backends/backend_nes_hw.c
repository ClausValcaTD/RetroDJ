/* GPLv3 - RetroDJ Project - NES Real Hardware Backend */

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
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "core/rdj_backend.h"

#if !defined(__NES__) && !defined(__CC65__)
static uint8_t nes_apu_shadow[0x20];
#endif

static void nes_hw_write_apu(uint16_t addr, uint8_t val) {
#if defined(__NES__) || defined(__CC65__)
    *((volatile uint8_t*)addr) = val;
#else
    if (addr >= 0x4000 && addr < 0x4020) {
        nes_apu_shadow[addr - 0x4000] = val;
    }
#endif
}

static void nes_hw_init(void) {
    /* Enable APU channels (Square 1, Square 2, Triangle, Noise) via $4015 */
    nes_hw_write_apu(0x4015, 0x0F);
    /* Disable frame IRQ via $4017 */
    nes_hw_write_apu(0x4017, 0x40);
}

static void nes_hw_shutdown(void) {
    /* Disable all APU channels */
    nes_hw_write_apu(0x4015, 0x00);
}

static void nes_hw_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
    (void)port;
    (void)reg;
    (void)val;
}

static void nes_hw_wait_samples(uint16_t samples) {
    (void)samples;
}

static void nes_hw_wait_frame(void) {
}

static const char nes_backend_name[] = "NES Real Hardware Backend";

rdj_backend_t backend_nes_hw = {
    nes_backend_name,
    nes_hw_init,
    nes_hw_shutdown,
    nes_hw_write_ym2612,
    nes_hw_write_apu,
    nes_hw_wait_samples,
    nes_hw_wait_frame
};

#if defined(__NES__) || defined(__CC65__)
rdj_backend_t *rdj_active_backend = &backend_nes_hw;

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
        rdj_active_backend = &backend_nes_hw;
    }
}
#endif
