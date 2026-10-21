/* RetroDJ - Game Boy Tracker UI Implementation */
/* GPLv3 - RetroDJ Project */

#include "ui_gb.h"
#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

#include <stdio.h>
#include <string.h>

#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
#include <gb/gb.h>
#include <gbdk/font.h>
#include <gbdk/console.h>
#endif

static rdj_song_t g_song;
static input_tracker_state_t g_tracker_state;

static uint8_t s_tick_counter = 0;
static uint8_t s_current_play_row = 0;
static uint8_t s_prev_is_playing = 0;

static uint8_t s_last_cursor_row = 0;
static uint8_t s_last_cursor_chan = 0;
static uint8_t s_last_cursor_subcol = 0;
static uint8_t s_last_start_row = 0;
static uint8_t s_last_status_bpm = 0;
static uint8_t s_last_status_oct = 0;
static uint8_t s_last_status_ch = 0;
static uint8_t s_last_status_playing = 0;

static char hex_char(uint8_t val) {
    val &= 0x0F;
    return (val < 10) ? ('0' + val) : ('A' + (val - 10));
}

void get_note_name(uint8_t note, char *out) {
    uint8_t note_idx;
    int octave;
    static const char note_names[12][2] = {
        {'C', '-'}, {'C', '#'}, {'D', '-'}, {'D', '#'},
        {'E', '-'}, {'F', '-'}, {'F', '#'}, {'G', '-'},
        {'G', '#'}, {'A', '-'}, {'A', '#'}, {'B', '-'}
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

    out[0] = note_names[note_idx][0];
    out[1] = note_names[note_idx][1];
    out[2] = (char)('0' + octave);
    out[3] = '\0';
}

static void format_status_str(char *out) {
    uint16_t bpm = g_song.bpm;
    uint8_t oct = g_tracker_state.octave;
    uint8_t ch = g_tracker_state.cursor_channel + 1;
    bool playing = g_tracker_state.is_playing;

    /* Fits 20 columns: "BPM:120 [STOP] O:4 C:1" */
    out[0] = 'B'; out[1] = 'P'; out[2] = 'M'; out[3] = ':';
    out[4] = (char)('0' + ((bpm / 100) % 10));
    out[5] = (char)('0' + ((bpm / 10) % 10));
    out[6] = (char)('0' + (bpm % 10));
    out[7] = ' '; out[8] = '[';
    if (playing) {
        out[9] = 'P'; out[10] = 'L'; out[11] = 'A'; out[12] = 'Y';
    } else {
        out[9] = 'S'; out[10] = 'T'; out[11] = 'O'; out[12] = 'P';
    }
    out[13] = ']'; out[14] = ' ';
    out[15] = 'O'; out[16] = ':';
    out[17] = (char)('0' + (oct % 10));
    out[18] = ' ';
    out[19] = (char)('0' + (ch % 10));
    out[20] = '\0';
}

static void format_row_str(uint8_t r, char *out) {
    bool is_cursor_row = (r == g_tracker_state.cursor_row);
    char note_name[4];
    uint8_t c;
    int pos = 0;

    /* 20 columns: "00>|C-4*0|C-4 0|..." */
    out[pos++] = hex_char(r >> 4);
    out[pos++] = hex_char(r);
    out[pos++] = is_cursor_row ? '>' : ' ';
    out[pos++] = '|';

    for (c = 0; c < 4; c++) {
        rdj_row_t *cell = &g_song.patterns[0].rows[r][c];
        bool is_curr_cell = is_cursor_row && (g_tracker_state.cursor_channel == c);
        char sub_marker = (is_curr_cell) ? ((g_tracker_state.cursor_subcol == RDJ_SUBCOL_INST) ? 'i' : '*') : ' ';

        get_note_name(cell->note, note_name);

        out[pos++] = note_name[0];
        out[pos++] = note_name[1];
        out[pos++] = note_name[2];
        out[pos++] = sub_marker;
    }
    out[pos] = '\0';
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

    s_last_cursor_row = g_tracker_state.cursor_row;
    s_last_cursor_chan = g_tracker_state.cursor_channel;
    s_last_cursor_subcol = g_tracker_state.cursor_subcol;
    s_last_start_row = 0;
    s_last_status_bpm = (uint8_t)g_song.bpm;
    s_last_status_oct = g_tracker_state.octave;
    s_last_status_ch = g_tracker_state.cursor_channel;
    s_last_status_playing = g_tracker_state.is_playing;

#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    font_t min_font;
    font_init();
    min_font = font_load(font_ibm);
    font_set(min_font);

    /* Clear screen and draw initial layout */
    cls();
    gotoxy(0, 0);
    printf("--- RetroDJ GameBoy ---");

    char init_str[24];
    format_status_str(init_str);
    gotoxy(0, 1);
    printf("%s", init_str);

    gotoxy(0, 2);
    printf("RW |PU1 PU2 WAV NOI ");
    gotoxy(0, 3);
    printf("---|----------------");

    for (uint8_t i = 0; i < 12; i++) {
        format_row_str(i, init_str);
        gotoxy(0, 4 + i);
        printf("%s", init_str);
    }

    gotoxy(0, 16);
    printf("--------------------");
    gotoxy(0, 17);
    printf("A+DIR:Note B+DIR:Val");

    SHOW_BKG;
#endif
}

static void ui_trigger_row_notes(uint8_t row) {
    uint8_t ch;
    for (ch = 0; ch < 4; ch++) {
        rdj_row_t *r = &g_song.patterns[0].rows[row][ch];
        if (r->note != RDJ_NOTE_EMPTY && r->note != RDJ_NOTE_OFF) {
            gb_sound_driver.note_on(ch, r->note, r->instrument, r->volume ? r->volume : 128);
        } else if (r->note == RDJ_NOTE_OFF) {
            gb_sound_driver.note_off(ch);
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
                gb_sound_driver.note_off(ch);
            }
        }
    }
}

static uint16_t gb_read_controller(void) {
    uint16_t buttons = 0;
#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    uint8_t joy = joypad();
    if (joy & J_A)      buttons |= BUTTON_A;
    if (joy & J_B)      buttons |= BUTTON_B;
    if (joy & J_START)  buttons |= BUTTON_START;
    if (joy & J_UP)     buttons |= BUTTON_UP;
    if (joy & J_DOWN)   buttons |= BUTTON_DOWN;
    if (joy & J_LEFT)   buttons |= BUTTON_LEFT;
    if (joy & J_RIGHT)  buttons |= BUTTON_RIGHT;
#endif
    return buttons;
}

void ui_update(void) {
    uint16_t joy = gb_read_controller();
    input_mapper_update(&g_tracker_state, &g_song, joy);
    ui_process_playback();
}

void ui_render(void) {
#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    char cur_str[24];
    uint8_t start_row, r, i;

    /* 1. Update status bar if changed */
    if (g_song.bpm != s_last_status_bpm ||
        g_tracker_state.octave != s_last_status_oct ||
        g_tracker_state.cursor_channel != s_last_status_ch ||
        g_tracker_state.is_playing != s_last_status_playing) {

        format_status_str(cur_str);
        gotoxy(0, 1);
        printf("%s", cur_str);

        s_last_status_bpm = (uint8_t)g_song.bpm;
        s_last_status_oct = g_tracker_state.octave;
        s_last_status_ch = g_tracker_state.cursor_channel;
        s_last_status_playing = g_tracker_state.is_playing;
    }

    /* 2. Calculate visible start_row */
    start_row = g_tracker_state.cursor_row >= 6 ? g_tracker_state.cursor_row - 6 : 0;
    if (start_row > RDJ_MAX_ROWS - 12) start_row = RDJ_MAX_ROWS - 12;

    /* 3. Handle full grid redrawing if start_row changed */
    if (start_row != s_last_start_row) {
        s_last_start_row = start_row;
        for (i = 0; i < 12; i++) {
            r = start_row + i;
            format_row_str(r, cur_str);
            gotoxy(0, 4 + i);
            printf("%s", cur_str);
        }
        s_last_cursor_row = g_tracker_state.cursor_row;
        s_last_cursor_chan = g_tracker_state.cursor_channel;
        s_last_cursor_subcol = g_tracker_state.cursor_subcol;
        return;
    }

    /* 4. Update old and new cursor rows if cursor moved */
    if (g_tracker_state.cursor_row != s_last_cursor_row) {
        /* Render previous row without cursor */
        r = s_last_cursor_row;
        if (r >= start_row && r < start_row + 12) {
            i = r - start_row;
            format_row_str(r, cur_str);
            gotoxy(0, 4 + i);
            printf("%s", cur_str);
        }

        s_last_cursor_row = g_tracker_state.cursor_row;
        s_last_cursor_chan = g_tracker_state.cursor_channel;
        s_last_cursor_subcol = g_tracker_state.cursor_subcol;

        /* Render new active row with cursor */
        r = g_tracker_state.cursor_row;
        if (r >= start_row && r < start_row + 12) {
            i = r - start_row;
            format_row_str(r, cur_str);
            gotoxy(0, 4 + i);
            printf("%s", cur_str);
        }
        return;
    }

    /* 5. Update active row if channel/subcol or cell content changed */
    r = g_tracker_state.cursor_row;
    if (r >= start_row && r < start_row + 12) {
        i = r - start_row;
        format_row_str(r, cur_str);
        gotoxy(0, 4 + i);
        printf("%s", cur_str);
    }

    s_last_cursor_chan = g_tracker_state.cursor_channel;
    s_last_cursor_subcol = g_tracker_state.cursor_subcol;
#endif
}

input_tracker_state_t *ui_get_tracker_state(void) {
    return &g_tracker_state;
}

rdj_song_t *ui_get_song(void) {
    return &g_song;
}
