#ifndef ARPEGGIATORH
#define ARPEGGIATORH

#include <stdint.h>
#include <stdbool.h>

#define ARP_KEYS_MAX 50
#define ARP_LOOP_MAX 150

#define START_BPM 120
#define START_NPB 4
#define START_GATE_TIME 0.5
#define START_NOTE_ORDER 0
#define START_NUM_OCTAVES 1
#define START_DYNAMIC_NPB_SWITCHING false

typedef volatile struct {
    // uint32_t held_key_freqs[ARP_KEYS_MAX];  // Defunct, replaced by held_key_notes
    char held_key_notes[ARP_KEYS_MAX];  // Holds the MIDI note values of all held keys
    char held_key_channels[ARP_KEYS_MAX];  // Holds the channel for the notes 
    uint8_t num_held_keys;
    char arp_loop[ARP_LOOP_MAX];
    uint8_t loop_length;
    uint8_t current_note_index;

    uint16_t BPM;
    uint8_t notes_per_beat;
    uint8_t playback_order;  // 0: ascending, 1: descending, 2: up-and-down, 3: key-press-order, 4: random (NOT IMPLEMENTED)
    uint8_t num_octaves;

    float gate_time;  // Number between 0 and 1 exclusive -- percentage of time between consecutive notes a note should be played for

    bool dynamic_NPB_switching;
} Arpeggiator;

void add_held_key(Arpeggiator *self, char freq, char channel);
void remove_held_key(Arpeggiator *self, char freq);

void set_BPM(Arpeggiator *self, uint16_t new_BPM);
void set_notes_per_beat(Arpeggiator *self, uint8_t new_notes_per_beat);
void set_playback_order(Arpeggiator *self, uint8_t playback_order);
void set_num_octaves(Arpeggiator *self, uint8_t num_octaves);

Arpeggiator init_arpeggiator(uint16_t init_BPM, uint8_t init_playback_order, uint8_t init_num_octaves, uint8_t init_notes_per_beat, float init_gate_time, bool init_dynamic_NPB_switching);
Arpeggiator setup_arpeggiator(void);
char play_current_note(Arpeggiator *self);
char current_channel(Arpeggiator *self);
#endif
