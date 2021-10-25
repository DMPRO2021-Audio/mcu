#include <em_device.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "segmentlcd.h"

#include "usart.h"

const uint8_t MSB_MASK    = 0xF0;
const uint8_t LSB_MASK    = 0x0F;
const uint8_t STATUS_MASK = 0x80;

typedef enum {
    NOTE_OFF   = 0x80,
    NOTE_ON    = 0x90,
} Command;

static inline bool is_status_byte(uint8_t byte);
static inline bool is_data_byte(uint8_t byte);
static inline bool is_command(uint8_t byte, Command c);

typedef struct { char s[2]; } Note;
const Note notes[12] = {
    {" C"}, {"CS"},
    {" D"}, {"DS"},
    {" E"},
    {" F"}, {"FS"},
    {" G"}, {"GS"},
    {" A"}, {"AS"},
    {" B"},
};
static Note get_note(uint8_t byte);

static int get_octave(uint8_t byte);

int main(void) {
    uart_init();
    SegmentLCD_Init(false);

    __enable_irq();

    volatile uint8_t byte;
    while(1) {
        byte = uart_next_valid_byte();

        if (is_status_byte(byte)) {
            if (is_command(byte, NOTE_OFF)) {
                /* turn note off*/
                char out[16];
                uint8_t key = uart_next_valid_byte();
                sprintf(out, "X %s %d", (get_note(key)).s, get_octave(key));
                SegmentLCD_Write((const char *)out);
                (void) uart_next_valid_byte();
            } else if (is_command(byte, NOTE_ON)) {
                /* turn note on*/
                char out[16];
                uint8_t key = uart_next_valid_byte();
                sprintf(out, "O %s %d", (get_note(key)).s, get_octave(key));
                SegmentLCD_Write((const char *)out);
                (void) uart_next_valid_byte();
            }
        }

    }
    return 0;
}

static inline bool is_status_byte(uint8_t byte)
{
    return (byte & STATUS_MASK);
}

static inline bool is_data(uint8_t byte)
{
    return !is_status_byte(byte);

}

static inline bool is_command(uint8_t byte, Command c)
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
