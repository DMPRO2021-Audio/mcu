#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const Program *program;
    float gain;
    float pitch_bend;
    bool sustain;

    /* private */
    List status_list;
} Channel;

void channel_note_off(Channel *self, char note, float velocity);
void channel_note_on(Channel *self, char note, float velocity);
void channel_update_wavegens(Channel *self);
void channel_all_notes_off(Channel *self, bool reset_envelopes);

void reset_channels(void);
