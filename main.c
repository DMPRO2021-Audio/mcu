#include <em_device.h>
#include <em_dbg.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "segmentlcd.h"

#include "swo.h"

#include "usart.h"

const uint8_t MSB_MASK    = 0xF0;
const uint8_t LSB_MASK    = 0x0F;
const uint8_t STATUS_MASK = 0x80;

typedef enum {
    NOTE_OFF              = 0x80,
    NOTE_ON               = 0x90,
    AFTERTOUCH            = 0xA0,
    CONTINUOUS_CONTROLLER = 0xB0,
    PATCH_CHANGE          = 0xC0,
    CHANNEL_PRESSURE      = 0xD0,
    PITCH_BEND            = 0xE0,
    NON_MUSICAL_COMMAND   = 0xF0,
} Command;

static inline bool is_status_byte(uint8_t byte);
static inline bool is_data_byte(uint8_t byte);
static inline bool is_command(uint8_t byte, Command c);

typedef struct { const char s[3]; } Note;
/* "*S" should be "*#", but the LCD can't display '#' */
const Note notes[12] = {
    {"C "}, {"C#"},
    {"D "}, {"D#"},
    {"E "},
    {"F "}, {"F#"},
    {"G "}, {"G#"},
    {"A "}, {"A#"},
    {"B "},
};

static Note get_note(uint8_t byte);
static int get_octave(uint8_t byte);
int main(void) {
    uart_init();
    SegmentLCD_Init(false);
    SWO_Setup();
    // while (!DBG_Connected())
    //     ;
    // DBG_SWOEnable(1);

    __enable_irq();

    char out[256];

    volatile uint8_t byte;
    while(1) {
        byte = uart_next_valid_byte();
        for (int i = 0; i < 8; i++)  SegmentLCD_ARing(i, 0);

        if (is_status_byte(byte)) {
            uint8_t cmd = byte & MSB_MASK;
            switch (cmd) {
            case NOTE_ON:
                {
                    uint8_t key      = uart_next_valid_byte();
                    uint8_t velocity = uart_next_valid_byte();

                    uint8_t octave = get_octave(key);

                    sprintf(out, "NOTE ON:  %s (octave: %d, vel:%3d)\n", get_note(key).s, octave, velocity);

                    /* LCD print*/
                    {
                        SegmentLCD_Number(velocity);
                        SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
                        for (int i = 0; i < octave; i++)
                            SegmentLCD_ARing(i, 1);
                        SegmentLCD_Write(get_note(key).s);
                    }
                } break;
            case NOTE_OFF:
                {
                    uint8_t key      = uart_next_valid_byte();
                    uint8_t velocity = uart_next_valid_byte();

                    uint8_t octave = get_octave(key);

                    sprintf(out, "NOTE OFF: %s (octave: %d, vel:%3d)\n", get_note(key).s, octave, velocity);

                    /* LCD print*/
                    {
                        SegmentLCD_Number(velocity);
                        SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 0);
                        for (int i = 0; i < octave; i++)
                            SegmentLCD_ARing(i, 1);
                        SegmentLCD_Write(get_note(key).s);
                    }
                } break;
            case AFTERTOUCH:
                {
                    uint8_t key   = uart_next_valid_byte();
                    uint8_t touch = uart_next_valid_byte();

                    sprintf(out, "AFTERTOUCH: key:%d touch:%d\n", key, touch);

                    /* LCD print*/
                    {
                        char out[16];
                        sprintf(out, "AT %2s", get_note(key).s);
                        SegmentLCD_Write(out);
                        SegmentLCD_Number(touch);
                    }

                } break;
            case CONTINUOUS_CONTROLLER:
                {
                    uint8_t controller_num   = uart_next_valid_byte();
                    uint8_t controller_value = uart_next_valid_byte();

                    sprintf(out, "CONTINUOUS CONTROLLER: num:%d val:%d\n", controller_num, controller_value);

                    /* LCD print*/
                    {
                        SegmentLCD_Write("CONTCTRL");
                        SegmentLCD_Number(controller_value);
                        for (int i = 0; i < controller_num; i++)
                            SegmentLCD_ARing(i, 1);
                    }
                } break;
            case PATCH_CHANGE:
                {
                    uint8_t instrument_num =  uart_next_valid_byte();

                    sprintf(out, "PATCH CHANGE: instrument:%d\n", instrument_num);

                    /* LCD print*/
                    {
                        SegmentLCD_Write("PATCH");
                        SegmentLCD_Number(instrument_num);
                    }
                } break;
            case CHANNEL_PRESSURE:
                {
                    uint8_t pressure = uart_next_valid_byte();

                    sprintf(out, "CHANNEL PRESSURE: pressure:%d\n", pressure);
                    /* LCD print*/
                    {
                        SegmentLCD_Write("CHPRESS");
                        SegmentLCD_Number(pressure);
                    }
                } break;
            case PITCH_BEND:
                {
                    uint8_t lsb = uart_next_valid_byte();
                    uint8_t msb = uart_next_valid_byte();
                    uint16_t val = ((uint16_t)msb << 6) | (uint16_t)lsb;

                    sprintf(out, "PITCH BEND: val:%d\n", val);

                    /* LCD print*/
                    {
                        SegmentLCD_Write("PITCHBN");
                        SegmentLCD_Number(val);
                    }
                } break;
            case NON_MUSICAL_COMMAND:
            default:
                break;
            }
            SWO_PrintString(out);
        }
    }
    return 0;
}

static bool is_status_byte(uint8_t byte)
{
    return (byte & STATUS_MASK);
}

static bool is_data_byte(uint8_t byte)
{
    return !is_status_byte(byte);

}

static bool is_command(uint8_t byte, Command c)
{
    return (byte & MSB_MASK) == c;
}

static Note get_note(uint8_t byte)
{
    return notes[(byte % 12)];
}

static int get_octave(uint8_t byte)
{
    return ((byte - (byte%12)) / 12);
}
