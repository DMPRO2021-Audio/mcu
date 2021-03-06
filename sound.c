#include <math.h>

#include "util.h"
#include "synth.h"
#include "sound.h"

static const EnvelopeStep piano_press_envelope[] = {
    { .rate = 1400, .duration = 50, },
    { .rate = -1400, .duration = 25, },
    { .rate = -40, .duration = 255, },
    { .rate = -19,  .duration = 255, },
    { .rate = -10,  .duration = 255, },
    { .rate = -6,  .duration = 255, },
    { .rate = -3,  .duration = 255, },
    { .rate = -1,  .duration = 255, },
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
    { .rate = -6500,   .duration = 100, },
    {0},
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
    { .rate = 10000, .duration = 32, },
    {0}
};

static const EnvelopeStep quick_release_envelope[] = {
    { .rate = -10000, .duration = 32, },
    {0}
};

const Reverb reverb_presets[REVERB_PRESET_LEN] = {
    { /* Off */
        .tau = {1024, 1024, 1024, 1024, 1024, 1024},
        .gain = {
            F2FP(0.0),
            F2FP(0.0),
            F2FP(0.0),
            F2FP(0.0),
            F2FP(0.0),
            F2FP(0.0),
            F2FP(0.0)
        }
    },
    { /* Small room, 1 sek reverb time */
        .tau = {782 , 931, 1056, 1181, 241, 73},
        .gain = {
            F2FP(0.893),
            F2FP(0.874),
            F2FP(0.858),
            F2FP(0.844),
            F2FP(0.7),
            F2FP(0.7),
            F2FP(0.5)
        }
    },
    { /* Large room, 2 sek reverb time */
        .tau = {1441 , 1683, 1921, 2159, 241, 83},
        .gain = {
            F2FP(0.899),
            F2FP(0.886),
            F2FP(0.871),
            F2FP(0.856),
            F2FP(0.7),
            F2FP(0.7),
            F2FP(0.6)
        }
    },
    { /* Hall, 4 sek reverb time */
        .tau = {3003, 3403, 3905, 4495, 241, 83},
        .gain = {
            F2FP(0.895),
            F2FP(0.883),
            F2FP(0.867),
            F2FP(0.853),
            F2FP(0.7),
            F2FP(0.7),
            F2FP(0.7)
        }
    },
};


const Program programs[] = {
    {WAVEGEN_SHAPE_PIANO, piano_press_envelope, release_envelope},          // 0. Piano
    {WAVEGEN_SHAPE_PIANO, organ_press_envelope, release_envelope},          // 1. Organ
    {WAVEGEN_SHAPE_GUITAR, piano_press_envelope, release_envelope},         // 2. Guitar
    {WAVEGEN_SHAPE_FLUTE, organ_press_envelope, release_envelope},          // 3. FLute??
    {WAVEGEN_SHAPE_SAWTOOTH, quick_press_envelope, quick_release_envelope}, // 4. Straight sawtooth
    {WAVEGEN_SHAPE_SINE, quick_press_envelope, quick_release_envelope},     // 5. Straight sine
    {WAVEGEN_SHAPE_SQUARE, quick_press_envelope, quick_release_envelope},   // 6. Straight square
    {WAVEGEN_SHAPE_SAWTOOTH, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SQUARE, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SINE, piano_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, organ_press_envelope, release_envelope},   // Saw
    {WAVEGEN_SHAPE_SQUARE, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SINE, organ_press_envelope, release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, tremolo_press_envelope, tremolo_release_envelope},
    {WAVEGEN_SHAPE_SAWTOOTH, quick_press_envelope, quick_release_envelope},
};

const int num_programs = lenof(programs);

uint32_t freq_from_note(float note) {
    return (uint32_t)(CONCERT_PITCH * powf(2, (note - CONCERT_PITCH_NOTE) / OCTAVE)) << 8;
}
