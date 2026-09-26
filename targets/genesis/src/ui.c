/* RetroDJ - Sega Genesis VDP Text-Mode Tracker UI */
/* GPLv3 - RetroDJ Project */

#include "ui.h"

#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE) && !defined(__m68k__)
#include <stdio.h>
#endif

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
#include <genesis.h>
#endif

#include "core/rdj_driver.h"

/* YM2612 patch structure definition */
typedef struct {
    uint8_t DT_MUL, TL, AR, DR, SR, SL_RR, SSG_EG;
} ym2612_op_test_t;

typedef struct {
    ym2612_op_test_t ops[4];
    uint8_t FB_ALG;
    uint8_t LR_AMS_PMS;
} ym2612_patch_test_t;

extern void ym2612_rdj_set_patch(uint8_t inst_slot, void *patch);
extern void ym2612_rdj_note_on(uint8_t channel, uint8_t note, uint8_t instrument);
extern void ym2612_rdj_note_off(uint8_t channel);

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
static uint8_t current_row = 0;
static uint8_t current_col = 0;
static uint16_t last_joy = 0;
#endif

static uint8_t channel_notes[6] = {
    0,0,0,0,0,0
};

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
static void get_note_name(uint8_t note, char *out) {
    if (note == 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '-'; out[3] = '\0';
        return;
    }
    static const char *note_names[12] = {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
    };
    uint8_t note_idx = note % 12;
    int octave = (note / 12) - 1;
    if (octave < 0) octave = 0;
    if (octave > 9) octave = 9;
    sprintf(out, "%s%d", note_names[note_idx], octave);
}
#endif

void ui_init(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    VDP_drawText("--- RetroDJ Tracker ---", 8, 1);
    VDP_drawText("Press A, B, C to Play FM Sound!", 4, 3);
    VDP_drawText("CH1  CH2  CH3  CH4  CH5  CH6", 7, 5);

    /* Prepare FM Synth Patch on slot 0 */
    ym2612_patch_test_t bass_patch = {
        .ops = {
            /* Detune/Mul, TotalLevel, Attack, Decay, Sustain, SusLvl/Rel, SSG */
            { 0x31, 0x00, 0x1F, 0x05, 0x02, 0x14, 0x00 }, /* Op 1 (Carrier) */
            { 0x01, 0x24, 0x1F, 0x08, 0x05, 0x24, 0x00 }, /* Op 2 */
            { 0x01, 0x22, 0x1F, 0x06, 0x04, 0x24, 0x00 }, /* Op 3 */
            { 0x01, 0x00, 0x1F, 0x04, 0x02, 0x14, 0x00 }  /* Op 4 (Carrier) */
        },
        .FB_ALG = 0x04,      /* Algorithm 4 (2 Modulators, 2 Carriers) */
        .LR_AMS_PMS = 0xC0   /* Stereo Center (L+R on) */
    };
    ym2612_rdj_set_patch(0, &bass_patch);
#endif
}

void ui_update(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    uint16_t joy = JOY_readJoypad(JOY_1);

    /* Move Cursor and Rows with D-Pad */
    if ((joy & BUTTON_UP) && !(last_joy & BUTTON_UP)) {
        if (current_row > 0) current_row--;
    }
    if ((joy & BUTTON_DOWN) && !(last_joy & BUTTON_DOWN)) {
        if (current_row < 63) current_row++;
    }
    if ((joy & BUTTON_LEFT) && !(last_joy & BUTTON_LEFT)) {
        if (current_col > 0) current_col--;
    }
    if ((joy & BUTTON_RIGHT) && !(last_joy & BUTTON_RIGHT)) {
        if (current_col < 5) current_col++;
    }

    /* Live FM sound notes:
       Button A -> Note C-3 (MIDI 48)
       Button B -> Note E-3 (MIDI 52)
       Button C -> Note G-3 (MIDI 55)
    */
    if ((joy & BUTTON_A) && !(last_joy & BUTTON_A)) {
        ym2612_rdj_note_on(current_col, 48, 0);
        channel_notes[current_col] = 48;
    } else if (!(joy & BUTTON_A) && (last_joy & BUTTON_A)) {
        ym2612_rdj_note_off(current_col);
        channel_notes[current_col] = 0;
    }

    if ((joy & BUTTON_B) && !(last_joy & BUTTON_B)) {
        ym2612_rdj_note_on(current_col, 52, 0);
        channel_notes[current_col] = 52;
    } else if (!(joy & BUTTON_B) && (last_joy & BUTTON_B)) {
        ym2612_rdj_note_off(current_col);
        channel_notes[current_col] = 0;
    }

    if ((joy & BUTTON_C) && !(last_joy & BUTTON_C)) {
        ym2612_rdj_note_on(current_col, 55, 0);
        channel_notes[current_col] = 55;
    } else if (!(joy & BUTTON_C) && (last_joy & BUTTON_C)) {
        ym2612_rdj_note_off(current_col);
        channel_notes[current_col] = 0;
    }

    last_joy = joy;
#endif
}

void ui_render(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    char buf[64];
    char note_str[6][4];

    for (uint8_t c = 0; c < 6; c++) {
        get_note_name(channel_notes[c], note_str[c]);
    }

    for (uint8_t r = 0; r < 14; r++) {
        uint8_t display_row = (current_row + r) % 64;
        sprintf(buf, "%02X |%c%s %c%s %c%s %c%s %c%s %c%s",
                display_row,
                (current_col == 0) ? '>' : ' ', note_str[0],
                (current_col == 1) ? '>' : ' ', note_str[1],
                (current_col == 2) ? '>' : ' ', note_str[2],
                (current_col == 3) ? '>' : ' ', note_str[3],
                (current_col == 4) ? '>' : ' ', note_str[4],
                (current_col == 5) ? '>' : ' ', note_str[5]);
        VDP_drawText(buf, 2, 7 + r);
    }

    char status_buf[32];
    sprintf(status_buf, "COL:%d ROW:%02X  A=C3 B=E3 C=G3", current_col, current_row);
    VDP_drawText(status_buf, 2, 22);
#endif
}
