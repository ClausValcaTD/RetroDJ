/* GPLv3 - RetroDJ Project - Hardware Serial Communication Backend */

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

#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
#include <stdio.h>
#endif

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "core/rdj_backend.h"

/* Serial Port Configuration String */
static const char *serial_config = "/dev/ttyUSB0:115200";

static void serial_init(void) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL] Initializing interface %s...\n", serial_config);
#else
    (void)serial_config;
#endif
}

static void serial_shutdown(void) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL] Closing serial connection...\n");
#endif
}

static void serial_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL TX] YM2612 Port %d Reg %02X Val %02X\n", port, reg, val);
#else
    (void)port; (void)reg; (void)val;
#endif
}

static void serial_write_apu(uint16_t addr, uint8_t val) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL TX] APU Addr %04X Val %02X\n", addr, val);
#else
    (void)addr; (void)val;
#endif
}

static void serial_wait_samples(uint16_t samples) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL TX] Wait %u samples\n", samples);
#else
    (void)samples;
#endif
}

static void serial_wait_frame(void) {
#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE)
    printf("[SERIAL TX] Wait 1 frame\n");
#endif
}

rdj_backend_t backend_serial = {
    .name = "Serial Hardware Backend",
    .init = serial_init,
    .shutdown = serial_shutdown,
    .write_ym2612 = serial_write_ym2612,
    .write_apu = serial_write_apu,
    .wait_samples = serial_wait_samples,
    .wait_frame = serial_wait_frame
};
