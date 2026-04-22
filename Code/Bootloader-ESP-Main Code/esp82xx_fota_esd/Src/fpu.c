/*
 * File : fpu.c
 * Author : Prudhvi Raj Belide
 * Description : This file enables the Floating Point Unit (FPU) on an STM32F4 microcontroller by setting the appropriate bits
 * in the CPACR register of the SCB (System Control Block)
 */


#include "stm32f4xx.h"



void fpu_enable(void)
{
	/*Enable floating point unit:  Enable CP10 and CP11 full access*/
	SCB->CPACR |=(1<<20);
	SCB->CPACR |=(1<<21);
	SCB->CPACR |=(1<<22);
	SCB->CPACR |=(1<<23);

}
