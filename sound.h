#include <stdint.h>

#define CONCERT_PITCH 440
#define CONCERT_PITCH_NOTE 69
#define OCTAVE 12

#define REVERB_PRESET_LEN 4
#define REVERB_PRESET_OFF 0
#define REVERB_PRESET_SMALL_ROOM 1
#define REVERB_PRESET_LARGE_ROOM 2
#define REVERB_PRESET_HALL 3

int currrent_reverb_preset;
extern const Reverb reverb_presets[REVERB_PRESET_LEN];

typedef struct {
    uint8_t shape;
    const EnvelopeStep *press_envelope;
    const EnvelopeStep *release_envelope;
    uint8_t reverb;
} Program;

extern const Program programs[];
extern const int num_programs;

uint32_t freq_from_note(float note);
