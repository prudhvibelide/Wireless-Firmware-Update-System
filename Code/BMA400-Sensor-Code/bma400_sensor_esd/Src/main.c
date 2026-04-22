/*
 * File : main.c
 * Author : Sriramkumar Jayaraman
 * Description : This file is the mian file for Accelorometer functionality with initialisations
 */

#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "uart.h"
#include "circular_buffer.h"
#include "bma.h"


#define  GPIOAEN		(1U<<0)
#define  PIN5			(1U<<5)
#define  LED_PIN		PIN5


#define VECT_TAB_BASE_ADDRESS			FLASH_BASE   //0x08000000
#define VECT_TAB_OFFSET					0x8000
int main()
{


	char msg[50];
	    int16_t x_acc, y_acc, z_acc;

	    // Initialize system clocks (your existing code)
	    RCC->CR |= RCC_CR_HSION;
	    while(!(RCC->CR & RCC_CR_HSIRDY));

	    // Configure PLL for 84MHz (your existing code)
	    RCC->PLLCFGR = (4 << RCC_PLLCFGR_PLLM_Pos) |
	                   (84 << RCC_PLLCFGR_PLLN_Pos) |
	                   (0 << RCC_PLLCFGR_PLLP_Pos);

	    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;
	    RCC->CR |= RCC_CR_PLLON;
	    while(!(RCC->CR & RCC_CR_PLLRDY));


	/*Enable FPU*/
	fpu_enable();

	/*Initialize debug UART*/
    debug_uart_init();
    circular_buffer_init();
    I2C1_Init();


    printf("\r\nInitializing BMA400...\r\n");

        // Initialize BMA400
        if (BMA400_Init()) {
            printf("BMA400 initialized successfully!\r\n");
        } else {
            printf("BMA400 initialization failed!\r\n");
            while(1);
        }

       while(1) {

    	   //Read X-axis acceleration
            x_acc = BMA400_ReadXAxis();
            sprintf(msg, "X-axis: %d \r\n", x_acc);
            printf(msg);

          //Read Y-axis acceleration
            y_acc = BMA400_ReadYAxis();
            sprintf(msg, "Y-axis: %d \r\n", y_acc);
            printf(msg);

         //Read Z-axis acceleration
           z_acc = BMA400_ReadZAxis();
           sprintf(msg, "Z-axis: %d \r\n", z_acc);
           printf(msg);

         //Delay between readings
            for(volatile uint32_t i = 0; i < 1000000; i++);
        }
}

void SystemInit(void)
{
	SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
}
