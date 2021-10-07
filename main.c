#include <em_device.h>
#include <stdint.h>
#include <bsp.h>
#include <unistd.h>
#include <stdio.h>
unsigned ch;
void UART_Init(void);
unsigned UART_GetChar(void);

void delay(uint32_t s) {
	volatile uint32_t counter;
	for(int i=0;i<s; i++) {
		counter = 100000;
		while(counter) counter--;
	}

}

int main(void) {

	UART_Init();
	BSP_LedsInit();
	__enable_irq();
	while(1) {
		delay(5);
		if((ch = UART_GetChar()) != 0) {
			if(ch != '\r') {
				BSP_LedSet(0);
				delay(2);
				BSP_LedClear(0);
			}
		}
	}
	return 0;
}

