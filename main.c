#include <stdio.h>
#include <string.h>

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"

#include "network/drivers/w5500.h"
#include "network/network.h"

#include "ethernet.h"
#include "ov2640.h"
#include "transfer.h"

const int PIN_LED = 25;

const int PIN_CAM_SIOC = 21; // I2C0 SCL
const int PIN_CAM_SIOD = 20; // I2C0 SDA
const int PIN_CAM_RESETB = 2;
const int PIN_CAM_XCLK = 3;
const int PIN_CAM_VSYNC = 14;
const int PIN_CAM_Y2_PIO_BASE = 4;

const uint8_t CMD_REG_WRITE = 0xAA;
const uint8_t CMD_REG_READ = 0xBB;
const uint8_t CMD_CAPTURE = 0xCC;

#define WIDTH 352
#define HEIGHT 288
#define BYTES_PER_PIXEL 2

//uint8_t image_buf[WIDTH * HEIGHT * BYTES_PER_PIXEL]; // RGB565 format
uint8_t image_buf[WIDTH * HEIGHT * BYTES_PER_PIXEL]; // YUV422 format, 202752 bytes
uint8_t frame_buf1[WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL]; // YUV422 format
uint8_t frame_buf2[WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL]; // YUV422 format

volatile bool frame_chunk_ready = false;
extern uint8_t chunk_count;

volatile uint16_t buf_tail = 0;
volatile uint16_t received_chunks = 0;

volatile uint16_t tail_history[20];
volatile uint16_t tail_history_count = 0;

enum transfer_type_s transfer_type = TRANSFER_ETHERNET;

void core1_main()
{
	while (true) {
		if (frame_chunk_ready) {
			// memcpy(image_buf + buf_tail, frame_buf1, sizeof(frame_buf1));
			// buf_tail += sizeof(frame_buf1);
			// memset(image_buf + buf_tail, 0, WIDTH * 10 * BYTES_PER_PIXEL);
			// buf_tail += WIDTH * 10;

			if (received_chunks % 2 == 0) {
				send_data(frame_buf1, sizeof(frame_buf1));
				// send_data(frame_buf1, 100);
			} else {
				send_data(frame_buf2, sizeof(frame_buf2));
				// send_data(frame_buf2, 100);
			}

			// if (received_chunks == 10) {
			// 	send_data(frame_buf1, sizeof(frame_buf1));
			// }

			// if (received_chunks < NUMBER_OF_CHUNKS) {
			// 	if (received_chunks % 2 == 0) {
			// 		send_data(frame_buf1, SEND_CHUNK_LENGTH * BYTES_PER_PIXEL);
			// 	} else {
			// 		send_data(frame_buf2, SEND_CHUNK_LENGTH * BYTES_PER_PIXEL);
			// 	}
			// }

			// tail_history[tail_history_count++] = buf_tail;

			++received_chunks;
			frame_chunk_ready = false;
		}
	}
}

void reset_image_buffer()
{
	memset(image_buf, 0, sizeof(image_buf));
	buf_tail = 0;
	received_chunks = 0;

	// memset(tail_history, 0, sizeof(tail_history));
	tail_history_count = 0;
}

int main() {
	stdio_init_all();
	uart_set_baudrate(uart0, 460800);

	printf("\n\nBooted!\n");

	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);

	struct ov2640_config config;
	config.sccb = i2c0;
	config.pin_sioc = PIN_CAM_SIOC;
	config.pin_siod = PIN_CAM_SIOD;

	config.pin_resetb = PIN_CAM_RESETB;
	config.pin_xclk = PIN_CAM_XCLK;
	config.pin_vsync = PIN_CAM_VSYNC;
	config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

	config.pio = pio0;
	config.pio_sm = 0;

	config.dma_channel = 0;
	config.image_buf = image_buf;
	config.image_buf_size = sizeof(image_buf);
	config.frame_buf1 = frame_buf1;
	config.frame_buf_size = sizeof(frame_buf1);
	config.frame_buf2 = frame_buf2;

	reset_image_buffer();

	ov2640_init(&config);

	printf("Camera initialized\n");

	if (transfer_type == TRANSFER_ETHERNET)
	{
		bool success = ethernet_init();
		if (!success)
		{
			printf("Ethernet init failed!\n");

			return 1;
		}
	}
	
	multicore_launch_core1(core1_main);

	uint8_t cmd = CMD_CAPTURE;

	while (true) {
		// uart_read_blocking(uart0, &cmd, 1);
		// printf("Cmd %u received\n", cmd);

		gpio_put(PIN_LED, !gpio_get(PIN_LED));
		
		if (cmd == CMD_REG_WRITE) {
			printf("Cmd: register write\n");
			uint8_t reg;
			uart_read_blocking(uart0, &reg, 1);

			uint8_t value;
			uart_read_blocking(uart0, &value, 1);

			ov2640_reg_write(&config, reg, value);
		} else if (cmd == CMD_REG_READ) {
			printf("Cmd: register read\n");
			uint8_t reg;
			uart_read_blocking(uart0, &reg, 1);

			uint8_t value = ov2640_reg_read(&config, reg);

			uart_write_blocking(uart0, &value, 1);
		} else if (cmd == CMD_CAPTURE) {
			printf("Cmd: capture frame\n");
        	uint64_t start_us = time_us_64();
			uint8_t frame_count = 0;

			bool full_frame = true;

			ov2640_capture_frame(&config, full_frame);
			while (time_us_64() - start_us < 1 * 1000 * 1000) {
				// ov2640_capture_frame(&config);

				while (gpio_get(config.pin_vsync) == true);
				while (gpio_get(config.pin_vsync) == false);

				if (full_frame) {
					// uint64_t start_us = time_us_64();
					send_data(config.image_buf, config.image_buf_size);
					// uint64_t end_us = time_us_64();
					// printf("%llu micros passed\n", end_us - start_us);
				}

				++frame_count;
			}

			// cmd = 0;

			sleep_ms(200);
			printf("FPS: %u\n", frame_count);
            
			// if (full_frame) {
	        // 	uint64_t start_us = time_us_64();
			// 	send_data(config.image_buf, config.image_buf_size);
	        // 	uint64_t end_us = time_us_64();
			// 	printf("%llu micros passed\n", end_us - start_us);
			// 	//sleep_ms(100);
			// }
		}
	}

	return 0;
}
