#include "util.h"
#include "synth.h"

static void set_envelope(EnvelopeStep *dst, int dstlen, const EnvelopeStep *src) {
    /* copy src to dst */
    while (dstlen-- > 0 && src->duration) *dst++ = *src++;

    /* pad remainder of dst with {0, 0} */
    while (dstlen-- > 0) *dst++ = (EnvelopeStep){0, 0};
}

void wavegen_set_vol_envelope(Wavegen *self, const EnvelopeStep *steps) {
    set_envelope(self->envelope, lenof(self->envelope), steps);
}

void wavegen_clearcmds(Wavegen *self) {
    self->cmds = WAVEGEN_CMD_ENABLE;
}

void synth_clearcmds(Synth *self) {
    for (Wavegen *w = self->wavegens; w < endof(self->wavegens); w++) {
        wavegen_clearcmds(w);
    }
}
