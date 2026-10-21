/* RetroDJ - NES Tracker UI Header */
/* GPLv3 - RetroDJ Project */

#ifndef RDJ_NES_UI_H
#define RDJ_NES_UI_H

#include <stdint.h>
#include <stdbool.h>

#include "targets/genesis/src/input_mapper.h"
#include "core/sequencer.h"

void nes_vblank_wait(void);
uint16_t nes_read_controller(void);

void ui_init(void);
void ui_update(void);
void ui_render(void);

input_tracker_state_t *ui_get_tracker_state(void);
rdj_song_t *ui_get_song(void);

#endif
