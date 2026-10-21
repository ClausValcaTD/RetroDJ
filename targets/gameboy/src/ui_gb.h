/* RetroDJ - Game Boy UI Header */
/* GPLv3 - RetroDJ Project */

#ifndef RDJ_GB_UI_H
#define RDJ_GB_UI_H

#if defined(__PORT_sm83) || defined(__TARGET_gb) || defined(GBDK)
    #include <gb/gb.h>
    #include <stdint.h>
    #include <stddef.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "targets/genesis/src/input_mapper.h"
#include "core/sequencer.h"
#include "core/rdj_driver.h"
#include "core/rdj_backend.h"

extern rdj_driver_t gb_sound_driver;
extern rdj_backend_t backend_gb_hw;

void ui_init(void);
void ui_update(void);
void ui_render(void);
void ui_process_playback(void);

void get_note_name(uint8_t note, char *out);
input_tracker_state_t *ui_get_tracker_state(void);
rdj_song_t *ui_get_song(void);

#endif
