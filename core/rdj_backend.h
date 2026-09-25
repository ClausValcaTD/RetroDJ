/* RDJ Hardware Abstraction Backend Interface */
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

#ifndef RDJ_BACKEND_H
#define RDJ_BACKEND_H

#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
#endif

typedef struct rdj_backend_t {
    const char *name;
    void (*init)(void);
    void (*shutdown)(void);
    void (*write_ym2612)(uint8_t port, uint8_t reg, uint8_t val);
    void (*write_apu)(uint16_t addr, uint8_t val);
} rdj_backend_t;

extern rdj_backend_t *rdj_active_backend;
extern rdj_backend_t backend_null;
extern rdj_backend_t backend_vgm;
extern rdj_backend_t backend_serial;
extern rdj_backend_t backend_genesis_hw;

void rdj_set_backend(rdj_backend_t *backend);

#endif
