/*
 * File : bma.c
 * Author : Sriramkumar Jayaraman
 * Description : This file configures 3-axis firmware for BMA400 Accelorometer Sensor
 */

#include "bma.h"
#include "stm32f4xx.h"
#include <stdint.h>


void I2C1_Init(void) {
    // Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  // Enable GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;   // Enable I2C1 clock

    // Configure GPIO pins PB8 (SCL) and PB9 (SDA)
    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);  // Clear bits
    GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1; // Alternate function

    // Set to open-drain output type
    GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;

    // Enable pull-up resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9);    // Clear bits
    GPIOB->PUPDR |= (1U << GPIO_PUPDR_PUPD8_Pos) | (1U << GPIO_PUPDR_PUPD9_Pos);

    // Set alternate function 4 (I2C)
    GPIOB->AFR[1] &= ~(0xFF);              // Clear AF for PB8 and PB9
    GPIOB->AFR[1] |= (4U << 0) | (4U << 4); // AF4 for PB8 and PB9

    // Reset and configure I2C
    I2C1->CR1 = 0;                         // Reset I2C configuration

    // Set I2C clock frequency
    I2C1->CR2 &= ~I2C_CR2_FREQ;           // Clear FREQ bits
    I2C1->CR2 |= 16;                       // Set APB1 frequency = 16MHz

    // Configure CCR for 400kHz Fast Mode
    I2C1->CCR = 0;                         // Clear CCR register
    I2C1->CCR |= I2C_CCR_FS;              // Fast mode
    I2C1->CCR |= 13;                       // CCR calculation: Tscl = CCR * Tpclk1 * 3
                                          // For 400kHz: CCR = 16MHz/(3 * 400kHz) ≈ 13

    // Configure rise time register
    I2C1->TRISE = 17;                      // Maximum rise time in Fast Mode
                                          // (1000ns / (1/16MHz)) + 1

    // Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;              // Enable I2C peripheral

    // Short delay to ensure stable I2C operation
    for(volatile int i = 0; i < 1000; i++);
}
// Function to write to BMA400 register
static uint8_t BMA400_WriteReg(uint8_t reg, uint8_t data) {
    uint16_t timeout;

    // Start condition
    I2C1->CR1 |= I2C_CR1_START;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        if (--timeout == 0) return 0;
    }

    // Send slave address
    I2C1->DR = BMA400_ADDR << 1; // Write mode
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (--timeout == 0) return 0;
        if (I2C1->SR1 & I2C_SR1_AF) return 0;
    }
    uint8_t temp = I2C1->SR1 | I2C1->SR2;
    (void)temp;

    // Send register address
    I2C1->DR = reg;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) {
        if (--timeout == 0) return 0;
    }

    // Send data
    I2C1->DR = data;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) {
        if (--timeout == 0) return 0;
    }

    // Stop condition
    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

// Function to read from BMA400 register
static uint8_t BMA400_ReadReg(uint8_t reg, uint8_t* data) {
    uint16_t timeout;

    // Start condition
    I2C1->CR1 |= I2C_CR1_START;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        if (--timeout == 0) return 0;
    }

    // Send slave address (write mode)
    I2C1->DR = BMA400_ADDR << 1;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (--timeout == 0) return 0;
        if (I2C1->SR1 & I2C_SR1_AF) return 0;
    }
    uint8_t temp = I2C1->SR1 | I2C1->SR2;
    (void)temp;

    // Send register address
    I2C1->DR = reg;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) {
        if (--timeout == 0) return 0;
    }

    // Restart for reading
    I2C1->CR1 |= I2C_CR1_START;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) {
        if (--timeout == 0) return 0;
    }

    // Send slave address (read mode)
    I2C1->DR = (BMA400_ADDR << 1) | 0x01;
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (--timeout == 0) return 0;
    }
    temp = I2C1->SR1 | I2C1->SR2;

    // Configure for NACK
    I2C1->CR1 &= ~I2C_CR1_ACK;

    // Read data
    timeout = 1000;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)) {
        if (--timeout == 0) return 0;
    }
    *data = I2C1->DR;

    // Stop condition
    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

// Initialize BMA400
uint8_t BMA400_Init(void) {
    uint8_t chip_id;

    // Read chip ID to verify communication
    if (!BMA400_ReadReg(BMA400_CHIP_ID, &chip_id)) {
        return 0;
    }

    if (chip_id != 0x90) {  // Verify correct chip ID
        return 0;
    }

    // Configure accelerometer
    // Power control - enable accelerometer
    BMA400_WriteReg(BMA400_POWER_CTRL, 0x04);

    // Configure ACC_CONFIG0:
    // - Range: ±2g (00)
    // - OSR: Normal mode (00)
    // - ODR: 100Hz (0101)
    BMA400_WriteReg(BMA400_ACC_CONFIG_0, 0x05);

    return 1;
}

// Read X-axis acceleration
int16_t BMA400_ReadXAxis(void) {
    uint8_t lsb, msb;
    int16_t x_acc;

    // Read X-axis LSB and MSB
    BMA400_ReadReg(BMA400_ACC_X_LSB, &lsb);
    BMA400_ReadReg(BMA400_ACC_X_MSB, &msb);

    // Combine MSB and LSB (12-bit resolution)
    x_acc = (msb << 4) | (lsb >> 4);

    // Convert to signed value
    if (x_acc > 2047) {
        x_acc -= 4096;
    }

    return x_acc;
}

int16_t BMA400_ReadYAxis(void) {
    uint8_t lsb, msb;
    int16_t y_acc;

    // Read Y-axis LSB and MSB
    BMA400_ReadReg(BMA400_ACC_Y_LSB, &lsb);
    BMA400_ReadReg(BMA400_ACC_Y_MSB, &msb);

    // Combine MSB and LSB (12-bit resolution)
    y_acc = (msb << 4) | (lsb >> 4);

    // Convert to signed value
    if (y_acc > 2047) {
        y_acc -= 4096;
    }

    return y_acc;
}

// Read Z-axis acceleration
int16_t BMA400_ReadZAxis(void) {
    uint8_t lsb, msb;
    int16_t z_acc;

    // Read Z-axis LSB and MSB
    BMA400_ReadReg(BMA400_ACC_Z_LSB, &lsb);
    BMA400_ReadReg(BMA400_ACC_Z_MSB, &msb);

    // Combine MSB and LSB (12-bit resolution)
    z_acc = (msb << 4) | (lsb >> 4);

    // Convert to signed value
    if (z_acc > 2047) {
        z_acc -= 4096;
    }

    return z_acc;
}

