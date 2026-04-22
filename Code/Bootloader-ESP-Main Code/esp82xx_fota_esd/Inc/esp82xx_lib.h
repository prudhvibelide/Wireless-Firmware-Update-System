/*
 * File : esp82xx_lib.h
 * Author : Sriramkumar Jayaraman, Harshal Wadhwa
 * Description : Library header for higher-level functions and utilities related to the ESP82xx Wi-Fi modules.
 */

#ifndef __ESP82XX_LIB_H
#define __ESP82XX_LIB_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp82xx_driver.h"
#include "circular_buffer.h"
#include "timebase.h"

#define esp82xx_port		SLAVE_DEV_PORT
#define debug_port			DEBUG_PORT

void esp8266_init(char *ssid, char *password);
void esp82xx_get_version_file(char *dest_buffer);
void esp82xx_get_firmware(char *dest_buffer, const char *firmware_file);

#endif
