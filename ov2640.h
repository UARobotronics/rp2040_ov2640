#ifndef OV2640_H
#define OV2640_H
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"

typedef struct {
	i2c_inst_t *sccb;
	uint pin_sioc;
	uint pin_siod;
	
	uint pin_resetb;
	uint pin_xclk;
	uint pin_vsync;
	// Y2, Y3, Y4, Y5, Y6, Y7, Y8, PCLK, HREF
	uint pin_y2_pio_base;

	PIO pio;
	uint pio_sm;

	uint dma_channel;
	uint8_t *image_buf;
	size_t image_buf_size;
	uint8_t *frame_buf;
	size_t frame_buf_size;
} ov2640_config;

void ov2640_init(ov2640_config *config);

void ov2640_init_capture(ov2640_config *config, bool full_frame);
void ov2640_reset_dma_channel(ov2640_config *config);

void ov2640_reg_write(ov2640_config *config, uint8_t reg, uint8_t value);
uint8_t ov2640_reg_read(ov2640_config *config, uint8_t reg);
void ov2640_regs_write(ov2640_config *config, const uint8_t (*regs_list)[2]);

#endif
