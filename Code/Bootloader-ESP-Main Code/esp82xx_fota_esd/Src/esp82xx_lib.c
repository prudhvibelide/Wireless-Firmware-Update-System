/*
 * File : esp82xx_lib.c
 * Author : Sriramkumar Jayaraman, Harshal Wadhwa
 * Description : This file is for ESP82xx module to connect Wi-Fi, send HTTP GET requests, and fetch firmware/version files.
 */

#include "esp82xx_lib.h"



#define TEMP_BUFF_LNG_SZ		600
#define TEMP_BUFF2_SHT_SZ		30

// Macros for commonly used strings in the function
#define SERVER_ADDRESS "esd-fota.batcave.net"

#define TCP_START_COMMAND "AT+CIPSTART=\"TCP\",\"" SERVER_ADDRESS "\",80\r\n"
#define OK_RESPONSE "OK\r\n"
#define SEND_PROMPT ">"
#define SEND_OK_RESPONSE "SEND OK\r\n"
#define END_OF_HEADERS "\r\n\r\n"
#define LINE_TERMINATOR "\r\n"
#define CLOSED_RESPONSE "CLOSED\r\n"
#define CIPSEND_COMMAND "AT+CIPSEND=%d\r\n"
#define HTTP_GET_REQUEST_VER_FILE "GET /releases/firmware_version.txt HTTP/1.1\r\n" \
                          "Host: " SERVER_ADDRESS "\r\n" \
                          "Connection: close\r\n\r\n"

#define HTTP_GET_REQUEST_FIRM "GET /releases/%s HTTP/1.1\r\n" \
                                "Host: " SERVER_ADDRESS "\r\n" \
                                "Connection: close\r\n\r\n"


static void esp82xx_reset(void);
static void esp82xx_startup_test(void);
static void esp82xx_sta_mode(void);
static void esp82xx_ap_connect(char *ssid, char *password);

void esp8266_init(char *ssid, char *password)
{
	esp82xx_reset();
	esp82xx_startup_test();
	esp82xx_sta_mode();
	esp82xx_ap_connect(ssid, password);
}
 static void esp82xx_startup_test(void)
{
	/*Clear ESP uart buffer*/
	buffer_clear(esp82xx_port);

	/*Send test command*/
	buffer_send_string("AT\r\n",esp82xx_port);

	/*Wait for "OK" response*/
	while(!(is_response("OK\r\n"))){}


	buffer_send_string("AT startup test successful....\n\r",debug_port);
}


static void esp82xx_reset(void)
{
	/*Clear ESP uart buffer*/
	buffer_clear(esp82xx_port);

	/*Send test command*/
	buffer_send_string("AT+RST\r\n",esp82xx_port);

	/*Delay for 1 second*/
	systick_delay_ms(1000);

	/*Wait for "OK" response*/
	while(!(is_response("OK\r\n"))){}

	buffer_send_string("Reset was successful....\n\r",debug_port);
}


static void esp82xx_sta_mode(void)
{
	/*Clear ESP uart buffer*/
	buffer_clear(esp82xx_port);

	/*Send test command*/
	buffer_send_string("AT+CWMODE=1\r\n",esp82xx_port);

	/*Wait for "OK" response*/
	while(!(is_response("OK\r\n"))){}


	buffer_send_string("STA Mode set successful....\n\r",debug_port);
}


static void esp82xx_ap_connect(char *ssid, char *password)
{
	char data[80];

	/*Clear ESP uart buffer*/
	buffer_clear(esp82xx_port);

	buffer_send_string("Connecting to access point....\n\r",debug_port);

	/*Pust ssid, password and command into one string packet*/
	sprintf(data,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);

	/*Send test command*/
	buffer_send_string(data,esp82xx_port);



	/*Wait for "OK" response*/
	while(!(is_response("OK\r\n"))){}

    sprintf(data,"Connected : \"%s\"\r\n",ssid);

	buffer_send_string(data,debug_port);
}

void esp82xx_get_firmware(char *dest_buffer, const char *firmware_file)
{
	/*Buffer to hold HTTP GET request and other string*/
	char request_buffer[TEMP_BUFF_LNG_SZ] = {0};
	char send_command_buffer[TEMP_BUFF2_SHT_SZ] = {0};

	/*Clear esp uart buffer*/
	buffer_clear(esp82xx_port);

	/*Establish a TCP connection to the server*/
	buffer_send_string(TCP_START_COMMAND,esp82xx_port);

	/*Wait until connection is successfully established*/
	while(!is_response(OK_RESPONSE)){}

	/*Prepare the HTTP GET request to retrieve the file*/
	snprintf(request_buffer, sizeof(request_buffer),HTTP_GET_REQUEST_FIRM,firmware_file);

	/*Get the length of the prepared request*/
	int request_length = strlen(request_buffer);

	/*Prepare the AT+CIPSEND command with the request length */

	snprintf(send_command_buffer,sizeof(send_command_buffer),CIPSEND_COMMAND,request_length);

	/*Send the AT+CIPSEND command */
	buffer_send_string(send_command_buffer,esp82xx_port);
	buffer_send_string(send_command_buffer,debug_port);

	/*Wait for the ESP to be ready to receive the HTTP GET request*/
	while(!is_response(SEND_PROMPT)){}

	/*Send the actual HTTP GET request to the server*/
	buffer_send_string(request_buffer,esp82xx_port);

	/*Wait to confirm that the data was sent*/
	while(!is_response(SEND_OK_RESPONSE)){}

	/*Wait for the HTTP response header to finish*/
	while(!is_response(END_OF_HEADERS)){}

	/*Copy the data to destination buffer*/
	while(!copy_up_to_string(CLOSED_RESPONSE,dest_buffer)){}



}


void esp82xx_get_version_file(char *dest_buffer)
{
	/*Buffer to hold HTTP GET request and other string*/
	char request_buffer[TEMP_BUFF_LNG_SZ] = {0};
	char send_command_buffer[TEMP_BUFF2_SHT_SZ] = {0};

	/*Clear esp uart buffer*/
	buffer_clear(esp82xx_port);

	/*Establish a TCP connection to the server*/
	buffer_send_string(TCP_START_COMMAND,esp82xx_port);

	/*Wait until connection is successfully established*/
	while(!is_response(OK_RESPONSE)){}

	/*Prepare the HTTP GET request to retrieve the file*/
	snprintf(request_buffer, sizeof(request_buffer),HTTP_GET_REQUEST_VER_FILE);

	/*Get the length of the prepared request*/
	int request_length = strlen(request_buffer);

	/*Prepare the AT+CIPSEND command with the request length */

	snprintf(send_command_buffer,sizeof(send_command_buffer),CIPSEND_COMMAND,request_length);

	/*Send the AT+CIPSEND command */
	buffer_send_string(send_command_buffer,esp82xx_port);

	/*Wait for the ESP to be ready to receive the HTTP GET request*/
	while(!is_response(SEND_PROMPT)){}

	/*Send the actual HTTP GET request to the server*/
	buffer_send_string(request_buffer,esp82xx_port);

	/*Wait to confirm that the data was sent*/
	while(!is_response(SEND_OK_RESPONSE)){}

	/*Wait for the HTTP response header to finish*/
	while(!is_response(END_OF_HEADERS)){}

	/*Copy the data to destination buffer*/
	while(!copy_up_to_string(CLOSED_RESPONSE,dest_buffer)){}



}
