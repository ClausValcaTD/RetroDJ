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
#include <string.h>
#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "../core/rdj_backend.h"

/* Standard VGM 1.51 Header Structure (64 bytes) */
typedef struct {
    uint8_t  ident[4];        /* "Vgm " = 0x56 0x67 0x6D 0x20 */
    uint32_t eof_offset;     /* File length - 4 */
    uint32_t version;        /* Version 1.51 = 0x00000151 */
    uint32_t sn76489_clock;  /* 0 */
    uint32_t ym2413_clock;   /* 0 */
    uint32_t gd3_offset;     /* 0 (no GD3 tag) */
    uint32_t total_samples;  /* Total sample count */
    uint32_t loop_offset;    /* 0 */
    uint32_t loop_samples;   /* 0 */
    uint32_t rate;           /* 50 or 60 Hz rate */
    uint16_t sn76489_feedback;
    uint8_t  sn76489_shift_reg_width;
    uint8_t  sn76489_flags;
    uint32_t ym2612_clock;   /* YM2612 clock: 7670454 */
    uint32_t ym2151_clock;   /* 0 */
    uint32_t data_offset;    /* Offset to data - 0x34 (0x0000000C) */
    uint32_t reserved1;
    uint32_t reserved2;
} vgm_header_t;

static FILE *vgm_file = NULL;

void vgm_open(const char *filename) {
    if (vgm_file) {
        fclose(vgm_file);
        vgm_file = NULL;
    }
    if (filename) {
        vgm_file = fopen(filename, "wb");
        if (vgm_file) {
            vgm_header_t header;
            memset(&header, 0, sizeof(header));
            header.ident[0] = 'V';
            header.ident[1] = 'g';
            header.ident[2] = 'm';
            header.ident[3] = ' ';
            header.version = 0x00000151;
            header.ym2612_clock = 7670454;
            header.data_offset = 0x0000000C; /* Data starts at 0x40 relative to 0x34 */

            fwrite(&header, sizeof(header), 1, vgm_file);
        }
    }
}

static void vgm_init(void) {
    if (!vgm_file) {
        /* Default output file if none opened explicitly */
        vgm_open("output.vgm");
    }
}

static void vgm_shutdown(void) {
    if (vgm_file) {
        /* Write VGM end-of-sound-data command */
        fputc(0x66, vgm_file);

        /* Patch total file size in header */
        long file_len = ftell(vgm_file);
        if (file_len >= 4) {
            uint32_t eof_offset = (uint32_t)(file_len - 4);
            fseek(vgm_file, 4, SEEK_SET);
            fwrite(&eof_offset, sizeof(uint32_t), 1, vgm_file);
        }

        fclose(vgm_file);
        vgm_file = NULL;
    }
}

static void vgm_write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
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
