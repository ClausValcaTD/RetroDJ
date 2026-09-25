/* RDJ Sequencer Structures */
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

#ifndef RDJ_SEQUENCER_H
#define RDJ_SEQUENCER_H

#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "rdj_driver.h"

#define RDJ_NOTE_EMPTY   0x00
#define RDJ_NOTE_OFF     0xFF
#define RDJ_MAX_CHANNELS 8
#define RDJ_MAX_PATTERNS 64
#define RDJ_MAX_ROWS     64
#define RDJ_MAX_INSTR    32

typedef struct {
    uint8_t note;       /* 0=empty 1-127=MIDI 255=off */
    uint8_t instrument;
    uint8_t volume;     /* 0=empty 1-128 */
    uint8_t fx_cmd;     /* effect A-Z */
    uint8_t fx_val;     /* effect param 00-FF */
} rdj_row_t;

typedef struct {
    rdj_row_t rows[RDJ_MAX_ROWS][RDJ_MAX_CHANNELS];
    uint8_t   length;
    uint8_t   speed;
} rdj_pattern_t;

typedef struct {
    rdj_pattern_t patterns[RDJ_MAX_PATTERNS];
    uint8_t       order[RDJ_MAX_PATTERNS];
    uint8_t       order_length;
    uint8_t       bpm;
    uint8_t       speed;
    rdj_driver_t *driver;
} rdj_song_t;

#endif
