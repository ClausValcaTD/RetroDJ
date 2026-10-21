/* RetroDJ - NES Tracker UI & Controller Implementation */
/* GPLv3 - RetroDJ Project */

#include "ui_nes.h"
#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

#include <stdio.h>
#include <string.h>

#define PPU_CTRL  (*((volatile uint8_t*)0x2000))
#define PPU_MASK  (*((volatile uint8_t*)0x2001))
#define PPU_STATUS (*((volatile uint8_t*)0x2002))
#define PPU_SCROLL (*((volatile uint8_t*)0x2005))
#define PPU_ADDR  (*((volatile uint8_t*)0x2006))
#define PPU_DATA  (*((volatile uint8_t*)0x2007))
#define JOYPAD1   (*((volatile uint8_t*)0x4016))

static rdj_song_t g_song;
static input_tracker_state_t g_tracker_state;

static uint8_t s_tick_counter = 0;
static uint8_t s_current_play_row = 0;
static uint8_t s_prev_is_playing = 0;

extern volatile uint8_t vblank_flag;

void nes_vblank_wait(void) {
#if defined(__NES__) || defined(__CC65__)
    vblank_flag = 0;
    while (vblank_flag == 0) {
    }
#endif
}

uint16_t nes_read_controller(void) {
    uint16_t buttons = 0;
#if defined(__NES__) || defined(__CC65__)
    /* Strobe controller 1 */
    JOYPAD1 = 0x01;
    JOYPAD1 = 0x00;

    /* NES Controller order: A, B, SELECT, START, UP, DOWN, LEFT, RIGHT */
    if (JOYPAD1 & 1) buttons |= BUTTON_A;     /* Bit 0: A */
    if (JOYPAD1 & 1) buttons |= BUTTON_B;     /* Bit 1: B */
    if (JOYPAD1 & 1) { /* SELECT bit - unmapped */ }
    if (JOYPAD1 & 1) buttons |= BUTTON_START; /* Bit 3: START */
    if (JOYPAD1 & 1) buttons |= BUTTON_UP;    /* Bit 4: UP */
    if (JOYPAD1 & 1) buttons |= BUTTON_DOWN;  /* Bit 5: DOWN */
    if (JOYPAD1 & 1) buttons |= BUTTON_LEFT;  /* Bit 6: LEFT */
    if (JOYPAD1 & 1) buttons |= BUTTON_RIGHT; /* Bit 7: RIGHT */
#endif
    return buttons;
}

static void ppu_write_string(uint16_t ppu_addr, const char *str) {
#if defined(__NES__) || defined(__CC65__)
    (void)PPU_STATUS; /* Reset PPU address latch */
    PPU_ADDR = (uint8_t)(ppu_addr >> 8);
    PPU_ADDR = (uint8_t)(ppu_addr & 0xFF);
    while (*str) {
        PPU_DATA = (uint8_t)(*str++);
    }
#else
    (void)ppu_addr; (void)str;
#endif
}

void get_note_name(uint8_t note, char *out) {
    uint8_t note_idx;
    int octave;
    static const char * const note_names[12] = {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
    };

    if (note == RDJ_NOTE_EMPTY || note == 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '-'; out[3] = '\0';
        return;
    }
    if (note == RDJ_NOTE_OFF) {
        out[0] = 'O'; out[1] = 'F'; out[2] = 'F'; out[3] = '\0';
        return;
    }
    note_idx = (note - 12) % 12;
    octave = (note - 12) / 12;
    if (octave < 0) octave = 0;
    if (octave > 9) octave = 9;
    sprintf(out, "%s%d", note_names[note_idx], octave);
}

void ui_init(void) {
    int r, c;
    g_song.bpm = 120;
    g_song.speed = 6;
    g_song.order_length = 1;
    g_song.order[0] = 0;

    for (r = 0; r < RDJ_MAX_ROWS; r++) {
        for (c = 0; c < RDJ_MAX_CHANNELS; c++) {
            g_song.patterns[0].rows[r][c].note = RDJ_NOTE_EMPTY;
            g_song.patterns[0].rows[r][c].instrument = 0;
            g_song.patterns[0].rows[r][c].volume = 0;
            g_song.patterns[0].rows[r][c].fx_cmd = 0;
            g_song.patterns[0].rows[r][c].fx_val = 0;
        }
    }
    g_song.patterns[0].length = 32;
    g_song.patterns[0].speed = 6;

    input_mapper_init(&g_tracker_state);

    s_tick_counter = 0;
    s_current_play_row = 0;
    s_prev_is_playing = 0;

#if defined(__NES__) || defined(__CC65__)
    /* Turn off PPU screen rendering before palette setup */
    PPU_MASK = 0x00;

    /* Reset address latch and write visible palette to $3F00 */
    (void)PPU_STATUS;
    PPU_ADDR = 0x3F;
    PPU_ADDR = 0x00;
    PPU_DATA = 0x0F; /* Background: Black */
    PPU_DATA = 0x30; /* Color 1: White text */
    PPU_DATA = 0x16; /* Color 2: Red */
    PPU_DATA = 0x27; /* Color 3: Orange */

    /* Reset PPU Scroll & Address registers */
    (void)PPU_STATUS;
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;

    /* Turn on PPU rendering: Enable BG & Sprites, show in leftmost 8px ($1E) */
    PPU_MASK = 0x1E;
#endif
}

static void ui_trigger_row_notes(uint8_t row) {
    uint8_t ch;
    for (ch = 0; ch < 4; ch++) {
        rdj_row_t *r = &g_song.patterns[0].rows[row][ch];
        if (r->note != RDJ_NOTE_EMPTY && r->note != RDJ_NOTE_OFF) {
            nes_2a03_driver.note_on(ch, r->note, r->instrument, r->volume ? r->volume : 128);
        } else if (r->note == RDJ_NOTE_OFF) {
            nes_2a03_driver.note_off(ch);
        }
    }
}

void ui_process_playback(void) {
    uint8_t speed, pat_len, ch;
    if (g_tracker_state.is_playing) {
        if (!s_prev_is_playing) {
            s_prev_is_playing = 1;
            s_tick_counter = 0;
            s_current_play_row = g_tracker_state.cursor_row;
            ui_trigger_row_notes(s_current_play_row);
        } else {
            s_tick_counter++;
            speed = g_song.patterns[0].speed ? g_song.patterns[0].speed : (g_song.speed ? g_song.speed : 6);
            if (s_tick_counter >= speed) {
                s_tick_counter = 0;
                pat_len = g_song.patterns[0].length ? g_song.patterns[0].length : RDJ_MAX_ROWS;
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
            for (ch = 0; ch < 4; ch++) {
                nes_2a03_driver.note_off(ch);
            }
        }
    }
}

void ui_update(void) {
    uint16_t joy = nes_read_controller();
    input_mapper_update(&g_tracker_state, &g_song, joy);
    ui_process_playback();
}

void ui_render(void) {
#if defined(__NES__) || defined(__CC65__)
    char buf[33];
    char row_str[33];
    char note_name[4];
    char chan_buf[8];
    uint8_t start_row, i, r, c;
    bool is_cursor_row, is_curr_cell;
    char sub_note, sub_inst;

    /* 1. Header Status Bar */
    sprintf(buf, "BPM:%03d [%s] OCT:%d CH:%d",
            g_song.bpm,
            g_tracker_state.is_playing ? "PLAY" : "STOP",
            g_tracker_state.octave,
            g_tracker_state.cursor_channel + 1);

    ppu_write_string(0x2022, "--- RetroDJ NES Tracker ---");
    ppu_write_string(0x2062, buf);

    ppu_write_string(0x20A2, "ROW| SQ1   SQ2   TRI   NOI  ");
    ppu_write_string(0x20C2, "---|------------------------");

    /* 2. Tracker Pattern Grid (Render 12 rows for 32x28 screen limit) */
    start_row = g_tracker_state.cursor_row >= 6 ? g_tracker_state.cursor_row - 6 : 0;
    if (start_row > RDJ_MAX_ROWS - 12) start_row = RDJ_MAX_ROWS - 12;

    for (i = 0; i < 12; i++) {
        r = start_row + i;
        is_cursor_row = (r == g_tracker_state.cursor_row);

        sprintf(row_str, "%02X%c|", r, is_cursor_row ? '>' : ' ');

        for (c = 0; c < 4; c++) {
            rdj_row_t *cell = &g_song.patterns[0].rows[r][c];
            get_note_name(cell->note, note_name);

            is_curr_cell = is_cursor_row && (g_tracker_state.cursor_channel == c);

            sub_note = (is_curr_cell && g_tracker_state.cursor_subcol == RDJ_SUBCOL_NOTE) ? '*' : ' ';
            sub_inst = (is_curr_cell && g_tracker_state.cursor_subcol == RDJ_SUBCOL_INST) ? '*' : ' ';

            sprintf(chan_buf, "%s%c%X%c|", note_name, sub_note, cell->instrument & 0xF, sub_inst);
            strcat(row_str, chan_buf);
        }

        ppu_write_string(0x20E2 + (i * 32), row_str);
    }

    ppu_write_string(0x2380, "--------------------------------");
    ppu_write_string(0x23A0, "A+DIR:Note B+DIR:Val START:Play");

    /* Reset scroll position and address latch after VBlank drawing */
    (void)PPU_STATUS;
    PPU_SCROLL = 0x00;
    PPU_SCROLL = 0x00;
#endif
}

input_tracker_state_t *ui_get_tracker_state(void) {
    return &g_tracker_state;
}

rdj_song_t *ui_get_song(void) {
    return &g_song;
}
