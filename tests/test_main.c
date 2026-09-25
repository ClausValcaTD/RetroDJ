/* RDJ Driver Integration Test */
/* GPLv3 - RetroDJ Project */

#include <stdio.h>
#include <assert.h>
#include "../core/rdj_driver.h"
#include "../core/sequencer.h"
#include "../core/rdj_backend.h"

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
    rdj_set_backend(&backend_vgm);
    nes_2a03_driver.init(1789773);
    nes_2a03_driver.note_on(0, 60, 0, 128);
    nes_2a03_driver.note_off(0);

    printf("Testing YM2612 Driver with Serial Backend...\n");
    rdj_set_backend(&backend_serial);
    ym2612_driver.note_on(1, 64, 0, 128);
    ym2612_driver.note_off(1);

    rdj_set_backend(&backend_null);
    printf("All RetroDJ backend tests passed successfully!\n");
    return 0;
}
