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

void ui_init(void);
void ui_update(void);
void ui_render(void);

#endif
