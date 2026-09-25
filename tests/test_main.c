/* RDJ Driver Integration Test */
/* GPLv3 - RetroDJ Project */

#include <stdio.h>
#include <assert.h>
#include "../core/rdj_driver.h"
#include "../core/sequencer.h"

/* External function prototypes to test driver specific functions */
void ym2612_rdj_init(uint32_t clock);
void ym2612_rdj_reset(void);
void ym2612_rdj_write(uint8_t port, uint8_t reg, uint8_t val);
void ym2612_rdj_note_on(uint8_t channel, uint8_t note, uint8_t instrument);
void ym2612_rdj_note_off(uint8_t channel);

void apu_rdj_init(uint32_t cpu_clock);
void apu_rdj_reset(void);
void apu_rdj_write(uint16_t addr, uint8_t val);
void apu_rdj_enable_channels(uint8_t mask);
void apu_rdj_square_note_on(uint8_t ch, uint8_t note, uint8_t duty, uint8_t vol);
void apu_rdj_square_note_off(uint8_t ch);
void apu_rdj_triangle_note_on(uint8_t note);
void apu_rdj_triangle_note_off(void);
void apu_rdj_noise_on(uint8_t period, uint8_t vol);
void apu_rdj_noise_off(void);

int main(void) {
    printf("Testing YM2612 Driver...\n");
    ym2612_driver.init(7670454);
    ym2612_driver.write_reg(0x002B, 0x00);
    ym2612_driver.note_on(0, 60, 0, 128);
    ym2612_driver.note_off(0);
    ym2612_driver.reset();
    ym2612_driver.shutdown();

    printf("Testing NES 2A03 APU Driver...\n");
    nes_2a03_driver.init(1789773);
    nes_2a03_driver.write_reg(0x4015, 0x0F);
    nes_2a03_driver.note_on(0, 60, 0, 128); /* Square 1 */
    nes_2a03_driver.note_on(2, 60, 0, 128); /* Triangle */
    nes_2a03_driver.note_on(3, 10, 0, 128); /* Noise */
    nes_2a03_driver.note_off(0);
    nes_2a03_driver.note_off(2);
    nes_2a03_driver.note_off(3);
    apu_rdj_enable_channels(0x0F);
    apu_rdj_square_note_on(0, 60, 2, 15);
    apu_rdj_square_note_off(0);
    apu_rdj_triangle_note_on(60);
    apu_rdj_triangle_note_off();
    apu_rdj_noise_on(8, 15);
    apu_rdj_noise_off();
    nes_2a03_driver.reset();
    nes_2a03_driver.shutdown();

    printf("Testing Sequencer Song Data Structure...\n");
    rdj_song_t song;
    song.bpm = 120;
    song.speed = 6;
    song.driver = &ym2612_driver;
    assert(song.driver->num_channels == 6);

    printf("All RetroDJ tests passed successfully!\n");
    return 0;
}
