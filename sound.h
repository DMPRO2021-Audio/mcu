#include <stdint.h>

#define CONCERT_PITCH 440
#define CONCERT_PITCH_NOTE 69
#define OCTAVE 12

typedef struct {
    uint8_t shape;
    const EnvelopeStep *press_envelope;
    const EnvelopeStep *release_envelope;
} Program;

extern const Program programs[];
extern const int num_programs;

uint32_t freq_from_note(float note);
