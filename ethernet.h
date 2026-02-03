#pragma once

#include <stdbool.h>
#include <stdint.h>

bool ethernet_init();
bool ethernet_send_udp(uint8_t* data, uint32_t data_len);