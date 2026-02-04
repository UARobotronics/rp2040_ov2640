#pragma once

#include <stdint.h>

#define CHUNK_HEIGHT 1
#define SEND_CHUNK_LENGTH 5
#define NUMBER_OF_CHUNKS 288

enum transfer_type_s {
    TRANSFER_UART,
    TRANSFER_SPI,
    TRANSFER_ETHERNET
};

void send_data(uint8_t* data, uint32_t data_len);