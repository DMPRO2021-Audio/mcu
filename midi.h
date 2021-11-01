enum {
    MIDI_COMMAND_MASK = 0xF0,
    MIDI_CHANNEL_MASK = 0x0F,
    MIDI_STATUS_MASK  = 0x80,
};

enum {
    MIDI_NOTE_OFF              = 0x80,
    MIDI_NOTE_ON               = 0x90,
    MIDI_AFTERTOUCH            = 0xA0,
    MIDI_CONTINUOUS_CONTROLLER = 0xB0,
    MIDI_PATCH_CHANGE          = 0xC0,
    MIDI_CHANNEL_PRESSURE      = 0xD0,
    MIDI_PITCH_BEND            = 0xE0,
    MIDI_NON_MUSICAL_COMMAND   = 0xF0,
};

extern const uint32_t notes[128]; /* maps midi note to frequency */
