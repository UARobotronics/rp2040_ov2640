#include <stdio.h>

#include "ov2640.h"
// #include "ov2640_init.h" // UXGA, 640 x 480, bad quality
// #include "ov2640_init_no_comments.h" // UXGA, 800 x 600, bad quality
#include "ov2640_svga_init.h" // SVGA, 800 x 600, bad quality
// #include "ov2640_uxga_init.h" // UXGA, 1600 x 1200, bad quality
// #include "ov2640_picampinos_init.h" // UXGA, 640 x 480, better quality

#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "image.pio.h"

#include "common.h"
#include "transfer.h"

static const uint8_t OV2640_ADDR = 0x60 >> 1;

void ov2640_init(struct ov2640_config *config) {
	// XCLK generation (~20.83 MHz)
	gpio_set_function(config->pin_xclk, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(config->pin_xclk);
	// 6 cycles (0 to 5), 125 MHz / 6 = ~20.83 MHz wrap rate
	pwm_set_wrap(slice_num, 5);
	pwm_set_gpio_level(config->pin_xclk, 3);
	pwm_set_enabled(slice_num, true);

	// SCCB I2C @ 100 kHz
	gpio_set_function(config->pin_sioc, GPIO_FUNC_I2C);
	gpio_set_function(config->pin_siod, GPIO_FUNC_I2C);
	gpio_pull_up(config->pin_sioc);
	gpio_pull_up(config->pin_siod);
	i2c_init(config->sccb, 100 * 1000);

	// Initialise reset pin
	gpio_init(config->pin_resetb);
	gpio_set_dir(config->pin_resetb, GPIO_OUT);

	// Reset camera, and give it some time to wake back up
	gpio_put(config->pin_resetb, 0);
	sleep_ms(100);
	gpio_put(config->pin_resetb, 1);
	sleep_ms(100);

	// Initialise the camera itself over SCCB
	ov2640_regs_write(config, ov2640_vga);
	ov2640_regs_write(config, ov2640_uxga_cif);

	// // Set RGB565 output mode
	// ov2640_reg_write(config, 0xff, 0x00);
	// ov2640_reg_write(config, 0xDA, (ov2640_reg_read(config, 0xDA) & 0xC) | 0x8);

	// Enable image RX PIO
	uint offset = pio_add_program(config->pio, &image_program);
	image_program_init(config->pio, config->pio_sm, offset, config->pin_y2_pio_base);
}

uint8_t dma_channel = 0;

extern uint8_t frame_buf1[];
extern uint8_t frame_buf2[];
extern uint8_t* transfer_buf;
extern uint8_t* frame_ready_buf;

extern volatile uint16_t current_row;
extern volatile uint8_t active_buffer;
extern uint8_t frame_buffers[BUFFERS_COUNT][CHUNK_SIZE];

void frame_buffer_handler()
{
	// Clear interrupt register
	dma_hw->ints0 = dma_hw->ints0;

	frame_ready_buf = frame_buffers[active_buffer];
	active_buffer = (active_buffer + 1) % BUFFERS_COUNT;
	dma_channel_set_write_addr(dma_channel, frame_buffers[active_buffer], true);

	++current_row;
}

void ov2640_init_capture(struct ov2640_config *config, bool full_frame) {
	current_row = 0;

	irq_set_enabled(DMA_IRQ_0, false);

	dma_channel_config c = dma_channel_get_default_config(config->dma_channel);
	channel_config_set_read_increment(&c, false);
	channel_config_set_write_increment(&c, true);
	channel_config_set_dreq(&c, pio_get_dreq(config->pio, config->pio_sm, false));
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	
	if (full_frame)
	{
		dma_channel_configure(
			config->dma_channel, &c,
			config->image_buf,
			&config->pio->rxf[config->pio_sm],
			config->image_buf_size,
			false
		);

		irq_set_enabled(DMA_IRQ_0, true);
	}
	else
	{
		dma_channel_configure(
			config->dma_channel, &c,
			config->frame_buf,
			&config->pio->rxf[config->pio_sm],
			config->frame_buf_size,
			false
		);

		dma_channel = config->dma_channel;

		dma_channel_set_irq0_enabled(config->dma_channel, true);
		irq_set_exclusive_handler(DMA_IRQ_0, frame_buffer_handler);
		irq_set_enabled(DMA_IRQ_0, true);
	}

	// Wait for vsync rising edge to start frame
	while (gpio_get(config->pin_vsync) == true);
	// uint64_t start = time_us_64();
	// while (gpio_get(config->pin_vsync) == false);
	// uint64_t end = time_us_64();

	// printf("VSYNC has been low for %llu micros\n", end - start);

	dma_channel_start(config->dma_channel);

	if (full_frame)
	{
		dma_channel_wait_for_finish_blocking(config->dma_channel);
	}
	else
	{
		//dma_channel_wait_for_finish_blocking(config->dma_channel);
	}
}

void ov2640_reg_write(struct ov2640_config *config, uint8_t reg, uint8_t value) {
	uint8_t data[] = {reg, value};
	i2c_write_blocking(config->sccb, OV2640_ADDR, data, sizeof(data), false);
}

uint8_t ov2640_reg_read(struct ov2640_config *config, uint8_t reg) {
	i2c_write_blocking(config->sccb, OV2640_ADDR, &reg, 1, false);

	uint8_t value;
	i2c_read_blocking(config->sccb, OV2640_ADDR, &value, 1, false);

	return value;
}

void ov2640_regs_write(struct ov2640_config *config, const uint8_t (*regs_list)[2]) {
	while (1) {
		uint8_t reg = (*regs_list)[0];
		uint8_t value = (*regs_list)[1];

		printf("Reg 0x%02x = 0x%02x\n", reg, value);
		
		if (reg == 0x00 && value == 0x00) {
			break;
		}

		ov2640_reg_write(config, reg, value);

		regs_list++;
	}
}
