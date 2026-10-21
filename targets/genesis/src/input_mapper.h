/* RetroDJ - Genesis Input Mapper Header */
/* GPLv3 - RetroDJ Project */

#ifndef RDJ_GENESIS_INPUT_MAPPER_H
#define RDJ_GENESIS_INPUT_MAPPER_H

#if defined(RDJ_GENESIS_HARDWARE) || defined(SGDK_GCC) || defined(__m68k__)
    #include <genesis.h>
#else
    #include <stdint.h>
    #include <stddef.h>
    #include <stdbool.h>

    #ifndef BUTTON_UP
    #define BUTTON_UP    0x0001
    #define BUTTON_DOWN  0x0002
    #define BUTTON_LEFT  0x0004
    #define BUTTON_RIGHT 0x0008
    #define BUTTON_A     0x0040
    #define BUTTON_B     0x0010
    #define BUTTON_C     0x0020
    #define BUTTON_START 0x0080
    #endif
#endif

#include "core/sequencer.h"

#define RDJ_SUBCOL_NOTE 0
#define RDJ_SUBCOL_INST 1
#define RDJ_SUBCOL_VOL  2
#define RDJ_SUBCOL_FX   3
#define RDJ_NUM_SUBCOLS 4

typedef struct {
    uint8_t cursor_row;      /* 0..63 */
    uint8_t cursor_channel;  /* 0..5 */
    uint8_t cursor_subcol;   /* 0..3 */
    uint8_t octave;          /* 0..9, default 4 */
    uint8_t is_playing;      /* 0=Stop, 1=Play */
    uint16_t last_joy;       /* Previous joypad state */
} input_tracker_state_t;

void input_mapper_init(input_tracker_state_t *state);
void input_mapper_update(input_tracker_state_t *state, rdj_song_t *song, uint16_t joy_state);

#endif
