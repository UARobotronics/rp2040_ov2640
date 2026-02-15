#pragma once

#include <stdint.h>

#define CHUNK_HEIGHT 1

enum transfer_type_s {
    TRANSFER_UART,
    TRANSFER_SPI,
    TRANSFER_ETHERNET
};

void send_data(uint8_t* data, uint32_t data_len);