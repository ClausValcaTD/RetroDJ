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

/* تعريف بنية الـ Patch للـ YM2612 */
typedef struct {
    uint8_t DT_MUL, TL, AR, DR, SR, SL_RR, SSG_EG;
} ym2612_op_test_t;

typedef struct {
    ym2612_op_test_t ops[4];
    uint8_t FB_ALG;
    uint8_t LR_AMS_PMS;
} ym2612_patch_test_t;

extern void ym2612_rdj_set_patch(uint8_t inst_slot, void *patch);

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
static uint8_t current_row = 0;
static uint8_t current_col = 0;
static uint16_t last_joy = 0;
#endif

void ui_init(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    VDP_drawText("--- RetroDJ Tracker ---", 8, 1);
    VDP_drawText("Press A, B, C to Play FM Sound!", 4, 3);
    VDP_drawText("CH1  CH2  CH3  CH4  CH5  CH6", 4, 5);

    /* تجهيز Patch لصوت FM Synth بيز كلاسيكي على السلوت 0 */
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

    /* تحريك الـ Cursor والـ Rows بالأسهم */
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

    /* عزف نوتات الـ FM الحية:
       زرار A -> نوتة C-3 (MIDI 48)
       زرار B -> نوتة E-3 (MIDI 52)
       زرار C -> نوتة G-3 (MIDI 55)
    */
    if ((joy & BUTTON_A) && !(last_joy & BUTTON_A)) {
        ym2612_driver.note_on(current_col, 48, 0, 127);
    } else if (!(joy & BUTTON_A) && (last_joy & BUTTON_A)) {
        ym2612_driver.note_off(current_col);
    }

    if ((joy & BUTTON_B) && !(last_joy & BUTTON_B)) {
        ym2612_driver.note_on(current_col, 52, 0, 127);
    } else if (!(joy & BUTTON_B) && (last_joy & BUTTON_B)) {
        ym2612_driver.note_off(current_col);
    }

    if ((joy & BUTTON_C) && !(last_joy & BUTTON_C)) {
        ym2612_driver.note_on(current_col, 55, 0, 127);
    } else if (!(joy & BUTTON_C) && (last_joy & BUTTON_C)) {
        ym2612_driver.note_off(current_col);
    }

    last_joy = joy;
#endif
}

void ui_render(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    char buf[32];
    for (uint8_t r = 0; r < 14; r++) {
        uint8_t display_row = (current_row + r) % 64;
        sprintf(buf, "%02X | C-3  ---  E-3  ---  G-3  ---", display_row);
        VDP_drawText(buf, 2, 7 + r);
    }
#endif
}
