/* RetroDJ - Sega Genesis VDP Text-Mode Tracker UI */
/* GPLv3 - RetroDJ Project */

#include "ui.h"
#include "input_mapper.h"

#if !defined(SGDK_GCC) && !defined(RDJ_GENESIS_HARDWARE) && !defined(__m68k__)
#include <stdio.h>
#include <string.h>
#endif

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
#include <genesis.h>
#endif

#include "core/rdj_driver.h"
#include "core/sequencer.h"

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

static rdj_song_t g_song;
static input_tracker_state_t g_tracker_state;

static uint8_t s_tick_counter = 0;
static uint8_t s_current_play_row = 0;
static uint8_t s_prev_is_playing = 0;

void get_note_name(uint8_t note, char *out) {
    if (note == RDJ_NOTE_EMPTY || note == 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '-'; out[3] = '\0';
        return;
    }
    if (note == RDJ_NOTE_OFF) {
        out[0] = 'O'; out[1] = 'F'; out[2] = 'F'; out[3] = '\0';
        return;
    }
    static const char *note_names[12] = {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
    };
    uint8_t note_idx = (note - 12) % 12;
    int octave = (note - 12) / 12;
    if (octave < 0) octave = 0;
    if (octave > 9) octave = 9;
    sprintf(out, "%s%d", note_names[note_idx], octave);
}

void ui_init(void) {
    /* Initialize song and default pattern data */
    g_song.bpm = 120;
    g_song.speed = 6;
    g_song.order_length = 1;
    g_song.order[0] = 0;

    for (int r = 0; r < RDJ_MAX_ROWS; r++) {
        for (int c = 0; c < RDJ_MAX_CHANNELS; c++) {
            g_song.patterns[0].rows[r][c].note = RDJ_NOTE_EMPTY;
            g_song.patterns[0].rows[r][c].instrument = 0;
            g_song.patterns[0].rows[r][c].volume = 0;
            g_song.patterns[0].rows[r][c].fx_cmd = 0;
            g_song.patterns[0].rows[r][c].fx_val = 0;
        }
    }
    g_song.patterns[0].length = 64;
    g_song.patterns[0].speed = 6;

    input_mapper_init(&g_tracker_state);

    s_tick_counter = 0;
    s_current_play_row = 0;
    s_prev_is_playing = 0;

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    VDP_clearText(0, 0, 40);

    /* Prepare FM Synth Patch on slot 0 */
    ym2612_patch_test_t bass_patch = {
        .ops = {
            { 0x31, 0x00, 0x1F, 0x05, 0x02, 0x14, 0x00 }, /* Op 1 */
            { 0x01, 0x24, 0x1F, 0x08, 0x05, 0x24, 0x00 }, /* Op 2 */
            { 0x01, 0x22, 0x1F, 0x06, 0x04, 0x24, 0x00 }, /* Op 3 */
            { 0x01, 0x00, 0x1F, 0x04, 0x02, 0x14, 0x00 }  /* Op 4 */
        },
        .FB_ALG = 0x04,
        .LR_AMS_PMS = 0xC0
    };
    ym2612_rdj_set_patch(0, &bass_patch);
#endif
}

static void ui_trigger_row_notes(uint8_t row) {
    for (uint8_t ch = 0; ch < 4; ch++) {
        rdj_row_t *r = &g_song.patterns[0].rows[row][ch];
        if (r->note != RDJ_NOTE_EMPTY && r->note != RDJ_NOTE_OFF) {
            ym2612_rdj_note_on(ch, r->note, r->instrument);
        } else if (r->note == RDJ_NOTE_OFF) {
            ym2612_rdj_note_off(ch);
        }
        /* CRITICAL: If r->note == RDJ_NOTE_EMPTY (0x00 / '---'), DO NOT call note_off!
         * Let the previous note sustain naturally.
         */
    }
}

void ui_process_playback(void) {
    if (g_tracker_state.is_playing) {
        if (!s_prev_is_playing) {
            s_prev_is_playing = 1;
            s_tick_counter = 0;
            s_current_play_row = g_tracker_state.cursor_row;
            ui_trigger_row_notes(s_current_play_row);
        } else {
            s_tick_counter++;
            uint8_t speed = g_song.patterns[0].speed ? g_song.patterns[0].speed : (g_song.speed ? g_song.speed : 6);
            if (s_tick_counter >= speed) {
                s_tick_counter = 0;
                uint8_t pat_len = g_song.patterns[0].length ? g_song.patterns[0].length : RDJ_MAX_ROWS;
                s_current_play_row++;
                if (s_current_play_row >= pat_len) {
                    s_current_play_row = 0;
                }
                g_tracker_state.cursor_row = s_current_play_row;
                ui_trigger_row_notes(s_current_play_row);
            }
        }
    } else {
        if (s_prev_is_playing) {
            s_prev_is_playing = 0;
            s_tick_counter = 0;
            for (uint8_t ch = 0; ch < 4; ch++) {
                ym2612_rdj_note_off(ch);
            }
        }
    }
}

void ui_update(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    uint16_t joy = JOY_readJoypad(JOY_1);
    input_mapper_update(&g_tracker_state, &g_song, joy);
#endif
    ui_process_playback();
}

void ui_render(void) {
#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    char buf[64];

    /* 1. Top Header Status Bar */
    sprintf(buf, "BPM:%03d [%s] OCT:%d CH:%d",
            g_song.bpm,
            g_tracker_state.is_playing ? "PLAY" : "STOP",
            g_tracker_state.octave,
            g_tracker_state.cursor_channel + 1);
    VDP_drawText("--- RetroDJ Tracker ---", 8, 0);
    VDP_drawText(buf, 2, 2);

    /* Channel Headers */
    VDP_drawText("ROW| CH1   CH2   CH3   CH4  ", 2, 4);
    VDP_drawText("---|------------------------", 2, 5);

    /* 2. Multi-column Pattern Grid (Render 14 rows) */
    uint8_t start_row = g_tracker_state.cursor_row >= 7 ? g_tracker_state.cursor_row - 7 : 0;
    if (start_row > RDJ_MAX_ROWS - 14) start_row = RDJ_MAX_ROWS - 14;

    for (uint8_t i = 0; i < 14; i++) {
        uint8_t r = start_row + i;
        bool is_cursor_row = (r == g_tracker_state.cursor_row);

        char row_str[40];
        sprintf(row_str, "%02X%c|", r, is_cursor_row ? '>' : ' ');

        for (uint8_t c = 0; c < 4; c++) { /* Display 4 active channels */
            rdj_row_t *cell = &g_song.patterns[0].rows[r][c];
            char note_name[4];
            get_note_name(cell->note, note_name);

            bool is_curr_cell = is_cursor_row && (g_tracker_state.cursor_channel == c);

            char sub_note = (is_curr_cell && g_tracker_state.cursor_subcol == RDJ_SUBCOL_NOTE) ? '*' : ' ';
            char sub_inst = (is_curr_cell && g_tracker_state.cursor_subcol == RDJ_SUBCOL_INST) ? '*' : ' ';
            char sub_vol  = (is_curr_cell && g_tracker_state.cursor_subcol == RDJ_SUBCOL_VOL)  ? '*' : ' ';

            char chan_buf[12];
            sprintf(chan_buf, "%s%c%02X%c%02X%c|",
                    note_name, sub_note,
                    cell->instrument, sub_inst,
                    cell->volume, sub_vol);
            strcat(row_str, chan_buf);
        }

        VDP_drawText(row_str, 2, 6 + i);
    }

    /* 3. Bottom Contextual Help Bar */
    VDP_drawText("----------------------------------------", 0, 21);
    VDP_drawText("A+DIR:Note B+DIR:Val B+A:Clr START:Play", 0, 22);
#endif
}

input_tracker_state_t *ui_get_tracker_state(void) {
    return &g_tracker_state;
}

rdj_song_t *ui_get_song(void) {
    return &g_song;
}
