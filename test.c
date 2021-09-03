#include <bspconfig.h>
#include <em_gpio.h>

struct {
	GPIO_Port_TypeDef port;
	unsigned int pin;
} leds[] = BSP_GPIO_LEDARRAY_INIT;

int main(void) {
	int a = 1;
}

void _exit(int status) {
	(void) status;
	while (1) {}
}
