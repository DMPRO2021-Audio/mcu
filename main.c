#include <em_device.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include "segmentlcd.h"

#include "usart.h"

const uint8_t NOTE_OFF = 0x80;
const uint8_t NOTE_ON  = 0x90;
const uint8_t MSB_MASK = 0xF0;
const uint8_t LSB_MASK = 0x0F;
const uint8_t STATUS_MASK = 0x80;

static inline bool is_status_byte(uint8_t byte);
static inline bool is_data_byte(uint8_t byte);

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

    SegmentLCD_Write("HELLO");
    __enable_irq();

    volatile uint8_t byte;
    while(1) {
        byte = uart_next_byte();
        if (byte == 0) continue;

        if (is_status_byte(byte)) /* Status byte */ {
            char out[16];
            if ((byte & MSB_MASK) == NOTE_OFF){
                /* turn note off*/
                uint8_t key = uart_next_nonzero_byte();
                uint8_t vel = uart_next_byte();
                sprintf(out, "X %s %d", get_note(key).s, get_octave(key));


            } else if  ((byte & MSB_MASK) == NOTE_ON) {
                /* turn note on*/
                uint8_t key = uart_next_nonzero_byte();
                uint8_t vel = uart_next_byte();

                sprintf(out, "O %s %d", get_note(key).s, get_octave(key));
            }
            SegmentLCD_Write((const char *)out);
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

static Note get_note(uint8_t byte)
{
    return notes[(byte & LSB_MASK) % 12];
}

static int get_octave(uint8_t byte)
{
    return (byte - (byte % 12)) / 12;
}
