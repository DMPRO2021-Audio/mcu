#include "em_device.h"
#include "em_chip.h"

/*********************************************************************
*
*       Defines for Cortex-M debug unit
*/
#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000)    // Stimulus Port Register word acces
#define ITM_STIM_U8  (*(volatile         char*)0xE0000000)    // Stimulus Port Register byte acces
#define ITM_ENA      (*(volatile unsigned int*)0xE0000E00)    // Trace Enable Ports Register
#define ITM_TCR      (*(volatile unsigned int*)0xE0000E80)    // Trace control register

/* Need to implement the  two Retarget IO functions with the read/write functions we want to use. */
/* Use ITM for printf over SWO. */
int RETARGET_WriteChar(char c){
  return ITM_SendChar (c);
}

int RETARGET_ReadChar(void){
  return 0;
}

void SWO_Setup(void)
{
    uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
    uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
    uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;
    uint32_t *tpiu_ffcr = (uint32_t *) 0xE0040304;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

    /* Enable Serial wire output pin */
    GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

#if defined(_EFM32_GECKO_FAMILY) || defined(_EFM32_TINY_FAMILY)
    /* Set location 1 */
    GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
    /* Enable output on pin */
    GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
    GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#elif defined(_EFM32_GIANT_FAMILY)
    /* Set location 0 */
    GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;
    /* Enable output on pin */
    GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
    GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
#error Unknown device family!
#endif

    /* Enable debug clock AUXHFRCO */
    CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

    while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

    /* Enable trace in core debug */
    CoreDebug->DHCSR |= 1;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* Enable PC and IRQ sampling output */
    *dwt_ctrl = 0x400113FF;

    /* Set TPIU prescaler to 16 (14 MHz / 16 = 875 kHz SWO speed) */
    *tpiu_prescaler = 0xf;

    /* Set protocol to NRZ */
    *tpiu_protocol = 2;
    *tpiu_ffcr = 0x100;

    /* Unlock ITM and output data */
    ITM->LAR = 0xC5ACCE55;
    ITM->TCR = 0x10009;

    /* ITM Channel 0 is used for UART output */
    ITM->TER |= (1UL << 0);

    /* ITM Channel 1 is used for a custom debug output in this example. */
    ITM->TER |= (1UL << 1);
}

/*********************************************************************
*
*       SWO_PrintChar()
*
* Function description
*   Checks if SWO is set up. If it is not, return,
*    to avoid program hangs if no debugger is connected.
*   If it is set up, print a character to the ITM_STIM register
*    in order to provide data for SWO.
* Parameters
*   c:    The Chacracter to be printed.
* Notes
*   Additional checks for device specific registers can be added.
*/
void SWO_PrintChar(char c) {
  //
  // Check if ITM_TCR.ITMENA is set
  //
  if ((ITM_TCR & 1) == 0) {
    return;
  }
  //
  // Check if stimulus port is enabled
  //
  if ((ITM_ENA & 1) == 0) {
    return;
  }
  //
  // Wait until STIMx is ready,
  // then send data
  //
  while ((ITM_STIM_U8 & 1) == 0);
  ITM_STIM_U8 = c;
}

/*********************************************************************
*
*       SWO_PrintString()
*
* Function description
*   Print a string via SWO.
*
*/
void SWO_PrintString(const char *s) {
  //
  // Print out character per character
  //
  while (*s) {
    SWO_PrintChar(*s++);
  }
}
