/* GPLv3 - RetroDJ Project - NES 2A03 APU Driver */
/* Derived from Nestopia (GPLv3) */
/* Derived from Mesen (GPLv3) */

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

#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

/* Register map (from Nestopia) */
#define APU_SQ1_VOL    0x4000  /* duty, loop, vol */
#define APU_SQ1_SWEEP  0x4001  /* sweep unit setup */
#define APU_SQ1_LO     0x4002  /* freq low byte */
#define APU_SQ1_HI     0x4003  /* freq high + length counter */
#define APU_SQ2_VOL    0x4004  /* duty, loop, vol */
#define APU_SQ2_SWEEP  0x4005  /* sweep unit setup */
#define APU_SQ2_LO     0x4006  /* freq low byte */
#define APU_SQ2_HI     0x4007  /* freq high + length counter */
#define APU_TRI_LINEAR 0x4008  /* linear counter, control */
#define APU_TRI_LO     0x400A  /* freq low byte */
#define APU_TRI_HI     0x400B  /* freq high + length counter */
#define APU_NOI_VOL    0x400C  /* loop, vol */
#define APU_NOI_LO     0x400E  /* loop/mode, period */
#define APU_NOI_HI     0x400F  /* length counter */
#define APU_STATUS     0x4015  /* channel enable status */

/* Forward Declarations */
void apu_rdj_init(uint32_t cpu_clock);
void apu_rdj_reset(void);
void apu_rdj_write(uint16_t addr, uint8_t val);
void apu_rdj_enable_channels(uint8_t mask);
void apu_rdj_square_note_on(uint8_t ch, uint8_t note, uint8_t duty, uint8_t vol);
void apu_rdj_square_note_off(uint8_t ch);
void apu_rdj_triangle_note_on(uint8_t note);
void apu_rdj_triangle_note_off(void);
void apu_rdj_noise_on(uint8_t period, uint8_t vol);
void apu_rdj_noise_off(void);

void apu_rdj_write(uint16_t addr, uint8_t val) {
    if (rdj_active_backend && rdj_active_backend->write_apu) {
        rdj_active_backend->write_apu(addr, val);
    }
}

/* Default NTSC 2A03 CPU Clock = 1789773 Hz */
static uint32_t apu_clock = 1789773;
static uint8_t channel_status_mask = 0x0F;

/* Precomputed uint16_t wavelength lookup table for all 128 MIDI notes
 * Calculated for NTSC CPU clock 1789773 Hz:
 * wavelength = (cpu_clock / (16 * note_freq_hz)) - 1
 * Clamped to 11-bit timer limit (0..2047)
 */
static const uint16_t nes_wavelength_table[128] = {
    2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047,
    2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047,
    2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047,
    2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047,
    2047, 2033, 1919, 1811, 1709, 1613, 1523, 1437,
    1356, 1280, 1208, 1140, 1076, 1016,  959,  905,
     854,  806,  761,  718,  678,  640,  604,  570,
     538,  507,  479,  452,  427,  403,  380,  359,
     338,  319,  301,  284,  268,  253,  239,  225,
     213,  201,  189,  179,  169,  159,  150,  142,
      134,  126,  119,  112,  106,  100,   94,   89,
      84,   79,   75,   70,   66,   63,   59,   56,
      52,   49,   47,   44,   41,   39,   37,   35,
      33,   31,   29,   27,   26,   24,   23,   21,
      20,   19,   18,   17,   16,   15,   14,   13,
      12,   12,   11,   10,   10,    9,    8,    8
};

void apu_rdj_init(uint32_t cpu_clock) {
    apu_clock = cpu_clock ? cpu_clock : 1789773;
    apu_rdj_reset();
}

void apu_rdj_reset(void) {
    channel_status_mask = 0x0F;

    /* Disable sweep for Square 1 & Square 2 */
    apu_rdj_write(APU_SQ1_SWEEP, 0x08); /* Bit 3 = negate disabled, enable = 0 */
    apu_rdj_write(APU_SQ2_SWEEP, 0x08);

    /* Silence all channels */
    apu_rdj_write(APU_SQ1_VOL, 0x30); /* Duty 0, constant volume, volume 0 */
    apu_rdj_write(APU_SQ2_VOL, 0x30);
    apu_rdj_write(APU_TRI_LINEAR, 0x80); /* Linear counter disabled/halted */
    apu_rdj_write(APU_NOI_VOL, 0x30); /* Volume 0 */

    /* APU_STATUS: enable Square 1, Square 2, Triangle, Noise (0x0F) */
    apu_rdj_write(APU_STATUS, 0x0F);
}

void apu_rdj_enable_channels(uint8_t mask) {
    channel_status_mask = mask & 0x1F;
    /* Write to APU_STATUS (0x4015) to enable/disable specific APU channels */
    apu_rdj_write(APU_STATUS, channel_status_mask);
}

static uint16_t calc_wavelength(uint8_t note) {
    if (note > 127) note = 127;
    return nes_wavelength_table[note];
}

void apu_rdj_square_note_on(uint8_t ch, uint8_t note, uint8_t duty, uint8_t vol) {
    uint16_t base_reg;
    uint16_t timer;
    uint8_t duty_bits;
    uint8_t vol_bits;

    if (ch > 1) return;
    base_reg = (ch == 0) ? APU_SQ1_VOL : APU_SQ2_VOL;

    timer = calc_wavelength(note);
    duty_bits = (duty & 0x03) << 6;
    vol_bits = vol & 0x0F;

    /* Write APU_SQx_VOL: duty cycle, disable length counter halt (bit 5 = 1), constant volume (bit 4 = 1), volume */
    apu_rdj_write(base_reg + 0, duty_bits | 0x30 | vol_bits);

    /* Write APU_SQx_SWEEP: disable sweep */
    apu_rdj_write(base_reg + 1, 0x08);

    /* Write APU_SQx_LO: frequency low 8 bits */
    apu_rdj_write(base_reg + 2, timer & 0xFF);

    /* Write APU_SQx_HI: frequency high 3 bits + length counter reload (0xF8 bit mask) */
    apu_rdj_write(base_reg + 3, ((timer >> 8) & 0x07) | 0xF8);
}

void apu_rdj_square_note_off(uint8_t ch) {
    uint16_t base_reg;
    if (ch > 1) return;
    base_reg = (ch == 0) ? APU_SQ1_VOL : APU_SQ2_VOL;

    /* Silence square channel by setting volume to 0 */
    apu_rdj_write(base_reg, 0x30);
}

void apu_rdj_triangle_note_on(uint8_t note) {
    uint16_t timer = calc_wavelength(note);

    /* Write APU_TRI_LINEAR (0x4008): control bit / linear counter reload value = 0x7F */
    apu_rdj_write(APU_TRI_LINEAR, 0xFF);

    /* Write APU_TRI_LO (0x400A): frequency low 8 bits */
    apu_rdj_write(APU_TRI_LO, timer & 0xFF);

    /* Write APU_TRI_HI (0x400B): frequency high 3 bits + length counter reload */
    apu_rdj_write(APU_TRI_HI, ((timer >> 8) & 0x07) | 0xF8);
}

void apu_rdj_triangle_note_off(void) {
    /* Halt triangle channel linear counter */
    apu_rdj_write(APU_TRI_LINEAR, 0x80);
}

void apu_rdj_noise_on(uint8_t period, uint8_t vol) {
    uint8_t vol_bits = vol & 0x0F;

    /* Write APU_NOI_VOL (0x400C): constant volume (bit 4 = 1), volume bits */
    apu_rdj_write(APU_NOI_VOL, 0x30 | vol_bits);

    /* Write APU_NOI_LO (0x400E): noise period index (bits 3:0) */
    apu_rdj_write(APU_NOI_LO, period & 0x0F);

    /* Write APU_NOI_HI (0x400F): length counter reload */
    apu_rdj_write(APU_NOI_HI, 0xF8);
}

void apu_rdj_noise_off(void) {
    /* Silence noise channel */
    apu_rdj_write(APU_NOI_VOL, 0x30);
}

/* Driver interface wrapper functions matching rdj_driver_t function pointers */
static void apu_wrapper_note_on(uint8_t ch, uint8_t note, uint8_t inst, uint8_t vol) {
    (void)inst;
    if (ch == 0 || ch == 1) {
        apu_rdj_square_note_on(ch, note, 2 /* 50% duty */, vol >> 3 /* 0-15 volume range */);
    } else if (ch == 2) {
        apu_rdj_triangle_note_on(note);
    } else if (ch == 3) {
        apu_rdj_noise_on(note & 0x0F, vol >> 3);
    }
}

static void apu_wrapper_note_off(uint8_t ch) {
    if (ch == 0 || ch == 1) {
        apu_rdj_square_note_off(ch);
    } else if (ch == 2) {
        apu_rdj_triangle_note_off();
    } else if (ch == 3) {
        apu_rdj_noise_off();
    }
}

static void apu_wrapper_write_reg(uint16_t reg, uint8_t val) {
    apu_rdj_write(reg, val);
}

static void apu_wrapper_shutdown(void) {
    apu_rdj_reset();
}

static const char nes_driver_name[] = "NES 2A03 APU Driver";
static const char nes_chip_name[] = "2A03";

/* Global unified driver structure instance */
rdj_driver_t nes_2a03_driver = {
    nes_driver_name,
    nes_chip_name,
    5,
    16,
    apu_rdj_init,
    apu_rdj_reset,
    apu_wrapper_note_on,
    apu_wrapper_note_off,
    apu_wrapper_write_reg,
    apu_wrapper_shutdown
};
