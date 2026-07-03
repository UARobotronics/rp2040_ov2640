#ifndef _W5500_H_
#define _W5500_H_

#include <stdbool.h>

#include "network/network.h"
#include "hardware/spi.h"

typedef void (*w5500_reset_hw_func)(void);

bool w5500_network_init(struct ETHERNET_INTERFACE_s* interface, spi_inst_t* spi, uint cs_gpio, uint reset_gpio);
bool w5500_network_init_with_reset_gpio(struct ETHERNET_INTERFACE_s* interface, spi_inst_t* spi, uint cs_gpio, uint reset_gpio);
bool w5500_network_init_with_reset_func(struct ETHERNET_INTERFACE_s* interface, spi_inst_t* spi, uint cs_gpio, w5500_reset_hw_func reset_func);

#endif