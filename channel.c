#include <stdbool.h>
#include <stdlib.h>

#include "util.h"
#include "synth.h"
#include "sound.h"
#include "channel.h"

typedef struct {
    List free_node;
    List channel_node;
    float velocity;
    char note;
    bool pressed;
} WavegenStatus;

extern Synth synth;

WavegenStatus wavegen_status[SYNTH_WAVEGEN_COUNT];
List free_wavegens_first, free_wavegens_last;

static WavegenStatus *alloc_wavegen(void) {
    List *node = free_wavegens_first.next;

    if (node == &free_wavegens_last) return NULL;
    list_delete(node);
    return container_of(node, WavegenStatus, free_node);
}

static void free_wavegen(WavegenStatus *self) {
    list_move_before(&self->free_node, &free_wavegens_last);
}

static Wavegen *get_wavegen_state(WavegenStatus *ws) {
    if (ws < wavegen_status || ws >= endof(wavegen_status)) exit(1);
    return &synth.wavegens[ws - wavegen_status];
}

static WavegenStatus *channel_get_note_wavegen(Channel *self, char note) {
    for (List *item = self->status_list.next; item; item = item->next) {
        WavegenStatus *ws = container_of(item, WavegenStatus, channel_node);

        if (ws->note == note) return ws;
    }
    return NULL;
}

static void channel_update_wavegen(Channel *self, WavegenStatus *ws) {
    Wavegen *w;

    if (!self->program) return;
    w = get_wavegen_state(ws);
    w->freq = freq_from_note(ws->note + self->pitch_bend);
    w->velocity = 10000ul * ws->velocity; // * self->gain;
    w->shape = self->program->shape;
    if (ws->pressed) {
        wavegen_set_vol_envelope(w, self->program->press_envelope);
    } else if (!self->sustain) {
        wavegen_set_vol_envelope(w, self->program->release_envelope);
    }
}

void channel_note_off(Channel *self, char note, float velocity) {
    WavegenStatus *ws = channel_get_note_wavegen(self, note);

    if (!ws) return;
    ws->pressed = false;
    channel_update_wavegen(self, ws);
    free_wavegen(ws);
}

void channel_note_on(Channel *self, char note, float velocity) {
    WavegenStatus *ws;

    if (velocity == 0) {
        channel_note_off(self, note, 127);
        return;
    }

    ws = channel_get_note_wavegen(self, note);
    if (!ws) {
        ws = alloc_wavegen();
        if (!ws) {
            warn(); /* no free wavegens */
            return;
        }
    }
    list_move_after(&ws->channel_node, &self->status_list);

    ws->note = note;
    ws->velocity = velocity;
    ws->pressed = true;

    channel_update_wavegen(self, ws);
    get_wavegen_state(ws)->cmds |= WAVEGEN_CMD_RESET_ENVELOPE;
}

void channel_update_wavegens(Channel *self) {
    for (List *item = self->status_list.next; item; item = item->next) {
        WavegenStatus *ws = container_of(item, WavegenStatus, channel_node);

        channel_update_wavegen(self, ws);
    }
}

void channel_all_notes_off(Channel *self, bool reset_envelopes) {
    for (List *item = self->status_list.next; item; item = item->next) {
        WavegenStatus *ws = container_of(item, WavegenStatus, channel_node);

        ws->pressed = false;
        channel_update_wavegen(self, ws);
        if (reset_envelopes) {
            get_wavegen_state(ws)->cmds |= WAVEGEN_CMD_RESET_ENVELOPE;
        }
    }
}

void reset_channels(void) {
    list_move_after(&free_wavegens_last, &free_wavegens_first);
    for (WavegenStatus *ws = wavegen_status; ws < endof(wavegen_status); ws++) {
        list_delete(&ws->channel_node);
        free_wavegen(ws);
    }
}
