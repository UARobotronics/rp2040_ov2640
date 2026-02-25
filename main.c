#include <stdio.h>
#include <string.h>
#include "tusb.h"

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

// typedef struct __attribute__((packed))
// {
// 	uint16_t frame : 13;
// 	uint16_t row : 11;
// 	uint8_t flags: 6;
// 	uint8_t format: 2;
// 	uint8_t* data;
// } packet_data_t;

typedef struct
{
	bool ready;
	uint16_t pixel_data_size;
	// packet_data_t packet_data;
	uint8_t* pixel_data;
} transfer_data_t;

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

const int PACKET_HEADER_SIZE = 4;

// uint8_t image_buf[WIDTH * HEIGHT * BYTES_PER_PIXEL]; // RGB565 format
// uint8_t image_buf[FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL];	// YUV422 format, 202752 bytes
uint8_t image_buf[352 * 288 * BYTES_PER_PIXEL];	// YUV422 format, 202752 bytes

#define CHUNK_SIZE FRAME_WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL
uint8_t frame_buf1[CHUNK_SIZE];  // YUV422 format
uint8_t frame_buf2[CHUNK_SIZE];  // YUV422 format
uint8_t* frame_ready_buf = NULL;
uint8_t process_buf1[CHUNK_SIZE];
uint8_t process_buf2[CHUNK_SIZE];

// uint8_t* transfer_buf = NULL;
transfer_data_t transfer_data = {
	.ready = false,
	.pixel_data_size = 0,
	.pixel_data = NULL
};

extern volatile uint16_t chunk_count;

volatile uint16_t received_chunks = 0;
uint16_t current_row = 0;
uint16_t current_frame = 0;

enum transfer_type_s transfer_type = TRANSFER_ETHERNET;
enum pixel_format_s pixel_format = PIXEL_FORMAT_YUV;
uint8_t pixel_encoding_flags = 0;
uint8_t pixel_encoding_format = 0;
bool full_frame_capturing = false;

void core1_main()
{
	while (true)
	{
		if (transfer_data.ready)
		{
			transfer_data.ready = false;
			uint64_t start = time_us_64();
			send_data(transfer_data.pixel_data, transfer_data.pixel_data_size);
			uint64_t end = time_us_64();
			// printf("Sent row data in %llu micros\n", end - start);
			++received_chunks;
		}
	}
}

uint16_t process_yuv_raw_data(volatile uint8_t* in_buf, uint16_t in_len, uint8_t* out_buf)
{
	uint16_t out_len = 0;
	
	// Create packet header first
	// 13 bits for frame number

	out_buf[0] = (current_frame >> 5) & 0xFF;
	out_buf[1] = (current_frame & 0x1F) << 3;
	// 11 bits for row number
	out_buf[1] |= (current_row >> 8) & 0x7;
	out_buf[2] = current_row & 0xFF;
	// 6 bits for flags
	out_buf[3] = (pixel_encoding_flags << 2) & 0xFF;
	// 2 bits for pixel format
	// 0 - 5 bits of color data
	// 1 - 6 bits of color data
	// 2 - 7 bits of color data
	// 3 - 8 bits of color data
	out_buf[3] |= pixel_encoding_format & 0x3;

	out_len = 4;

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

	// Wait until USB is ready for serial output
	while (!tud_cdc_connected())
		sleep_ms(10);

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

	ov2640_init_capture(&config, full_frame_capturing);
	printf("Camera streaming initialized\n");

	uint64_t start_time = time_us_64();
	uint16_t frame_count = 0;
	uint16_t missed_rows = 0;
	uint16_t prev_row = current_row;
	received_chunks = 0;

	while (true)
	{
		gpio_put(PIN_LED, !gpio_get(PIN_LED));

		if (full_frame_capturing)
		{
			while (gpio_get(config.pin_vsync) == true);
			while (gpio_get(config.pin_vsync) == false);

			send_data(config.image_buf, config.image_buf_size);
			++frame_count;
		}
		else
		{
			if (frame_ready_buf)
			{
				uint8_t* process_buf = NULL;
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
							transfer_data.pixel_data_size = len;
							transfer_data.pixel_data = process_buf;
							transfer_data.ready = true;
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
				current_row = (current_row + 1) % FRAME_HEIGHT;
			}
			else
			if (gpio_get(config.pin_vsync) == false)
			{
				// Period between frames data. No image data is coming now.
				current_row = 0;
			}

			++current_frame;
		}

		if (time_us_64() - start_time > 1 * 1000 * 1000)
		{
			if (full_frame_capturing)
			{
				printf("FPS: %u\n", frame_count);
			}
			else
			{
				printf("FPS: %u\n", received_chunks / FRAME_HEIGHT);
				printf("Obtained chunks: %u\n", received_chunks);
				printf("Missed rows: %u\n", missed_rows);
			}

			start_time = time_us_64();
			frame_count = 0;
			missed_rows = 0;
			received_chunks = 0;
		}
	}

	return 0;
}
