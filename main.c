#include <em_device.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

unsigned int bytes[3]; 					//vet ikke om å putte data'en i en liste gir mening her.
int counter = 0;

int ch;
void UART_Init(void);
unsigned int UART_GetChar(void);
long double int_to_freq(int freq); 			//midi function for note
double int_to_amplitude(int min, int max, int var); 	//midi function for amplitude
							//TODO: midi function for status byte

int main(void) {
	UART_Init();
	BSP_LedsInit();
	__enable_irq();
	while(1) {
		while(counter < 3) {
			ch = UART_GetChar();
			if(ch != 0 && ch != '\r') {
				bytes[counter] = ch;
				counter++;
			}

		}
		counter = 0;
		//TODO here: process bytes with midi functions
	}
	return 0;
}

