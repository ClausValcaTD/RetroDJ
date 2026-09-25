/* GPLv3 - RetroDJ Project - Sega Genesis Real Hardware Backend */

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

#include <stdint.h>
#include "../core/rdj_backend.h"

/* Memory-mapped YM2612 hardware addresses on Sega Genesis / Mega Drive */
#define YM2612_ADDR_PORT0 ((volatile uint8_t*) 0xA04000)
#define YM2612_DATA_PORT0 ((volatile uint8_t*) 0xA04001)
#define YM2612_ADDR_PORT1 ((volatile uint8_t*) 0xA04002)
#define YM2612_DATA_PORT1 ((volatile uint8_t*) 0xA04003)

static void genesis_hw_init(void) {
}

static void genesis_hw_shutdown(void) {
}

static void genesis_hw_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
    if ((port & 1) == 0) {
        *YM2612_ADDR_PORT0 = reg;
        *YM2612_DATA_PORT0 = val;
    } else {
        *YM2612_ADDR_PORT1 = reg;
        *YM2612_DATA_PORT1 = val;
    }
}

static void genesis_hw_write_apu(uint16_t addr, uint8_t val) {
    (void)addr;
    (void)val;
}

rdj_backend_t backend_genesis_hw = {
    .name = "Genesis Real Hardware Backend",
    .init = genesis_hw_init,
    .shutdown = genesis_hw_shutdown,
    .write_ym2612 = genesis_hw_write_ym2612,
    .write_apu = genesis_hw_write_apu
};
