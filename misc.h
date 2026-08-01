#ifndef MISC_H
#define MISC_H

#include "hardware/dma.h"
#include "hardware/pio.h"

#include "common.h"

io_rw_32 dma_channel_get_transfer_count(uint dma_channel);
uint get_pio_sm_by_color_depth(enum color_depth_e depth);
PIO  get_pio_by_color_depth(enum color_depth_e depth);
uint get_dma_channel_by_color_depth(enum color_depth_e depth);

#endif // MISC_H