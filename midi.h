#define MIDI_COMMAND_SHIFT 4

enum {
    MIDI_COMMAND_MASK = 0x70,
    MIDI_CHANNEL_MASK = 0x0F,
    MIDI_STATUS_MASK  = 0x80,
};

enum {
    MIDI_NOTE_OFF,
    MIDI_NOTE_ON,
    MIDI_KEY_PRESSURE,
    MIDI_CONTROL_CHANGE,
    MIDI_PROGRAM_CHANGE,
    MIDI_CHANNEL_PRESSURE,
    MIDI_PITCH_BEND_CHANGE,
    MIDI_SYSTEM_MESSAGE,
};

enum {
    /* common messages */
    MIDI_CC_MODULATION_WHEEL = 1,
    MIDI_CC_BREATH_CONTROL   = 2,
    MIDI_CC_VOLUME           = 7,
    MIDI_CC_PAN              = 10,
    MIDI_CC_EXPRESSION       = 11,
    MIDI_CC_SUSTAIN_PEDAL    = 64,
    MIDI_CC_PORTAMENTO       = 65,
    MIDI_CC_RESONANCE        = 71,
    MIDI_CC_FREQUENCY_CUTOFF = 74,

    /* channel mode messages */
    MIDI_CC_ALL_SOUND_OFF = 120,
    MIDI_CC_RESET_ALL_CONTROLLERS,
    MIDI_CC_LOCAL_CONTROL,
    MIDI_CC_ALL_NOTES_OFF,
    MIDI_CC_OMNI_MODE_OFF,
    MIDI_CC_OMNI_MODE_ON,
    MIDI_CC_MONO_MODE_ON,
    MIDI_CC_MONO_MODE_OFF,

    /* custom messages */
    MIDI_CC_SUSTAIN_KEY = 14,
};

extern const uint32_t notes[128]; /* maps midi note to frequency */
