#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <gpiointerrupt.h>
#include <spidrv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "util.h"
#include "synth.h"

Synth synth = {
    .wavegens = {
        {.freq = F_SAMPLE / 261.63, .vol = 1.0}, /* C4 */
        {.freq = F_SAMPLE / 392.00, .vol = 1.0}, /* G4 */
    },
    .vol = 1.0
};

volatile bool synth_dirty = true;
SPIDRV_HandleData_t synth_spi;

SPIDRV_Init_t synth_spi_init = {
    .port = USART1,
    .portLocation = _USART_ROUTE_LOCATION_LOC1,
    .bitRate = 1000000,
    .frameLength = 8,
    .type = spidrvMaster,
    .bitOrder = spidrvBitOrderLsbFirst,
    .clockMode = spidrvClockMode0,
    .csControl = spidrvCsControlApplication,
};

EnvelopeStep press_envelope[] = {
    ENVELOPE_STEP(1.0, 0.2 * F_SAMPLE),
    ENVELOPE_STEP(-0.2, 0.2 * F_SAMPLE),
};

EnvelopeStep release_envelope[] = {
    ENVELOPE_STEP(-1.0, 0.2 * F_SAMPLE),
};

Ecode_t ecode = ECODE_OK; /* for debugging */

void validate_synth_complete(SPIDRV_Handle_t handle, Ecode_t status, int nbytes) {
    synth_clearcmds(&synth);
    synth_dirty = false;
    //TODO: release chip select
}

void validate_synth(void) {
    if (!synth_dirty) return;
    ecode = SPIDRV_AbortTransfer(&synth_spi);
    if (ecode != ECODE_EMDRV_SPIDRV_OK && ecode != ECODE_EMDRV_SPIDRV_IDLE) {
        exit(1);
    }
    // TODO: assert chip select (but don't release it after abort!)
    ecode = SPIDRV_MTransmit(&synth_spi, (void *)&synth, sizeof(synth), validate_synth_complete);
    if (ecode != ECODE_EMDRV_SPIDRV_OK) exit(1);
}

void handle_button(uint8_t pin) {
    WaveGen *gen;

    switch (pin) {
    case 9:
        gen = &synth.wavegens[0];
        break;
    case 10:
        gen = &synth.wavegens[1];
        break;
    default:
        return;
    }

    if (GPIO_PinInGet(gpioPortB, pin)) {
        wavegen_set_vol_envelope(gen, press_envelope, lenof(press_envelope));
    } else {
        wavegen_set_vol_envelope(gen, release_envelope, lenof(release_envelope));
    }
    synth_dirty = true;
}

int main(void) {
    CHIP_Init();
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIOINT_Init();
    GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
    GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);
    GPIOINT_CallbackRegister(9, handle_button);
    GPIOINT_CallbackRegister(10, handle_button);
    GPIO_IntConfig(gpioPortB, 9, true, true, true);
    GPIO_IntConfig(gpioPortB, 10, true, true, true);
    SPIDRV_Init(&synth_spi, &synth_spi_init);

    while (1) {
        validate_synth();
        __WFI();
    }
}

void _exit(int status) {
    (void) status;
    while (1) {}
}
