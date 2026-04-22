/*
 * File : timebase.h
 * Author : Prudhvi Raj Belide
 * Description : Header file for system timebase initialization and delay functionality, typically using a timer or SysTick.
 */

#ifndef __TIMEBASE_H__
#define __TIMEBASE_H__

#include <stdint.h>


#define MAX_DELAY		 0xFFFFFFFF

uint32_t get_tick(void);
void systick_delay_ms(uint32_t delay);
void timebase_init(void);

#endif
