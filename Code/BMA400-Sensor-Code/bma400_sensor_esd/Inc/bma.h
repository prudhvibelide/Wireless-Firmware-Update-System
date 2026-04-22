/*
 * File : bma.h
 * Author : Sriramkumar Jayaraman
 * Description : This header file is for BMA400 Accelorometer Sensor
 */

#ifndef BMA_H_
#define BMA_H_


#include <stdint.h>
// BMA400 Register Addresses
#define BMA400_ADDR              0x14  // I2C address when SDO is low (0x15 when high)
#define BMA400_CHIP_ID          0x00   // Should return 0x90
#define BMA400_STATUS           0x03
#define BMA400_ACC_X_LSB       0x04
#define BMA400_ACC_X_MSB       0x05
#define BMA400_ACC_Y_LSB       0x06
#define BMA400_ACC_Y_MSB       0x07
#define BMA400_ACC_Z_LSB       0x08
#define BMA400_ACC_Z_MSB       0x09
#define BMA400_ACC_CONFIG_0    0x19
#define BMA400_ACC_CONFIG_1    0x1A
#define BMA400_ACC_CONFIG_2    0x1B
#define BMA400_INT_CONFIG_0    0x1F
#define BMA400_POWER_CONF      0x1C
#define BMA400_POWER_CTRL      0x1D


// Function Prototypes
uint8_t BMA400_Init(void);         // Initialize the BMA400
int16_t BMA400_ReadXAxis(void);
int16_t BMA400_ReadYAxis(void);
int16_t BMA400_ReadZAxis(void);
void I2C1_Init(void);


#endif /* BMA_H_ */
