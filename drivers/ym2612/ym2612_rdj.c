/* GPLv3 - RetroDJ Project - YM2612 Driver */
/* Derived from Blastem (GPLv3) */
/* Derived from Genesis-Plus-GX (GPLv3) */

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
#include <stddef.h>
#include "../../core/rdj_driver.h"

#define YM2612_NUM_CHANNELS 6
#define YM2612_NUM_PATTERNS 32

/* Required structs from task specification */
typedef struct {
    uint8_t DT_MUL;   /* Detune / Multiple */
    uint8_t TL;       /* Total Level */
    uint8_t AR;       /* Attack Rate */
    uint8_t DR;       /* Decay Rate */
    uint8_t SR;       /* Sustain Rate */
    uint8_t SL_RR;    /* Sustain Level / Release Rate */
    uint8_t SSG_EG;   /* SSG-EG */
} ym2612_operator_t;

typedef struct {
    ym2612_operator_t ops[4];
    uint8_t FB_ALG;   /* Feedback / Algorithm */
    uint8_t LR_AMS_PMS;
} ym2612_patch_t;

/* Forward Declarations */
void ym2612_rdj_init(uint32_t clock);
void ym2612_rdj_reset(void);
void ym2612_rdj_write(uint8_t port, uint8_t reg, uint8_t val);
void ym2612_rdj_note_on(uint8_t channel, uint8_t note, uint8_t instrument);
void ym2612_rdj_note_off(uint8_t channel);
void ym2612_rdj_set_patch(uint8_t inst_slot, ym2612_patch_t *patch);

#ifdef RDJ_TARGET_REAL_HARDWARE
    /* For actual Genesis hardware - user implements */
    extern void hardware_ym2612_write(uint8_t port,
                                      uint8_t reg,
                                      uint8_t val);
    void ym2612_rdj_write(uint8_t port, uint8_t reg, uint8_t val) {
        hardware_ym2612_write(port, reg, val);
    }
#else
    /* For emulator/PC - logs register writes */
    #include <stdio.h>
    static uint8_t reg_shadow[2][0x100]; /* shadow register state */
    void ym2612_rdj_write(uint8_t port, uint8_t reg, uint8_t val) {
        reg_shadow[port & 1][reg] = val;
        printf("[YM2612] P%d R%02X = %02X\n", port, reg, val);
    }
#endif

/* Register definitions */
#define REG_KEY_ONOFF    0x28  /* Key On/Off register */
#define REG_DETUNE_MULT  0x30  /* Detune / Multiple */
#define REG_TOTAL_LEVEL  0x40  /* Total Level */
#define REG_ATTACK_KS    0x50  /* Key Scale / Attack Rate */
#define REG_DECAY_AM     0x60  /* AM / Decay Rate */
#define REG_SUSTAIN_RATE 0x70  /* Sustain Rate */
#define REG_SUSTAIN_REL  0x80  /* Sustain Level / Release Rate */
#define REG_SSG_EG       0x90  /* SSG-EG */
#define REG_FREQ_LOW     0xA0  /* Frequency Low 8 bits */
#define REG_FREQ_HI      0xA4  /* Frequency High 3 bits + Block 3 bits */
#define REG_ALG_FEEDBACK 0xB0  /* Algorithm / Feedback */
#define REG_LR_AMS_PMS   0xB4  /* L/R Stereo, AMS, PMS */

/* Operator register mapping order for YM2612 hardware channel slots:
 * Op 1: offset 0x00
 * Op 2: offset 0x08
 * Op 3: offset 0x04
 * Op 4: offset 0x0C
 */
static const uint8_t op_offsets[4] = { 0x00, 0x08, 0x04, 0x0C };

/* FNUM table for 12 notes per octave (C to B).
 * Values standard for OPN2 / YM2612 phase calculations.
 */
static const uint16_t ym_fnum_table[12] = {
    617, 653, 692, 733, 777, 823, 872, 924, 979, 1037, 1099, 1164
};

/* Internal patch bank stored without dynamic allocation */
static ym2612_patch_t patch_bank[32];
static uint32_t current_clock = 7670454; /* Default Sega Genesis YM2612 NTSC clock ~7.67 MHz */

/* Port offset helper: Part 1 = port 0, Part 2 = port 1 */
static inline uint8_t get_port(uint8_t channel) {
    return (channel >= 3) ? 1 : 0;
}

/* Channel offset helper within part: 0..2 */
static inline uint8_t get_channel_offset(uint8_t channel) {
    return (channel >= 3) ? (channel - 3) : channel;
}

void ym2612_rdj_init(uint32_t clock) {
    current_clock = clock ? clock : 7670454;
    ym2612_rdj_reset();
}

void ym2612_rdj_reset(void) {
    /* Disable DAC / enable FM channels */
    ym2612_rdj_write(0, 0x2B, 0x00); /* DAC disable (Port 0 Reg 0x2B bit 7 = 0) */

    /* Turn off key for all 6 channels */
    for (uint8_t ch = 0; ch < 6; ch++) {
        uint8_t key_ch = (ch >= 3) ? (ch + 1) : ch;
        /* REG_KEY_ONOFF (0x28): bits[6:4] = 0 (all ops off), bits[2:0] = channel */
        ym2612_rdj_write(0, REG_KEY_ONOFF, key_ch & 0x07);
    }

    /* Clear patch memory */
    for (int i = 0; i < 32; i++) {
        for (int op = 0; op < 4; op++) {
            patch_bank[i].ops[op].DT_MUL = 0;
            patch_bank[i].ops[op].TL = 0x7F; /* Silent level */
            patch_bank[i].ops[op].AR = 0;
            patch_bank[i].ops[op].DR = 0;
            patch_bank[i].ops[op].SR = 0;
            patch_bank[i].ops[op].SL_RR = 0xFF;
            patch_bank[i].ops[op].SSG_EG = 0;
        }
        patch_bank[i].FB_ALG = 0;
        patch_bank[i].LR_AMS_PMS = 0xC0; /* Default L/R stereo enabled */
    }
}

void ym2612_rdj_set_patch(uint8_t inst_slot, ym2612_patch_t *patch) {
    if (inst_slot < 32 && patch != NULL) {
        patch_bank[inst_slot] = *patch;
    }
}

void ym2612_rdj_note_on(uint8_t channel, uint8_t note, uint8_t instrument) {
    if (channel >= YM2612_NUM_CHANNELS) return;

    /* Apply patch parameters if valid instrument slot */
    if (instrument < 32) {
        ym2612_patch_t *p = &patch_bank[instrument];
        uint8_t port = get_port(channel);
        uint8_t ch_off = get_channel_offset(channel);

        /* Program algorithm and feedback (0xB0) */
        /* Bits[5:3] = Feedback, Bits[2:0] = Algorithm */
        ym2612_rdj_write(port, REG_ALG_FEEDBACK + ch_off, p->FB_ALG);

        /* Program LR panning and LFO AMS/PMS (0xB4) */
        /* Bits[7:6] = L/R Pan, Bits[5:4] = AMS, Bits[2:0] = PMS */
        ym2612_rdj_write(port, REG_LR_AMS_PMS + ch_off, p->LR_AMS_PMS);

        /* Program 4 operators per channel */
        for (uint8_t op = 0; op < 4; op++) {
            uint8_t op_reg = op_offsets[op] + ch_off;
            ym2612_operator_t *o = &p->ops[op];

            /* REG_DETUNE_MULT (0x30): Bits[6:4] = DT, Bits[3:0] = MUL */
            ym2612_rdj_write(port, REG_DETUNE_MULT + op_reg, o->DT_MUL);
            /* REG_TOTAL_LEVEL (0x40): Bits[6:0] = Total Level (volume attenuation) */
            ym2612_rdj_write(port, REG_TOTAL_LEVEL + op_reg, o->TL);
            /* REG_ATTACK_KS (0x50): Bits[7:6] = KS, Bits[4:0] = AR */
            ym2612_rdj_write(port, REG_ATTACK_KS + op_reg, o->AR);
            /* REG_DECAY_AM (0x60): Bit[7] = AM, Bits[4:0] = DR */
            ym2612_rdj_write(port, REG_DECAY_AM + op_reg, o->DR);
            /* REG_SUSTAIN_RATE (0x70): Bits[4:0] = SR */
            ym2612_rdj_write(port, REG_SUSTAIN_RATE + op_reg, o->SR);
            /* REG_SUSTAIN_REL (0x80): Bits[7:4] = SL, Bits[3:0] = RR */
            ym2612_rdj_write(port, REG_SUSTAIN_REL + op_reg, o->SL_RR);
            /* REG_SSG_EG (0x90): Bits[3:0] = SSG-EG control */
            ym2612_rdj_write(port, REG_SSG_EG + op_reg, o->SSG_EG);
        }
    }

    /* Calculate Octave Block and FNUM from MIDI note */
    /* MIDI note 0 = C-1 (octave 0, note 0) */
    uint8_t note_idx = note % 12;
    uint8_t block = note / 12;
    if (block > 7) block = 7; /* YM2612 supports blocks 0-7 */

    uint16_t fnum = ym_fnum_table[note_idx];
    uint8_t port = get_port(channel);
    uint8_t ch_off = get_channel_offset(channel);

    /* Write Frequency Low Byte (0xA0 + channel) */
    /* Bits[7:0] = FNUM[7:0] */
    ym2612_rdj_write(port, REG_FREQ_LOW + ch_off, fnum & 0xFF);

    /* Write Frequency High Byte + Block (0xA4 + channel) */
    /* Bits[5:3] = Block (Octave), Bits[2:0] = FNUM[10:8] */
    uint8_t freq_hi = ((block & 0x07) << 3) | ((fnum >> 8) & 0x07);
    ym2612_rdj_write(port, REG_FREQ_HI + ch_off, freq_hi);

    /* Key ON: REG_KEY_ONOFF = 0x28 */
    /* Bits[7:4] = 0xF0 (Enable all 4 operators), Bits[2:0] = channel hardware index */
    uint8_t key_ch = (channel >= 3) ? (channel + 1) : channel;
    ym2612_rdj_write(0, REG_KEY_ONOFF, 0xF0 | (key_ch & 0x07));
}

void ym2612_rdj_note_off(uint8_t channel) {
    if (channel >= YM2612_NUM_CHANNELS) return;

    /* Key OFF: REG_KEY_ONOFF = 0x28 */
    /* Bits[7:4] = 0x00 (Disable all 4 operators), Bits[2:0] = channel hardware index */
    uint8_t key_ch = (channel >= 3) ? (channel + 1) : channel;
    ym2612_rdj_write(0, REG_KEY_ONOFF, 0x00 | (key_ch & 0x07));
}

/* Driver interface wrapper functions matching rdj_driver_t function pointers */
static void ym2612_wrapper_note_on(uint8_t ch, uint8_t note, uint8_t inst, uint8_t vol) {
    (void)vol;
    ym2612_rdj_note_on(ch, note, inst);
}

static void ym2612_wrapper_write_reg(uint16_t reg, uint8_t val) {
    /* High byte selects port (0 or 1), Low byte is register */
    uint8_t port = (reg >> 8) & 0x01;
    uint8_t r = reg & 0xFF;
    ym2612_rdj_write(port, r, val);
}

static void ym2612_wrapper_shutdown(void) {
    ym2612_rdj_reset();
}

/* Global unified driver structure instance */
rdj_driver_t ym2612_driver = {
    .name = "YM2612 OPN2 FM Driver",
    .chip = "YM2612",
    .num_channels = YM2612_NUM_CHANNELS,
    .num_instruments = YM2612_NUM_PATTERNS,
    .init = ym2612_rdj_init,
    .reset = ym2612_rdj_reset,
    .note_on = ym2612_wrapper_note_on,
    .note_off = ym2612_rdj_note_off,
    .write_reg = ym2612_wrapper_write_reg,
    .shutdown = ym2612_wrapper_shutdown
};
