#include <stdint.h>

#define F_SAMPLE (48ul*1000) /* 48kHz sample rate */

/* convert floating-point number in [-1, 1] to integer in [min, max] */
#define LVL(x, min, max) ((x) >= 1.0 ? (max) : (x) <= -1.0 ? (min) : (x) * (max))
#define LVLU32(x) (LVL(x, 0, UINT32_MAX))
#define LVLS32(x) (LVL(x, INT32_MIN, INT32_MAX))

#define ENVELOPE_STEP(delta_lvl, delta_time) {LVLS32(delta_lvl) / (delta_time), (delta_time)}

enum {
    WAVEGEN_SHAPE_NONE,
    WAVEGEN_SHAPE_SAWTOOTH,
    WAVEGEN_SHAPE_SQUARE,
    WAVEGEN_SHAPE_TRIANGLE,
    WAVEGEN_SHAPE_SINE,
};

enum {
    WAVEGEN_CMD_RESET_ENVELOPE = 1<<0,
};

enum {
    LOOPER_CMD_CLEAR = 1<<0,
    LOOPER_CMD_REWIND = 1<<1,
};

typedef volatile struct PACKED {
    int32_t rate;
    uint32_t time;
} EnvelopeStep;

typedef volatile struct PACKED {
    uint32_t freq;
    uint32_t vol;
    EnvelopeStep vol_envelope[8];
    uint8_t shape;
    uint8_t cmds;
} WaveGen;

typedef volatile struct PACKED {
    uint32_t delay;
    uint32_t feedback;
} Echo;

typedef volatile struct PACKED {
    int32_t balance;
} Pan;

typedef volatile struct PACKED {
    uint32_t playback_vol;
    int8_t playback_speed;
    uint8_t cmds;
} Looper;

typedef volatile struct PACKED {
    uint32_t delay[4];
} Reverb;

typedef volatile struct PACKED {
    WaveGen wavegens[16];
    Echo echo;
    Pan pan;
    Reverb reverb;
    Looper looper;
    uint32_t vol;
} Synth;

void wavegen_set_vol_envelope(WaveGen *self, EnvelopeStep *steps, int nsteps);
void wavegen_clearcmds(WaveGen *self);

void looper_clearcmds(Looper *self);

void synth_clearcmds(Synth *self);
