/* RetroDJ - Sega Genesis UI Header */
/* GPLv3 - RetroDJ Project */

#ifndef RDJ_GENESIS_UI_H
#define RDJ_GENESIS_UI_H

#ifdef RDJ_GENESIS_HARDWARE
    #include <genesis.h>
#else
    #include <stdint.h>
#endif

void ui_init(void);
void ui_update(void);
void ui_render(void);

#endif
