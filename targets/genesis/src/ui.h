/* RetroDJ - Sega Genesis UI Header */
/* GPLv3 - RetroDJ Project */

#ifndef RDJ_GENESIS_UI_H
#define RDJ_GENESIS_UI_H

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
#endif

#include "input_mapper.h"
#include "core/sequencer.h"

void ui_init(void);
void ui_update(void);
void ui_render(void);

void get_note_name(uint8_t note, char *out);
input_tracker_state_t *ui_get_tracker_state(void);
rdj_song_t *ui_get_song(void);

#endif
