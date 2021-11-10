#include <stdint.h>

#define F_SAMPLE (48ul*1000) /* 48kHz sample rate */

/* convert floating-point number in [-1, 1] to integer in [min, max] */
#define LVL(x, min, max) ((x) >= 1.0 ? (max) : (x) <= -1.0 ? (min) : (x) * (max))
#define LVLU32(x) (LVL(x, 0, UINT32_MAX))
#define LVLS32(x) (LVL(x, INT32_MIN, INT32_MAX))

// #define ENVELOPE_STEP(delta_lvl, delta_time) {LVLS32(delta_lvl) / (delta_time), (delta_time)}

#define FIXED_POINT 8
// TODO: Might want to assert f < +/-2^23
/* Float to Fixed Point */
#define F2FP(f) (int32_t)(f * (1<<FIXED_POINT))

#define SYNTH_WAVEGEN_COUNT 6
#define WAVEGEN_ENVELOPE_LENGTH 8

enum {
    WAVEGEN_SHAPE_SAWTOOTH,
    WAVEGEN_SHAPE_SQUARE,
    WAVEGEN_SHAPE_TRIANGLE,
    WAVEGEN_SHAPE_SINE,
    WAVEGEN_SHAPE_PIANO,
};

enum {
    WAVEGEN_CMD_RESET_ENVELOPE = 1<<0,
};

enum {
    LOOPER_CMD_CLEAR = 1<<0,
    LOOPER_CMD_REWIND = 1<<1,
};

typedef volatile struct PACKED {
    uint8_t gain; /* Named gain. Note: Interpreted as fixed point in range [0, 2) */ // FIXME: Not very elegant
    uint8_t duration; /* Shifted by 8: 1 = 256 samples, 255 = 65536 samples. */
} EnvelopeStep;

typedef volatile struct PACKED {
    uint32_t freq /* FIXED POINT */;
    uint32_t velocity /* FIXED POINT */;
    EnvelopeStep envelopes[WAVEGEN_ENVELOPE_LENGTH];
    uint8_t shape;
    uint8_t cmds;
} WaveGen;

// ! Not used
// typedef volatile struct PACKED {
//     uint32_t delay;
//     uint32_t feedback;
// } Echo;

typedef volatile struct PACKED {
    int32_t balance; /* Interpreted as in range [-1, 1] */
} Pan;

typedef volatile struct PACKED {
    uint32_t tau[6];
    uint32_t gain[7] /* FIXED_POINT */;
} Reverb;

typedef volatile struct PACKED {
    WaveGen wavegens[SYNTH_WAVEGEN_COUNT];
    uint32_t master_volume; /* Named master_volume */
    Reverb reverb;
    Pan pan;
} Synth;

void wavegen_set_vol_envelope(WaveGen *self, EnvelopeStep *steps, int nsteps);
void wavegen_clearcmds(WaveGen *self);
void synth_clearcmds(Synth *self);
