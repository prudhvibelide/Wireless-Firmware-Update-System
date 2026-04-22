/*
 * File : fota_processor.c
 * Author : Prudhvi Raj Belide, Harshal Wadhwa
 * Description : This file handles the process of updating firmware on an STM32 via a Wifi network connection,
 * specifically focusing on the retrieval, parsing, and storing of the firmware.
 */


#include "fota_processor.h"
#define IPD_MARKER "\r\n+IPD,"
#define CLOSED_MARKER "\r\nCLOSED\r\n"
#define COLON_MARKER ":"

char firmware_buffer[MAX_FIRMWARE_SIZE] = {0};

char temp_rx_buffer[TEMP_RX_BUFF_SZ] = {0};


#define EMPTY_MEM		0xFFFFFFFF
typedef void (*func_ptr)(void);

void jump_to_app(uint32_t address)
{
	uint32_t app_start_address;
	func_ptr jump_to_app;

	/*Preparing for jump*/

	__disable_irq();   /*Disable global interrupts*/
	SysTick->CTRL = 0; /*Disable Systick*/

	/*Disable and re-enable clock for AHB*/
	RCC->AHB1RSTR  =  0xFFFFFFFF;
	RCC->AHB1RSTR  =  0x00000000;

	if(  (*(uint32_t *)address) != EMPTY_MEM)
	{
		/*Get reset handler address*/
		app_start_address =  *(uint32_t *)(address + 4);

		/*Convert to function pointer*/
		jump_to_app =  (func_ptr)app_start_address;

		/*Initial the MSP*/
		__set_MSP(*(uint32_t *)address);

		/*Jump*/
		jump_to_app();
	}
	else{

		//Deal with emptiness

	}

}

/**
 * @brief Calculates the length of a null-terminated string.
 *
 * @param str Pointer to the null-terminated string.
 * @return The length of the string (number of characters before the null terminator).
 */
int string_length(const char *str) {
    int length = 0;

    // Iterate through the string until the null terminator is found
    while (str[length] != '\0') {
        length++;
    }

    return length;
}
/**
 * @brief Finds the first occurrence of a substring within a given string.
 *
 * @param str Pointer to the string in which to search for the substring.
 * @param substr Pointer to the substring to search for.
 * @param str_size The size of the main string.
 * @return Pointer to the first occurrence of the substring within the main string,
 *         or NULL if the substring is not found.
 */
char* find_substring(const char *str, const char *substr, int str_size) {
    int str_len = string_length(substr);
    char *found_ptr = NULL;

    // Iterate through the main string up to the point where the substring could fully fit
    for (int i = 0; i <= (str_size - str_len); i++) {
        int j = 0;

        // Check if the substring matches at this position
        while (j < str_len && str[i + j] == substr[j]) {
            j++;
        }

        // If we matched the entire substring, return the pointer to the start of the match
        if (j == str_len) {
            found_ptr = (char *)&str[i];
            break;
        }
    }

    return found_ptr;
}

/**
 * @brief Parses the firmware data from the source buffer, extracting the valid portions and storing them in the destination buffer.
 *
 * @param dst Pointer to the destination buffer where parsed data will be stored.
 * @param src Pointer to the source buffer containing the raw firmware data.
 * @param size The size of the source buffer in bytes.
 */
void firmware_parse(char *dst, const char *src, int size) {
    uint16_t dst_index = 0;            // Index for tracking the position in the destination buffer
    const char *current_pos = src;     // Pointer to the current position in the source buffer
    int remaining_size = size;         // Tracks the remaining bytes to process in the source buffer

    // Iterate over the source buffer, extracting valid firmware data
    while (find_substring(current_pos, IPD_MARKER, remaining_size)) {
        // Calculate the number of valid bytes before the IPD_MARKER
        uint16_t valid_data_length = find_substring(current_pos, IPD_MARKER, remaining_size) - current_pos;

        // Copy the valid data to the destination buffer
        for (int i = 0; i < valid_data_length; i++) {
            dst[dst_index] = current_pos[i];
            dst_index++;
        }

        // Move the current position past the valid data and the IPD_MARKER
        current_pos += (valid_data_length + sizeof(IPD_MARKER) - 1);
        remaining_size -= (valid_data_length + sizeof(IPD_MARKER) - 1);

        // Find the position of the next COLON_MARKER and adjust the remaining size
        const char* next_colon_pos = find_substring(current_pos, COLON_MARKER, remaining_size);
        remaining_size -= (int)(next_colon_pos + 1 - current_pos);
        current_pos = (next_colon_pos + 1);  // Move to the position after the colon
    }

    // Extract any remaining valid data before the CLOSED_MARKER
    uint16_t valid_data_length = find_substring(current_pos, CLOSED_MARKER, remaining_size) - current_pos;

    // Copy the remaining valid data to the destination buffer
    for (int i = 0; i < valid_data_length; i++) {
        dst[dst_index] = current_pos[i];
        dst_index++;
    }
}


void firmware_update(void)
{

#ifdef DEBUG_OUTPUT
		buffer_send_string("STAGE: Getting the firmware....\r\n",debug_port);

#endif

	/*Copy firmware from rx_buffer and put into temp_rx_buffer*/
	 esp82xx_get_firmware(temp_rx_buffer,FIRMWARE);
#ifdef DEBUG_OUTPUT
		buffer_send_string("STAGE: Parsing the firmware....\r\n",debug_port);

#endif

	 /*Parse firmware data from content temp_rx_buffer*/
	 firmware_parse(firmware_buffer, temp_rx_buffer, sizeof(firmware_buffer));
#ifdef DEBUG_OUTPUT
		buffer_send_string("STAGE: Writing the firmware to memory....\r\n",debug_port);

#endif

	 /*Write firmware data to microcontroller's flash memory*/
	 flash_write_data_byte(NEW_FIRMWARE_START_ADDRESS,(uint8_t *) firmware_buffer, sizeof(firmware_buffer));

}
