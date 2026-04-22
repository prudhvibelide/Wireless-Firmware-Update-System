/*
 * File : circular_buffer.h
 * Author : Prudhvi Raj Belide
 * Description : Header file for implementing and managing circular buffer functionality
 */

#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include "uart.h"


#define UART_BUFFER_SIZE 	100
#define INIT_VAL			0

typedef enum
{
	DEBUG_PORT = 0,
	SLAVE_DEV_PORT

}portType;

typedef struct
{
		unsigned char buffer[UART_BUFFER_SIZE];
		__IO uint32_t head;
		__IO uint32_t tail;

}circular_buffer;

void buffer_send_string(const char *s,portType uart);
void circular_buffer_init(void);

#endif



