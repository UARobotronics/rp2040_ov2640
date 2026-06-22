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

#include "common.h"
#include "ethernet.h"
#include "ov2640.h"
#include "transfer.h"

enum pixel_format_e
{
	PIXEL_FORMAT_RGB,
	PIXEL_FORMAT_YUV,
};

enum color_depth_e
{
	COLOR_DEPTH_8_BYTES = 0,
	COLOR_DEPTH_7_BYTES = 1,
	COLOR_DEPTH_6_BYTES = 2,
	COLOR_DEPTH_5_BYTES = 3,
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
	uint8_t* pixel_data;
} transfer_data_t;

#define TRANSFER_BUFFER_LENGTH 1400

typedef struct
{
	uint16_t length;
	uint8_t data[TRANSFER_BUFFER_LENGTH];
} process_data_t;

transfer_data_t transfer_data = {
	.ready = false,
	.pixel_data_size = 0,
	.pixel_data = NULL
};

const int PIN_LED = 25;

// const int PIN_CAM_SIOC = 21; // I2C0 SCL
// const int PIN_CAM_SIOD = 20; // I2C0 SDA
// const int PIN_CAM_RESETB = 2;
// const int PIN_CAM_XCLK = 3;
// const int PIN_CAM_VSYNC = 14;
// const int PIN_CAM_Y2_PIO_BASE = 4;
// #define I2C i2c0

const int PIN_CAM_SIOC = 27; // I2C0 SCL
const int PIN_CAM_SIOD = 26; // I2C0 SDA
const int PIN_CAM_RESETB = 22;
const int PIN_CAM_XCLK = 0;
const int PIN_CAM_VSYNC = 15;
const int PIN_CAM_Y2_PIO_BASE = 5;
#define I2C i2c1

const uint8_t CMD_REG_WRITE = 0xAA;
const uint8_t CMD_REG_READ = 0xBB;
const uint8_t CMD_CAPTURE = 0xCC;

const int PACKET_HEADER_SIZE = 4;

uint8_t image_buf[FRAME_WIDTH * SEND_FRAME_HEIGHT * BYTES_PER_PIXEL]; // YUV422 format

volatile uint8_t active_buffer = 0;
uint8_t active_process_buffer = 0;
uint8_t frame_buffers[BUFFERS_COUNT][CHUNK_SIZE];
process_data_t process_buffers[BUFFERS_COUNT] = {
	{.length = 0, .data = {}},
	{.length = 0, .data = {}},
	{.length = 0, .data = {}},
	{.length = 0, .data = {}}
};

uint8_t* frame_ready_buf = NULL;

// uint8_t* transfer_buf = NULL;

extern volatile uint16_t chunk_count;

volatile uint16_t sent_packets = 0;
volatile uint16_t current_row = 0;
uint16_t current_frame = 0;
uint16_t received_rows = 0;

// Video stream settings
enum transfer_type_s transfer_type = TRANSFER_ETHERNET;
enum pixel_format_e pixel_format = PIXEL_FORMAT_YUV;
uint8_t pixel_encoding_flags = 0;
uint8_t skip_rows = 0;
enum color_depth_e e_color_depth = COLOR_DEPTH_5_BYTES;
bool full_frame_capturing = false;

volatile uint64_t total_send_micros = 0;

// return : how many consecutive pixels that have the same color can be
//          packed into single byte with their color value.
uint8_t max_pixel_count_by_color_depth(uint8_t color_depth)
{
	switch (color_depth)
	{
		case COLOR_DEPTH_8_BYTES : return 0;
		case COLOR_DEPTH_7_BYTES : return 1;
		case COLOR_DEPTH_6_BYTES : return 3;
		case COLOR_DEPTH_5_BYTES : return 7;
	}
}

enum color_depth_e get_next_color_depth(enum color_depth_e depth)
{
	switch (depth)
	{
		case COLOR_DEPTH_5_BYTES : return COLOR_DEPTH_6_BYTES;
		case COLOR_DEPTH_6_BYTES : return COLOR_DEPTH_7_BYTES;
		case COLOR_DEPTH_7_BYTES : return COLOR_DEPTH_8_BYTES;
		case COLOR_DEPTH_8_BYTES : return COLOR_DEPTH_5_BYTES;
	}
}

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
			total_send_micros += end - start;
			++sent_packets;
		}
	}
}

uint16_t prepare_yuv_packet(const uint8_t* in_buf, uint16_t in_len, uint8_t* out_buf, bool header)
{
	uint16_t out_len = 0;
	
	if (header)
	{
		memset(out_buf, 0, PACKET_HEADER_SIZE);
		// HEADER FORMAT
		//
		// 13 bits for frame number.
		// 11 bits for row number.
		// 3 bits for flags - reserved and unused for now.
		// 3 bits for how many rows are skipped every frame, i.e.
		//    send every n-th row in the frame.
		// 2 bits for pixel format:
		//    0 - 5 bits of color data
		//    1 - 6 bits of color data
		//    2 - 7 bits of color data
		//    3 - 8 bits of color data

		uint8_t color_depth = e_color_depth;

		// Create packet header first
		// 13 bits for frame number
		out_buf[0] = (current_frame >> 5) & 0xFF;
		out_buf[1] = (current_frame & 0x1F) << 3;
		// 11 bits for row number
		out_buf[1] |= (current_row >> 8) & 0x7;
		out_buf[2] = current_row & 0xFF;
		// 3 bits for skip rows count
		out_buf[3] |= (skip_rows & 0x7) << 2;
		// 2 bits for pixel format
		// 0 - 8 bits of color data
		// 1 - 7 bits of color data
		// 2 - 6 bits of color data
		// 3 - 5 bits of color data
		out_buf[3] |= color_depth & 0x3;

		out_len = 4;
	}

	// Data packing
	uint32_t val = in_buf[0];
	uint32_t count = 0;
	uint8_t tmp;
	uint8_t max_count = max_pixel_count_by_color_depth(e_color_depth);

	for (int i = 1; i < in_len; i += 1) {
		tmp = in_buf[i];
		// Count is compared against 3 because in video protocol
		// 6 bits of byte are used for color coding and remaining 2 bits
		// are used for pixel count for which this color is applied.
		// 2 bits give 3 possible values.
		// If color depth is changed, this counter must change as well.
		if ((count < max_count) && (tmp == val))
		{
			++count;
		}
		else
		{
			out_buf[out_len] = val + count;
			++out_len;
			val = tmp;
			count = 0;
		}
	}

	out_buf[out_len] = val + count;
	++out_len;

	// memcpy(&out_buf[4], in_buf, in_len);
	// out_len += in_len;

	return out_len;
}

void test_ethernet_transfer()
{
	uint64_t start = time_us_64();
	uint64_t bytes_sent = 0;
	uint32_t buf_len = 1000;
	uint8_t buffer[buf_len];
	uint32_t packet_count = 1000;
	for (int i = 0; i < packet_count; ++i)
	{
		send_data(buffer, buf_len);
		bytes_sent += buf_len;
	}

	uint64_t end = time_us_64();

	printf("Sent %llu bytes of data\n", bytes_sent);
	printf("Total time in micros: %llu\n", end - start);
}

int main()
{
	stdio_init_all();
	uart_set_baudrate(uart0, 460800);

	// Wait until USB is ready for serial output
	// while (!tud_cdc_connected())
	// 	sleep_ms(10);

	printf("\n\nBooted!\n");

	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);

	ov2640_config config;
	config.sccb = I2C;
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
	config.frame_buf_size = sizeof(frame_buffers[0]);

	if (transfer_type == TRANSFER_ETHERNET)
	{
		bool success = ethernet_init();
		if (!success)
		{
			printf("Ethernet init failed!\n");

			return 1;
		}
	}
	
	sleep_ms(1000);
	
	ov2640_init(&config);
	printf("Camera initialized\n");

	// test_ethernet_transfer();

	multicore_launch_core1(core1_main);

	ov2640_init_capture(&config, full_frame_capturing);
	printf("Camera streaming initialized\n");

	uint64_t start_time = time_us_64();
	uint16_t frame_count = 0;
	uint16_t missed_rows = 0;
	uint16_t prev_row = current_row;
	sent_packets = 0;
	uint64_t row_time = 0;
	uint64_t prev_row_time = 0;

	memset(image_buf, 0, sizeof(image_buf));

	bool do_process = true;
	absolute_time_t new_frame_time = 0;
	uint64_t total_receive_micros = 0;
	uint64_t total_process_micros = 0;
	uint64_t total_send_len = 0;
	uint64_t row_count = 0;

	// Counters for rolling color depth and skip rows settings
	static uint8_t color_depth_counter = 0;
	static uint8_t skip_rows_counter = 0;

	while (true)
	{
		gpio_put(PIN_LED, !gpio_get(PIN_LED));

		if (full_frame_capturing)
		{
			if (do_process)
			{
				if (time_reached(new_frame_time))
				{
					// Wait for start of the new frame
					while (gpio_get(config.pin_vsync) == true);
					while (gpio_get(config.pin_vsync) == false);

					size_t buf_start = 0;
					uint8_t div = 2;
					uint32_t row_len = FRAME_WIDTH * BYTES_PER_PIXEL / div; 
					current_row = 0;
					while (buf_start < config.image_buf_size)
					{
						uint16_t len = prepare_yuv_packet(config.image_buf + buf_start, row_len, process_buffers[0].data, true);
						send_data(process_buffers[0].data, len);
						sleep_ms(100);
						buf_start += row_len * div;
						++current_row;
					}

					++frame_count;
					new_frame_time = make_timeout_time_ms(2000);
					// do_process = false;
				}
			}
		}
		else
		{
			if (frame_ready_buf)
			{
				++received_rows;

				if (do_process)
				{
					if (current_row == 0)
						++current_frame;

					if ((current_frame + current_row) % (skip_rows + 1) == 0)
					{
						process_data_t* process_buf = &process_buffers[active_process_buffer];

						row_time = time_us_64();
						if (prev_row_time > 0)
						{
							//printf("Micros passed since prev row data: %llu\n", row_time - prev_row_time);
							total_receive_micros += row_time - prev_row_time;
							row_count++;
						}
						prev_row_time = row_time;

						switch (pixel_format)
						{
							case PIXEL_FORMAT_YUV:
								uint64_t start = time_us_64();
								bool ready_to_send = true;
								// bool ready_to_send = (process_buf->length > 500);
								bool first_chunk = (process_buf->length == 0);
								uint16_t len = prepare_yuv_packet(frame_ready_buf, CHUNK_SIZE, &process_buf->data[process_buf->length], first_chunk);
								process_buf->length += len;

								if (ready_to_send)
								{
									active_process_buffer = (active_process_buffer + 1) % BUFFERS_COUNT;
									process_buffers[active_process_buffer].length = 0;
									
									transfer_data.pixel_data = process_buf->data;
									transfer_data.pixel_data_size = process_buf->length;

									transfer_data.ready = true;
								}
								uint64_t end = time_us_64();
								total_send_len += len;
								total_process_micros += end - start;

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
			else
			if (gpio_get(config.pin_vsync) == false)
			{
				// Period between frames data. No image data is coming now.

				// Underflow is intentional. On first increment current row must be 0.
				current_row = -1;
			}
		}

		if (time_us_64() - start_time > 1 * 1000 * 1000)
		{
			if (color_depth_counter++ >= 9)
			{
				color_depth_counter = 0;
				//e_color_depth = get_next_color_depth(e_color_depth);
			}

			if (skip_rows_counter++ >= 9)
			{
				skip_rows_counter = 0;
				// skip_rows = (skip_rows + 1) % 2;
			}

			if (full_frame_capturing)
			{
				printf("FPS: %u\n", frame_count);
			}
			else
			{
				uint64_t receive_micros = total_receive_micros / row_count;
				uint64_t process_micros = total_process_micros / row_count;
				uint64_t send_len = total_send_len / row_count;
				uint64_t send_micros = total_send_micros / sent_packets;

				printf("FPS: %u\n", received_rows / FRAME_HEIGHT);
				printf("Obtained rows: %u\n", received_rows);
				printf("Missed rows: %u\n", missed_rows);
				printf("Retrieved single row data in %llu micros\n", receive_micros);
				printf("Processed single row data in %llu micros\n", process_micros);
				printf("Sent single row data in %llu micros\n", send_micros);
				printf("Average udp packet length is %llu bytes\n", send_len);

				total_receive_micros = 0;
				total_process_micros = 0;
				total_send_micros = 0;
				total_send_len = 0;

				row_count = 0;
			}

			start_time = time_us_64();
			frame_count = 0;
			missed_rows = 0;
			sent_packets = 0;
			received_rows = 0;
		}
	}

	return 0;
}
