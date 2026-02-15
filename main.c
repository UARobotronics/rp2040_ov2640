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

enum pixel_format_s {
	PIXEL_FORMAT_RGB,
	PIXEL_FORMAT_YUV,
};

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

#define FRAME_WIDTH 352
#define FRAME_HEIGHT 288
#define BYTES_PER_PIXEL 2

// uint8_t image_buf[WIDTH * HEIGHT * BYTES_PER_PIXEL]; // RGB565 format
uint8_t image_buf[FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL];	// YUV422 format, 202752 bytes

#define CHUNK_SIZE FRAME_WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL
uint8_t frame_buf1[CHUNK_SIZE];  // YUV422 format
uint8_t frame_buf2[CHUNK_SIZE];  // YUV422 format
uint8_t* frame_ready_buf = NULL;
uint8_t process_buf1[CHUNK_SIZE];
uint8_t process_buf2[CHUNK_SIZE];
uint8_t* transfer_buf = NULL;

extern uint8_t chunk_count;

volatile uint16_t buf_tail = 0;
volatile uint16_t received_chunks = 0;
volatile uint16_t current_row = 0;

enum transfer_type_s transfer_type = TRANSFER_ETHERNET;
enum pixel_format_s pixel_format = PIXEL_FORMAT_YUV;

void core1_main()
{
	while (true) {
		if (transfer_buf) {
			uint64_t start = time_us_64();
			send_data(transfer_buf, CHUNK_SIZE / 2);
			// send_data(transfer_buf, 1);
			uint64_t end = time_us_64();
			// printf("Sent row data in %llu micros\n", end - start);
			++received_chunks;
			transfer_buf = NULL;
		}
	}
}

void reset_image_buffer()
{
	memset(image_buf, 0, sizeof(image_buf));
	buf_tail = 0;
	received_chunks = 0;
}

uint16_t process_yuv_raw_data(volatile uint8_t* in_buf, uint16_t in_len, uint8_t* out_buf)
{
	uint16_t out_len = 0;
	for (int i = 1; i < in_len; i += 2) {
		out_buf[out_len] = in_buf[i];
		// out_buf[out_len] = in_buf[i] & 0xFC;
		++out_len;
	}

	// out_len = in_len / 2;
	// memcpy(out_buf, in_buf, out_len);

	return out_len;
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
			uint16_t frame_count = 0;

			bool full_frame = false;

			ov2640_capture_frame(&config, full_frame);
			received_chunks = 0;
			uint16_t prev_row = current_row;
			uint16_t missed_rows = 0;
			while (time_us_64() - start_us < 1 * 1000 * 1000) {
				if (full_frame)
				{
					while (gpio_get(config.pin_vsync) == true);
					while (gpio_get(config.pin_vsync) == false);
					send_data(config.image_buf, config.image_buf_size);
				}
				else
				{
					uint8_t* process_buf = NULL;
					if (frame_ready_buf)
					{
						if (frame_ready_buf == frame_buf1)
							process_buf = process_buf1;
						else if (frame_ready_buf == frame_buf2)
							process_buf = process_buf2;

						if (process_buf)
						{
							switch (pixel_format)
							{
								case PIXEL_FORMAT_YUV:
									uint64_t start = time_us_64();
									uint16_t len = process_yuv_raw_data(frame_ready_buf, CHUNK_SIZE, process_buf);
									transfer_buf = process_buf;
									uint64_t end = time_us_64();

									// printf("Processed row data in %llu micros\n", end - start);
									// printf("Processed buffer length for %u row: %u\n", current_row, len);
									// printf("Received %u row data\n", current_row);

									if (current_row - prev_row > 1)
										++missed_rows;

									prev_row = current_row;

									break;
								case PIXEL_FORMAT_RGB:
								default:
									break;
							}
						}

						frame_ready_buf = NULL;
					}
				}

				++frame_count;
			}

			// cmd = 0;

			if (full_frame)
			{
				printf("FPS: %u\n", frame_count);
			}
			else
			{
				printf("FPS: %u\n", received_chunks / FRAME_HEIGHT);
				printf("Obtained chunks: %u\n", received_chunks);
				printf("Missed rows: %u\n", missed_rows);
			}
		}
	}

	return 0;
}
