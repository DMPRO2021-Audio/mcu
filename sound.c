#include <math.h>

#include "util.h"
#include "synth.h"
#include "sound.h"

static const EnvelopeStep piano_press_envelope[] = {
    { .rate = 260, .duration = 255, },
    { .rate = -60, .duration = 100, },
    { .rate = -20, .duration = 255, },
    { .rate = -14,  .duration = 255, },
    { .rate = -10,  .duration = 255, },
    { .rate = -8,  .duration = 255, },
    { .rate = -7,  .duration = 255, },
    { .rate = -6,  .duration = 255, },
};

static const EnvelopeStep organ_press_envelope[] = {
    { .rate = 260, .duration = 255, },
    { .rate = -60, .duration = 100, },
    { .rate = 0,   .duration = 255, },
    { .rate = 0,   .duration = 255, },
    { .rate = 0,   .duration = 255, },
    { .rate = 0,   .duration = 255, },
    { .rate = 0,   .duration = 255, },
    { .rate = 0,   .duration = 255, },
};

static const EnvelopeStep tremolo_press_envelope[] = {
    { .rate = 260, .duration = 255, },
    { .rate = -60, .duration = 100, },
    { .rate = -100, .duration = 50, },
    { .rate = 100,  .duration = 50, },
    { .rate = -100, .duration = 50, },
    { .rate = 100,  .duration = 50, },
    { .rate = -100, .duration = 50, },
    { .rate = 0,  .duration = 255, },
};

static const EnvelopeStep release_envelope[] = {
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
    { .rate = -60,   .duration = 255, },
};

static const EnvelopeStep tremolo_release_envelope[] = {
    { .rate = 100,  .duration = 50, },
    { .rate = -200, .duration = 50, },
    { .rate = 100,  .duration = 50, },
    { .rate = -200, .duration = 50, },
    { .rate = 100,  .duration = 50, },
    { .rate = -200, .duration = 50, },
    { .rate = 100,  .duration = 50, },
    { .rate = -200, .duration = 50, },
};

static const EnvelopeStep quick_press_envelope[] = {
    { .rate = 10000, .duration = 255, },
    {0}
};

static const EnvelopeStep quick_release_envelope[] = {
    { .rate = -10000, .duration = 255, },
    {0}
};

const Program programs[] = {
    {WAVEGEN_SHAPE_PIANO, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SQUARE, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SINE, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_PIANO, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SQUARE, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SINE, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, tremolo_press_envelope, tremolo_release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, quick_press_envelope, quick_release_envelope},
};

const int num_programs = lenof(programs);

uint32_t freq_from_note(float note) {
    return (uint32_t)(CONCERT_PITCH * powf(2, (note - CONCERT_PITCH_NOTE) / OCTAVE)) << 8;
}
