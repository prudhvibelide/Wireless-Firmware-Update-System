/*
 * File : main.c
 * Author : Prudhvi Raj Belide
 * Description : This file initializes peripherals, checks for button press to trigger FOTA (Firmware Over-the-Air) update using
 * ESP8266 and jumps to the updated firmware or the last valid firmware.
 */


#include <stdio.h>
#include "stm32f4xx.h"
#include "fpu.h"
#include "bsp.h"
#include "adc.h"
#include "circular_buffer.h"
#include "fota_processor.h"

#define SSID_NAME  "Arsive"
#define PASSKEY    "hps@e206"


char version_buff[10] = {0};
char temp_ver_buffer[25] = {0};


int main()
{
	/*Enable FPU*/
	fpu_enable();

	/*Initialize debug UART*/
	debug_uart_init();
	esp_uart_init();

	/*Initialize timebase*/
	timebase_init();

	/*Initialize LED*/
	led_init();

	/*Initialize Push button*/
	button_init();


	/*Initialize circular buffer*/
	circular_buffer_init();


	if(get_btn_state()){


#ifdef DEBUG_OUTPUT
		buffer_send_string("************************************************\n\r",debug_port);
		buffer_send_string("............Initiating FOTA System.........\n\r",debug_port);

#endif

		esp8266_init(SSID_NAME,PASSKEY);

#ifdef DEBUG_OUTPUT
		buffer_send_string("STAGE: Getting firmware version\n\r",debug_port);

#endif

		esp82xx_get_version_file(version_buff);
		sprintf(temp_ver_buffer,"Version:%s\r\n", version_buff);

#ifdef DEBUG_OUTPUT
		buffer_send_string(temp_ver_buffer,debug_port);

#endif

		firmware_update();

#ifdef DEBUG_OUTPUT
		buffer_send_string("STAGE: Jumping to new firmware....\r\n",debug_port);
		buffer_send_string("************************************************\n\r",debug_port);

		systick_delay_ms(2000);

#endif

		jump_to_app(NEW_FIRMWARE_START_ADDRESS);

	}
	else
	{
#ifdef DEBUG_OUTPUT
		buffer_send_string("............Running Last Update.........\n\r",debug_port);
		systick_delay_ms(2000);
#endif


		jump_to_app(NEW_FIRMWARE_START_ADDRESS);

	}

	return 0;
}

