#include "util.h"
#include "synth.h"

static void set_envelope(EnvelopeStep *dst, int dstlen, EnvelopeStep *src, int srclen) {
    if (srclen > dstlen) {
        warn();
        srclen = dstlen;
    }

    /* copy src to dst */
    for (EnvelopeStep *end = src + srclen; src < end; src++, dst++) *dst = *src;

    /* pad remaining dst steps with {0, 0} */
    for (EnvelopeStep *end = dst + (dstlen - srclen); dst < end; dst++) *dst = (EnvelopeStep){0, 0};
}

void wavegen_set_vol_envelope(WaveGen *self, EnvelopeStep *steps, int nsteps) {
    set_envelope(self->envelopes, lenof(self->envelopes), steps, nsteps);
}

void wavegen_clearcmds(WaveGen *self) {
    self->cmds = WAVEGEN_CMD_ENABLE;
}

void synth_clearcmds(Synth *self) {
    for (WaveGen *w = self->wavegens; w < endof(self->wavegens); w++) {
        wavegen_clearcmds(w);
    }
}
