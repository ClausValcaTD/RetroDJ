/* RDJ Driver Integration Test */
/* GPLv3 - RetroDJ Project */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "core/rdj_driver.h"
#include "core/sequencer.h"
#include "core/rdj_backend.h"
#include "targets/genesis/src/input_mapper.h"
#include "targets/genesis/src/ui.h"

extern void vgm_open(const char *filename);

static void test_input_mapper(void) {
    printf("Testing Tracker UI & Input Mapper...\n");
    ui_init();

    input_tracker_state_t *state = ui_get_tracker_state();
    rdj_song_t *song = ui_get_song();

    assert(state != NULL);
    assert(song != NULL);

    /* Check initial state */
    assert(state->cursor_row == 0);
    assert(state->cursor_channel == 0);
    assert(state->cursor_subcol == RDJ_SUBCOL_NOTE);
    assert(state->is_playing == 0);

    /* 1. Test D-Pad navigation (DOWN) */
    input_mapper_update(state, song, BUTTON_DOWN);
    assert(state->cursor_row == 1);
    input_mapper_update(state, song, 0); /* Release button */

    /* 2. Test D-Pad navigation (RIGHT - sub-column move) */
    input_mapper_update(state, song, BUTTON_RIGHT);
    assert(state->cursor_subcol == RDJ_SUBCOL_INST);
    input_mapper_update(state, song, 0);

    /* 3. Test BUTTON_A + UP (Set initial note & Transpose +1 semitone) */
    state->cursor_subcol = RDJ_SUBCOL_NOTE;
    input_mapper_update(state, song, BUTTON_A | BUTTON_UP);
    /* Octave 4 default -> 4*12 + 12 = 60 (C-4) */
    uint8_t note_val = song->patterns[0].rows[1][0].note;
    assert(note_val == 60);
    input_mapper_update(state, song, BUTTON_A); /* Release UP while holding A */

    /* Transpose +1 semitone -> C#4 (61) */
    input_mapper_update(state, song, BUTTON_A | BUTTON_UP);
    assert(song->patterns[0].rows[1][0].note == 61);
    input_mapper_update(state, song, 0);

    /* 4. Test BUTTON_A + RIGHT (Transpose +1 octave = +12 semitones -> 73) */
    input_mapper_update(state, song, BUTTON_A | BUTTON_RIGHT);
    assert(song->patterns[0].rows[1][0].note == 73);
    input_mapper_update(state, song, 0);

    /* 5. Test BUTTON_B + UP on Instrument (increment inst) */
    state->cursor_subcol = RDJ_SUBCOL_INST;
    input_mapper_update(state, song, BUTTON_B | BUTTON_UP);
    assert(song->patterns[0].rows[1][0].instrument == 1);
    input_mapper_update(state, song, 0);

    /* 6. Test BUTTON_B + RIGHT on Volume (increment vol by 16) */
    state->cursor_subcol = RDJ_SUBCOL_VOL;
    input_mapper_update(state, song, BUTTON_B | BUTTON_RIGHT);
    assert(song->patterns[0].rows[1][0].volume == 16);
    input_mapper_update(state, song, 0);

    /* 7. Test BUTTON_B + BUTTON_A (Clear current cell) */
    input_mapper_update(state, song, BUTTON_B | BUTTON_A);
    assert(song->patterns[0].rows[1][0].note == RDJ_NOTE_EMPTY);
    assert(song->patterns[0].rows[1][0].instrument == 0);
    assert(song->patterns[0].rows[1][0].volume == 0);
    input_mapper_update(state, song, 0);

    /* 8. Test BUTTON_START (Toggle Play / Stop) */
    assert(state->is_playing == 0);
    input_mapper_update(state, song, BUTTON_START);
    assert(state->is_playing == 1);
    input_mapper_update(state, song, 0);
    input_mapper_update(state, song, BUTTON_START);
    assert(state->is_playing == 0);
    input_mapper_update(state, song, 0);

    printf("Input Mapper & Tracker UI tests passed successfully!\n");
}

int main(void) {
    printf("Testing Backends...\n");
    rdj_set_backend(&backend_null);
    assert(rdj_active_backend == &backend_null);

    printf("Testing YM2612 Driver with Null Backend...\n");
    ym2612_driver.init(7670454);
    ym2612_driver.write_reg(0x002B, 0x00);
    ym2612_driver.note_on(0, 60, 0, 128);
    ym2612_driver.note_off(0);

    printf("Testing NES 2A03 Driver with VGM Backend...\n");
    vgm_open("test_output.vgm");
    rdj_set_backend(&backend_vgm);
    nes_2a03_driver.init(1789773);
    nes_2a03_driver.note_on(0, 60, 0, 128);

    if (rdj_active_backend->wait_frame) {
        rdj_active_backend->wait_frame(); /* 735 samples */
    }
    if (rdj_active_backend->wait_samples) {
        rdj_active_backend->wait_samples(100); /* 100 samples */
    }

    nes_2a03_driver.note_off(0);

    /* Switch backend to flush/close VGM */
    rdj_set_backend(&backend_null);

    /* Verify VGM file header values */
    FILE *f = fopen("test_output.vgm", "rb");
    assert(f != NULL);

    fseek(f, 0x18, SEEK_SET);
    uint32_t total_samples = 0;
    size_t read_bytes = fread(&total_samples, sizeof(uint32_t), 1, f);
    assert(read_bytes == 1);
    assert(total_samples == 835); /* 735 + 100 */

    fseek(f, 0x04, SEEK_SET);
    uint32_t eof_offset = 0;
    read_bytes = fread(&eof_offset, sizeof(uint32_t), 1, f);
    assert(read_bytes == 1);
    assert(eof_offset > 0);
    fclose(f);

    printf("VGM wait & sample tracking verification passed! Total samples = %u, EOF offset = %u\n", total_samples, eof_offset);

    printf("Testing YM2612 Driver with Serial Backend...\n");
    rdj_set_backend(&backend_serial);
    ym2612_driver.note_on(1, 64, 0, 128);
    ym2612_driver.note_off(1);

    rdj_set_backend(&backend_null);

    test_input_mapper();

    printf("All RetroDJ backend tests passed successfully!\n");
    return 0;
}
