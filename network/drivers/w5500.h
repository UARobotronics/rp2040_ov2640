#ifndef _W5500_H_
#define _W5500_H_

#include <stdbool.h>

#include "network/network.h"
#include "hardware/spi.h"

bool w5500_network_init(struct ETHERNET_INTERFACE_s* interface, spi_inst_t* spi, uint cs_gpio, uint reset_gpio);

#endif