/* RetroDJ - Genesis Input Mapper Implementation */
/* GPLv3 - RetroDJ Project */

#include "input_mapper.h"

extern void ym2612_rdj_note_on(uint8_t channel, uint8_t note, uint8_t instrument);
extern void ym2612_rdj_note_off(uint8_t channel);

void input_mapper_init(input_tracker_state_t *state) {
    if (!state) return;
    state->cursor_row = 0;
    state->cursor_channel = 0;
    state->cursor_subcol = RDJ_SUBCOL_NOTE;
    state->octave = 4;
    state->is_playing = 0;
    state->last_joy = 0;
}

void input_mapper_update(input_tracker_state_t *state, rdj_song_t *song, uint16_t joy_state) {
    uint16_t pressed;
    bool btn_a;
    bool btn_b;
    rdj_row_t *cell;

    if (!state || !song) return;

    pressed = joy_state & ~state->last_joy;
    btn_a = (joy_state & BUTTON_A) != 0;
    btn_b = (joy_state & BUTTON_B) != 0;

    /* BUTTON_B + BUTTON_A combo: Clear current cell */
    if ((btn_b && (pressed & BUTTON_A)) || (btn_a && (pressed & BUTTON_B))) {
        cell = &song->patterns[0].rows[state->cursor_row][state->cursor_channel];
        cell->note = RDJ_NOTE_EMPTY;
        cell->instrument = 0;
        cell->volume = 0;
        cell->fx_cmd = 0;
        cell->fx_val = 0;
        state->last_joy = joy_state;
        return;
    }

    /* BUTTON_START: Toggle playback */
    if (pressed & BUTTON_START) {
        state->is_playing = !state->is_playing;
    }

    /* BUTTON_C: Play audition note at current cursor */
    if (pressed & BUTTON_C) {
        uint8_t audition_note;
        cell = &song->patterns[0].rows[state->cursor_row][state->cursor_channel];
        audition_note = (cell->note != RDJ_NOTE_EMPTY && cell->note != RDJ_NOTE_OFF) ? cell->note : (state->octave * 12 + 12);
#if !defined(__NES__) && !defined(__CC65__)
        ym2612_rdj_note_on(state->cursor_channel, audition_note, cell->instrument);
#endif
    } else if ((state->last_joy & BUTTON_C) && !(joy_state & BUTTON_C)) {
#if !defined(__NES__) && !defined(__CC65__)
        ym2612_rdj_note_off(state->cursor_channel);
#endif
    }

    /* Directional handling based on modifiers */
    if (btn_a && !btn_b) {
        /* BUTTON_A + UP/DOWN: Transpose note by +/- 1 semitone */
        /* BUTTON_A + LEFT/RIGHT: Transpose note by +/- 1 octave (12 semitones) */
        cell = &song->patterns[0].rows[state->cursor_row][state->cursor_channel];

        if (pressed & BUTTON_UP) {
            if (cell->note == RDJ_NOTE_EMPTY) cell->note = state->octave * 12 + 12;
            else if (cell->note < 127) cell->note++;
        }
        if (pressed & BUTTON_DOWN) {
            if (cell->note == RDJ_NOTE_EMPTY) cell->note = state->octave * 12 + 12;
            else if (cell->note > 1) cell->note--;
        }
        if (pressed & BUTTON_RIGHT) {
            if (cell->note == RDJ_NOTE_EMPTY) cell->note = state->octave * 12 + 12;
            else if (cell->note <= 115) cell->note += 12;
            else cell->note = 127;
        }
        if (pressed & BUTTON_LEFT) {
            if (cell->note == RDJ_NOTE_EMPTY) cell->note = state->octave * 12 + 12;
            else if (cell->note > 12) cell->note -= 12;
            else cell->note = 1;
        }
    } else if (btn_b && !btn_a) {
        /* BUTTON_B + UP/DOWN: Increment/decrement current value by 1 */
        /* BUTTON_B + LEFT/RIGHT: Increment/decrement current value by 16 (0x10) */
        int delta = 0;
        cell = &song->patterns[0].rows[state->cursor_row][state->cursor_channel];

        if (pressed & BUTTON_UP) delta = 1;
        if (pressed & BUTTON_DOWN) delta = -1;
        if (pressed & BUTTON_RIGHT) delta = 16;
        if (pressed & BUTTON_LEFT) delta = -16;

        if (delta != 0) {
            switch (state->cursor_subcol) {
                case RDJ_SUBCOL_NOTE: {
                    int new_note;
                    if (cell->note == RDJ_NOTE_EMPTY) cell->note = state->octave * 12 + 12;
                    new_note = (int)cell->note + delta;
                    if (new_note < 1) new_note = 1;
                    if (new_note > 127) new_note = 127;
                    cell->note = (uint8_t)new_note;
                    break;
                }
                case RDJ_SUBCOL_INST: {
                    int new_inst = (int)cell->instrument + delta;
                    if (new_inst < 0) new_inst = 0;
                    if (new_inst > RDJ_MAX_INSTR - 1) new_inst = RDJ_MAX_INSTR - 1;
                    cell->instrument = (uint8_t)new_inst;
                    break;
                }
                case RDJ_SUBCOL_VOL: {
                    int new_vol = (int)cell->volume + delta;
                    if (new_vol < 0) new_vol = 0;
                    if (new_vol > 128) new_vol = 128;
                    cell->volume = (uint8_t)new_vol;
                    break;
                }
                case RDJ_SUBCOL_FX: {
                    int new_fx = (int)cell->fx_val + delta;
                    if (new_fx < 0) new_fx = 0;
                    if (new_fx > 255) new_fx = 255;
                    cell->fx_val = (uint8_t)new_fx;
                    break;
                }
            }
        }
    } else if (!btn_a && !btn_b) {
        /* Normal Cursor Navigation */
        if (pressed & BUTTON_UP) {
            if (state->cursor_row > 0) state->cursor_row--;
            else state->cursor_row = RDJ_MAX_ROWS - 1;
        }
        if (pressed & BUTTON_DOWN) {
            if (state->cursor_row < RDJ_MAX_ROWS - 1) state->cursor_row++;
            else state->cursor_row = 0;
        }
        if (pressed & BUTTON_LEFT) {
            if (state->cursor_subcol > 0) {
                state->cursor_subcol--;
            } else if (state->cursor_channel > 0) {
                state->cursor_channel--;
                state->cursor_subcol = RDJ_NUM_SUBCOLS - 1;
            }
        }
        if (pressed & BUTTON_RIGHT) {
            if (state->cursor_subcol < RDJ_NUM_SUBCOLS - 1) {
                state->cursor_subcol++;
            } else if (state->cursor_channel < RDJ_MAX_CHANNELS - 1) {
                state->cursor_channel++;
                state->cursor_subcol = 0;
            }
        }
    }

    state->last_joy = joy_state;
}
