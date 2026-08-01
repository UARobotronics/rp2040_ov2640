#include "misc.h"

inline io_rw_32 dma_channel_get_transfer_count(uint dma_channel)
{
    return dma_channel_hw_addr(dma_channel)->transfer_count;
}

inline uint get_pio_sm_by_color_depth(enum color_depth_e depth)
{
	switch(depth)
    {
        case COLOR_DEPTH_8_BITS : return 0;
        case COLOR_DEPTH_7_BITS : return 1;
        case COLOR_DEPTH_6_BITS : return 2;
        case COLOR_DEPTH_5_BITS : return 3;
        case COLOR_DEPTH_4_BITS : return 0;
        case COLOR_DEPTH_3_BITS : return 1;
        case COLOR_DEPTH_2_BITS : return 2;
        case COLOR_DEPTH_1_BITS : return 3;
    }
}

inline PIO  get_pio_by_color_depth(enum color_depth_e depth)
{
	switch(depth)
    {
        case COLOR_DEPTH_8_BITS : return pio0;
        case COLOR_DEPTH_7_BITS : return pio0;
        case COLOR_DEPTH_6_BITS : return pio0;
        case COLOR_DEPTH_5_BITS : return pio0;
        case COLOR_DEPTH_4_BITS : return pio1;
        case COLOR_DEPTH_3_BITS : return pio1;
        case COLOR_DEPTH_2_BITS : return pio1;
        case COLOR_DEPTH_1_BITS : return pio1;
    }
}

inline uint get_dma_channel_by_color_depth(enum color_depth_e depth)
{
    return 0;
}
