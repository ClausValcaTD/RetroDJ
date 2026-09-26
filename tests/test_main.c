/* RDJ Driver Integration Test */
/* GPLv3 - RetroDJ Project */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "core/rdj_driver.h"
#include "core/sequencer.h"
#include "core/rdj_backend.h"

extern void vgm_open(const char *filename);

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
    printf("All RetroDJ backend tests passed successfully!\n");
    return 0;
}
