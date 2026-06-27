#ifndef MISC_H
#define MISC_H

#include "hardware/dma.h"

io_rw_32 dma_channel_get_transfer_count(uint dma_channel)
{
    return dma_channel_hw_addr(dma_channel)->transfer_count;
}

#endif // MISC_H