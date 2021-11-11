#include <stdint.h>
#include <em_timer.h>

#define ARP_KEYS_MAX 50
#define ARP_LOOP_MAX 150

typedef volatile struct {
    uint32_t held_key_freqs[ARP_KEYS_MAX];
    uint8_t num_held_keys;
    uint32_t arp_loop[ARP_LOOP_MAX];
    uint8_t loop_length;
    uint8_t current_note_index;

    uint16_t BPM;
    uint8_t notes_per_beat;
    uint8_t playback_order;  // 0: ascending, 1: descending, 2: up-and-down, 3: key-press-order, 9: random
    uint8_t num_octaves;

    float gate_time;  // Number between 0 and 1 exclusive -- percentage of time between consecutive notes a note should be played for

    bool dynamic_NPB_switching;
} Arpeggiator;

void add_held_key(Arpeggiator *self, uint32_t freq);
void remove_held_key(Arpeggiator *self, uint32_t freq);

void set_BPM(Arpeggiator *self, uint16_t new_BPM);
void set_notes_per_beat(Arpeggiator *self, uint8_t new_notes_per_beat);
void change_playback_order(Arpeggiator *self, uint8_t playback_order);
void change_num_octaves(Arpeggiator *self, uint8_t num_octaves);

Arpeggiator init_arpeggiator(uint16_t init_BPM, uint8_t init_playback_order, uint8_t init_num_octaves, uint8_t init_notes_per_beat, float init_gate_time);

uint32_t play_current_note(Arpeggiator *self);