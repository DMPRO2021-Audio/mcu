#include <math.h>

//Note (2. byte)
long double int_to_freq(int freq) {
    return 0.0;
	// return 440*pow(2, (freq-69)/12);
}

//Velocity (3. byte)
double int_to_amplitude(int min, int max, int var) {
    return 0.0;
	// return min + ((var-0)/(127-0))*(max-min)
}

//TODO:Status (1. byte) (https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes)




