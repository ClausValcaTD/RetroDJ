/* RDJ Unified Driver Interface */
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

#ifndef RDJ_DRIVER_H
#define RDJ_DRIVER_H

#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
#endif

typedef struct rdj_driver_t {
    const char  *name;
    const char  *chip;
    uint8_t      num_channels;
    uint8_t      num_instruments;

    void (*init)(uint32_t clock);
    void (*reset)(void);
    void (*note_on)(uint8_t ch, uint8_t note,
                    uint8_t inst, uint8_t vol);
    void (*note_off)(uint8_t ch);
    void (*write_reg)(uint16_t reg, uint8_t val);
    void (*shutdown)(void);
} rdj_driver_t;

extern rdj_driver_t ym2612_driver;
extern rdj_driver_t nes_2a03_driver;

#endif
