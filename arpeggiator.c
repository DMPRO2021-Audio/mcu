#include <stdint.h>
// #include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <stdbool.h>
// #include <em_device.h>
// #include <em_chip.h>
// #include <em_cmu.h>
// #include <em_emu.h>
// #include <em_gpio.h>
// #include <em_timer.h>
// #include <gpiointerrupt.h>

#include "arpeggiator.h"
#include "util.h"


bool is_ascending = true;

/* NOTE: Defunct, as we are now using MIDI note values rather than frequencies.
   Due to difficulties linking libm, we make our own exponentiation function.
   This is needed for calculating the frequencies for notes in an arbitrary number of octaves. */
uint8_t power(uint8_t base, uint8_t exponent) {
	uint8_t out_value = 1;
	for (uint8_t i = 0; i < exponent; i++)
	{
		out_value *= base;
	}

	return out_value;
}

/* Shifts current_note_index forwards if notes which warrant such a shift are added to the loop.
   For instance: When the arpeggiator is set to ascending, and a note which has a lower MIDI note value
   than the current note is added, current_note_index should be shifted forwards in order to point
   to the same note that it did prior to adding the new note.
   
   After calling this method, the update_loop() method must also be called before
   arpeggiator is ready for playback. */
void add_to_loop(Arpeggiator *self, char note) {
	char current_note = self->arp_loop[self->current_note_index];

	switch (self->playback_order)
	{
	// Ascending
	case 0:
		if (note <= current_note && current_note != 0) {
			self->current_note_index++;
		}
		break;
	
	// Descending
	case 1:
		if (note >= current_note && current_note != 0) {
			self->current_note_index++;
		}
		break;
	
	// Up and down
	case 2:
		if (is_ascending && note <= current_note && current_note != 0) {
			self->current_note_index++;
		}
		else if (!is_ascending && note >= current_note && current_note != 0) {
			self->current_note_index++;
		}
		else {
			break;
		}
	
	// Key press order
	case 3:
		// Do nothing
		break;
	
	// Random
	case 9:
		// Do nothing
		break;

	default:
		exit(1);  // Potential room for error handling
	}
}

/* The converse of add_to_loop().
   
   After calling this method, the update_loop() method must also be called before
   arpeggiator is ready for playback. */
void remove_from_loop(Arpeggiator *self, char note) {
	char current_note = self->arp_loop[self->current_note_index];

	switch (self->playback_order)
	{
	// Ascending
	case 0:
		if (note < current_note) {
			self->current_note_index--;
		}
		break;
	
	// Descending
	case 1:
		if (note > current_note) {
			self->current_note_index--;
		}
		break;
	
	// Up and down
	case 2:
		if (is_ascending) {
			if (note < current_note) {
				self->current_note_index--;
			}
		}
		else {
			if (note > current_note) {
				self->current_note_index--;
			}
		}
		break;
	
	case 3:
		// Do nothing
		break;
	
	// Random
	case 9:
		// Do nothing
		break;

	default:
		exit(1);  // Potential room for error handling
	}
}

/* Comparison functions for use in library function qsort(). */
int cmpfunc_smaller (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
int cmpfunc_larger (const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

/* Recreates the entire loop, based on held keys, number of octaves and note order setting.
   Currently the only function which actually adds notes to the loop. */
void update_loop(Arpeggiator *self) {
	// Zero the entire arp_loop
	for (uint16_t i = 0; i < lenof(self->arp_loop); i++)
	{
		self->arp_loop[i] = 0;
	}

	uint8_t num_notes = self->num_held_keys * self->num_octaves;

	// Create an array of _all_ of the MIDI note values to be added to the loop, including potential octaves
	char arp_notes[num_notes];
	uint8_t loop_counter = 0;
	for (uint8_t i = 0; i < self->num_held_keys; i++)
	{
		for (uint8_t j = 0; j < self->num_octaves; j++)
		{
			arp_notes[loop_counter] = self->held_key_notes[i] + j*12;
			loop_counter++;
		}
	}

	// Sort the array according to playback_order setting
	switch (self->playback_order)
	{
	// Ascending
	case 0:
		self->loop_length = num_notes;

		qsort(arp_notes, num_notes, sizeof(char), cmpfunc_smaller);

		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[i] = arp_notes[i];
		}
		break;
	
	// Descending
	case 1:
		self->loop_length = num_notes;

		qsort(arp_notes, num_notes, sizeof(char), cmpfunc_larger);

		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[i] = arp_notes[i];
		}
		break;
	
	// Up and down
	case 2:
		self->loop_length = num_notes * 2;

		qsort(arp_notes, num_notes, sizeof(char), cmpfunc_smaller);

		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[i] = arp_notes[i];
		}
		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[num_notes+i] = arp_notes[num_notes-1-i];
		}
		break;
	// Key press order
	case 3:
		self->loop_length = num_notes;
		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[i] = arp_notes[i];
		}

		break;
	
	// Random
	case 9:
		self->loop_length = num_notes;
		for (uint8_t i = 0; i < num_notes; i++)
		{
			self->arp_loop[i] = arp_notes[i];
		}
		
		break;

	default:
		exit(1);  // Potential room for error handling
	}

	if (self->dynamic_NPB_switching == true) {
		self->notes_per_beat = self->loop_length;
	}
}

/* Adds a new key to the array and updates the arpeggiator loop, preserving position in the loop. */
void add_held_key(Arpeggiator *self, char note) {
	for (uint8_t i = 0; i < lenof(self->held_key_notes); i++)
	{
		if (self->held_key_notes[i]==0) {
			self->held_key_notes[i] = note;
			self->num_held_keys++;
			break;
		}
	}

	for (uint8_t i = 0; i < self->num_octaves; i++)
	{
		add_to_loop(self, note + i*12);
	}

	update_loop(self);
}

/* The converse of add_held_key(). */
void remove_held_key(Arpeggiator *self, char note) {
	bool shift_remainder = false;
	for (uint8_t i = 0; i < lenof(self->held_key_notes); i++)
	{
		if (self->held_key_notes[i] == note) {
			shift_remainder = true;
			self->num_held_keys--;
		}
		if (shift_remainder) {
			// Move all remaining elements one step up
			if (i != lenof(self->held_key_notes)-1) {
				self->held_key_notes[i] = self->held_key_notes[i+1];
			}
			// Set the final element to 0
			else {
				self->held_key_notes[i] = 0;
			}
		}
	}

	for (uint8_t i = 0; i < self->num_octaves; i++)
	{
		remove_from_loop(self, note + i*12);
	}

	update_loop(self);
}

/* Completely resets the loop, based on arpeggiator settings and held_key_notes.
   Should be run after changing playback_order or num_octaves. */
void init_loop(Arpeggiator *self) {
	self->current_note_index = 0;
	is_ascending = true;
	update_loop(self);
}

/* BPM setter function */
void set_BPM(Arpeggiator *self, uint16_t new_BPM) {
	if (new_BPM >= 1) {
		self->BPM = new_BPM;
	}
	else {
		self->BPM = new_BPM;
	}
}

/* notes_per_beat setter function */
void set_notes_per_beat(Arpeggiator *self, uint8_t new_notes_per_beat) {
	if (new_notes_per_beat >= 1) {
		self->notes_per_beat = new_notes_per_beat;
	}
	else {
		self->notes_per_beat = 1;
	}
}

/* Set a new playback order setting for the loop.
   0 = ascending
   1 = descending
   2 = up-and-down
   3 = key-press-order
   9 = random */
void change_playback_order(Arpeggiator *self, uint8_t playback_order) {
	self->playback_order = playback_order;
	init_loop(self);
}

/* Set how many octaves above the held keys should also be added to the loop.
   If num_octaves == 1, only the held keys themselves are included in the loop.*/
void change_num_octaves(Arpeggiator *self, uint8_t num_octaves) {
	self->num_octaves = num_octaves;
	init_loop(self);
}

/* Generetes a new arpeggiator with default settings. */
Arpeggiator init_arpeggiator(uint16_t init_BPM, uint8_t init_playback_order, uint8_t init_num_octaves, uint8_t init_notes_per_beat, float init_gate_time) {
	Arpeggiator new_arpeggiator = {
		.num_held_keys = 0,
		.loop_length = 0,
		.current_note_index = 0,
		.notes_per_beat = init_notes_per_beat,

		.BPM = init_BPM,
		.playback_order = init_playback_order,
		.num_octaves = init_num_octaves,
		.gate_time = init_gate_time
	};

	return new_arpeggiator;
}

/* Returns the MIDI note value of the note that should be played, and shifts the
   index forwards to point at the next note in the loop. */
char play_current_note(Arpeggiator *self) {
	// if (self->playback_order == 9) {
	// 	self->current_note_index = (uint8_t) rand() % self->loop_length;
	// }

	char current_note = self->arp_loop[self->current_note_index];

	self->current_note_index = (self->current_note_index+1) % self->loop_length;  // Increment index by one, or loop around if finished

	// Special case for note_order == 2 (up-and-down).
	// Keeps track of whether we are currently ascending or descending through the notes.
	// Necessary for dynamically and seamlessly adding held keys to the loop.
	if (self->playback_order == 2 && is_ascending && self->current_note_index >= (self->loop_length) / 2) {
		is_ascending = false;
	}
	else if (self->playback_order == 2 && !is_ascending && self->current_note_index < (self->loop_length) / 2) {
		is_ascending = true;
	}

	return current_note;
}
