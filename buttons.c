#include <stdint.h>
#include <stdbool.h>
#include <em_device.h>
#include <em_gpio.h>


#define GPIO_PE_DIN	((volatile uint32_t*) (0x40006090 + 0x1c))  //GPIO_PE_BASE + GPIO_DIN


void button_init(void) {
	GPIO_PinModeSet(gpioPortF, 10, gpioModeInput, 1);	//Button1 (keep in mind that this might not be the same as on the board)
	GPIO_PinModeSet(gpioPortE, 4, gpioModeInput, 1);	//Button2
	GPIO_PinModeSet(gpioPortE, 5, gpioModeInput, 1);	//Button3
	GPIO_PinModeSet(gpioPortE, 6, gpioModeInput, 1);	//Button4
	GPIO_PinModeSet(gpioPortE, 7, gpioModeInput, 1);	//Button5

	//Configure interrupts to trigger on falling edge (button pressed)
	/*Try GPIO_ExtIntConfig(GPIO_Port_TypeDef port, 
				          unsigned pin, 
			                unsigned intNo, 
				       bool risingEdge, 
				      bool fallingEdge, 
					   bool enable) 
	if this doesn't work (it should though)*/ 

	GPIO_IntConfig(gpioPortF, 10, false, true, true);  // GPIO_IntConfig(GPIO_Port_TypeDef port, unsigned pin, bool risingEdge, bool fallingEdge, bool enable)
	GPIO_IntConfig(gpioPortE, 4, false, true, true);
	GPIO_IntConfig(gpioPortE, 5, false, true, true);
	GPIO_IntConfig(gpioPortE, 6, false, true, true);
	GPIO_IntConfig(gpioPortE, 7, false, true, true);

	//Enable interrupts
	 NVIC_SetPriority(GPIO_EVEN_IRQn, 3);
    	 NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
    	 NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    	 NVIC_SetPriority(GPIO_ODD_IRQn, 3);
    	 NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    	 NVIC_EnableIRQ(GPIO_ODD_IRQn);

}

int check_button(int butNo) {

	return (~(*GPIO_PE_DIN) & 1 << (butNo - 1)); 
}

void GPIO_handler(void) {
	if (check_button(4)) {
		/*do stuff*/
		GPIO_PinOutSet(gpioPortA, 0);
		GPIO_IntClear(1 << 4);
	}

	if (check_button(5)) {
		/*do stuff*/
		GPIO_PinOutSet(gpioPortA, 0);
		GPIO_IntClear(1 << 5);
	}

	if (check_button(6)) {
		/*do stuff*/
		GPIO_PinOutSet(gpioPortA, 0);
		GPIO_IntClear(1 << 6);

	}

	if (check_button(7)) {
		/*do stuff*/
		GPIO_PinOutSet(gpioPortA, 0);
		GPIO_IntClear(1 << 7);
	}
	//Last button (PF10) missing for now
}


void GPIO_EVEN_IRQHandler(void) {

	GPIO_handler();
}

void GPIO_ODD_IRQHandler(void) {

	GPIO_handler();

}
