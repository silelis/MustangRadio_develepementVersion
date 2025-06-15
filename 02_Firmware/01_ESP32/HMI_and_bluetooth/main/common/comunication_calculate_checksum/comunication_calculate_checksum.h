#pragma once
#include <stdio.h>
#include <stdint.h>


#define I2C_SLAVE_CRC_NONE					0
#define I2C_SLAVE_CRC_XOR					1
#define I2C_SLAVE_CRC_8						2

#define I2C_SLAVE_CRC_METHOD				I2C_SLAVE_CRC_XOR

uint8_t calculate_checksum(const void *buffer, size_t length);



