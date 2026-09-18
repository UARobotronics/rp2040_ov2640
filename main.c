#include <stdio.h>
#include <string.h>
#include "tusb.h"

#include "pico/multicore.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"

#include "network/drivers/w5500.h"
#include "network/network.h"

#include "command.h"
#include "common.h"
#include "ethernet.h"
#include "misc.h"
#include "ov2640.h"
#include "transfer.h"

#define TRANSFER_BUFFER_LENGTH 1400

enum pixel_format_e
{
	PIXEL_FORMAT_RGB,
	PIXEL_FORMAT_YUV,
};

enum processing_state_e
{
	STATE_WAIT_FOR_CLIENT = 0,
	STATE_SEND_ACK_TO_CLIENT = 1,
	STATE_SEND_STREAM_PARAMS_TO_CLIENT = 2,
	STATE_STREAM_VIDEO = 3,
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

typedef struct
{
	uint16_t length;
	uint8_t data[TRANSFER_BUFFER_LENGTH];
} process_data_t;

typedef struct
{
	uint8_t msb;
	uint8_t lsb;
	bool valid;
} fps_line_adjust_values_t;

const int PIN_LED = 25;

const int PIN_CAM_SIOC = 27; // I2C0 SCL
const int PIN_CAM_SIOD = 26; // I2C0 SDA
const int PIN_CAM_RESETB = 22;
const int PIN_CAM_XCLK = 0;
const int PIN_CAM_VSYNC = 15;
const int PIN_CAM_Y2_PIO_BASE = 5;
#define I2C i2c1

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

transfer_data_t transfer_data = {
	.ready = false,
	.pixel_data_size = 0,
	.pixel_data = NULL
};

fps_line_adjust_values_t fps_line_adjust_values[MAX_FPS+1];
uint8_t min_target_fps;
uint8_t max_target_fps;

// Current state variables
uint8_t* frame_ready_buf = NULL;

volatile uint16_t current_row = 0;
mutex_t socket_mutex;
uint16_t current_frame = 0;
enum processing_state_e state = STATE_WAIT_FOR_CLIENT;

// Video stream settings
enum transfer_type_s transfer_type = TRANSFER_ETHERNET;
enum pixel_format_e pixel_format = PIXEL_FORMAT_YUV;
enum color_depth_e color_depth = COLOR_DEPTH_5_BITS;
uint8_t skip_rows = 0;
uint16_t min_packet_size = 0;
uint8_t target_fps = 0;
bool full_frame_capturing = false;

ov2640_config config;
volatile stats_t stats;

void reset_stats(volatile stats_t* st)
{
	st->total_send_micros = 0;
	st->total_packet_prepare_micros = 0;
	st->total_free_tx_micros = 0;
	st->total_spi_send_micros = 0;
	st->total_core_spi_send_micros = 0;
	st->total_packet_finalize_micros = 0;
	st->total_spi_send_count = 0;
	st->total_packet_attempt_send_count = 0;
	st->total_receive_micros = 0;
	st->total_process_communication_micros = 0;
	st->total_process_micros = 0;
	st->total_send_len = 0;
	st->processed_rows = 0;
	st->frame_count = 0;
    st->received_rows = 0;
    st->sent_packets = 0;
}

// return : how many consecutive pixels that have the same color can be
//          packed into single byte with their color value.
uint8_t max_pixel_count_by_color_depth(uint8_t color_depth)
{
	switch (color_depth)
	{
		case COLOR_DEPTH_8_BITS : return 0;
		case COLOR_DEPTH_7_BITS : return 1;
		case COLOR_DEPTH_6_BITS : return 3;
		case COLOR_DEPTH_5_BITS : return 7;
		case COLOR_DEPTH_4_BITS : return 15;
		case COLOR_DEPTH_3_BITS : return 31;
		case COLOR_DEPTH_2_BITS : return 63;
		case COLOR_DEPTH_1_BITS : return 127;
		default : return 0;
	}
}

void fill_fps_data()
{
	uint8_t reg_to_fps_map[MAX_LINE_ADJUSTMENT_MSB_REG_VALUE+1];

	uint8_t old_msb = ov2640_get_line_interval_adjust_msb(&config);

	for (int val = 0; val <= MAX_LINE_ADJUSTMENT_MSB_REG_VALUE; ++val)
	{
		ov2640_set_line_interval_adjust_msb(&config, val);
		while (gpio_get(config.pin_vsync) == false);
		while (gpio_get(config.pin_vsync) == true);

		uint64_t start = time_us_64();
		while (gpio_get(config.pin_vsync) == false);
		while (gpio_get(config.pin_vsync) == true);
		uint64_t end = time_us_64();
		reg_to_fps_map[val] = 1000000 / (end - start);
	}

	// reset to initial state
	ov2640_set_line_interval_adjust_msb(&config, old_msb);

	min_target_fps = reg_to_fps_map[MAX_LINE_ADJUSTMENT_MSB_REG_VALUE];
	max_target_fps = reg_to_fps_map[0];

	for (int i = 0; i <= MAX_FPS; ++i)
		fps_line_adjust_values[i].valid = false;

	uint8_t msb = 0;
	uint8_t fps = reg_to_fps_map[msb];

	while (msb <= MAX_LINE_ADJUSTMENT_MSB_REG_VALUE)
	{
		if (fps == reg_to_fps_map[msb])
		{
			uint8_t low_msb = msb;
			while ((msb <= MAX_LINE_ADJUSTMENT_MSB_REG_VALUE) && (reg_to_fps_map[msb] == fps))
				++msb;

			fps_line_adjust_values[fps].valid = true;
			fps_line_adjust_values[fps].msb = (msb - 1 + low_msb) / 2;
			fps_line_adjust_values[fps].lsb = 0;
		}
		else
		{
			if (fps > reg_to_fps_map[msb])
			{
				uint8_t max_fps = reg_to_fps_map[msb-1];
				uint8_t min_fps = reg_to_fps_map[msb];

				fps_line_adjust_values[fps].valid = true;
				fps_line_adjust_values[fps].msb = msb-1;
				fps_line_adjust_values[fps].lsb = 255 / (max_fps - min_fps);
			}
		}

		--fps;
	}
}

void core1_main()
{
	while (true)
	{
		if (transfer_data.ready)
		{
			if (mutex_try_enter(&socket_mutex, NULL))
			{
				transfer_data.ready = false;
				uint64_t start = time_us_64();
				send_data(transfer_data.pixel_data, transfer_data.pixel_data_size);
				uint64_t end = time_us_64();
				// PRINTF("Sent row data in %llu micros\n", end - start);
				stats.total_send_micros += end - start;
				++stats.sent_packets;
				mutex_exit(&socket_mutex);
			}
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

		// Create packet header first
		// 13 bits for frame number
		out_buf[0] = (current_frame >> 5) & 0xFF;
		out_buf[1] = (current_frame & 0x1F) << 3;
		// 11 bits for row number
		out_buf[1] |= (current_row >> 8) & 0x7;
		out_buf[2] = current_row & 0xFF;
		// 3 bits for skip rows count
		out_buf[3] |= (skip_rows & 0x7) << 3;
		// 2 bits for pixel format
		// 0 - 8 bits of color data
		// 1 - 7 bits of color data
		// 2 - 6 bits of color data
		// 3 - 5 bits of color data
		out_buf[3] |= color_depth & 0x7;

		out_len = 4;
	}

	// Data packing
	uint32_t val = in_buf[0];
	uint32_t count = 0;
	uint8_t tmp;
	uint8_t max_count = max_pixel_count_by_color_depth(color_depth);

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

	PRINTF("Sent %llu bytes of data\n", bytes_sent);
	PRINTF("Total time in micros: %llu\n", end - start);
}

void send_command_ack_to_client()
{
	uint8_t buf[MAX_COMMAND_LENGTH];
	buf[0] = CMD_SEND_ACK_TO_CLIENT;

	send_data(buf, CMD_SEND_ACK_TO_CLIENT_SIZE);
}

void send_command_stream_params_to_client()
{
	uint8_t buf[MAX_COMMAND_LENGTH];
	buf[0] = CMD_SEND_STREAM_PARAMS_TO_CLIENT;
	buf[1] = color_depth;
	buf[2] = skip_rows;
	buf[3] = min_packet_size >> 8;
	buf[4] = min_packet_size & 0xFF;
	buf[5] = target_fps;

	send_data(buf, CMD_SEND_STREAM_PARAMS_TO_CLIENT_SIZE);
}

void set_target_fps(uint8_t fps)
{
	if ((fps >= 0) && (fps <= MAX_FPS))
	{
		// FPS equal to 0 is valid and means that there is no target FPS
		// and it must be as high as possible
		if ((fps != 0) && !fps_line_adjust_values[fps].valid)
		{
			if (fps < min_target_fps)
				fps = min_target_fps;
			else if (fps > max_target_fps)
				fps = max_target_fps;
		}

		target_fps = fps;

		if (fps == 0)
		{
			ov2640_set_line_interval_adjust_msb(&config, 0);
			ov2640_set_line_interval_adjust_lsb(&config, 0);
		}
		else
		{
			ov2640_set_line_interval_adjust_msb(&config, fps_line_adjust_values[fps].msb);
			ov2640_set_line_interval_adjust_lsb(&config, fps_line_adjust_values[fps].lsb);
		}
	}
}

void process_communication()
{
    if (!mutex_try_enter(&socket_mutex, NULL))
        return;

	uint8_t buf_command[MAX_COMMAND_LENGTH];
	uint16_t size;

	switch (state)
	{
		case STATE_WAIT_FOR_CLIENT :
			size = receive_data(buf_command);

			if (size != CMD_CONNECT_TO_SERVER_SIZE)
				break;

			if (buf_command[0] == CMD_CONNECT_TO_SERVER)
				state = STATE_SEND_ACK_TO_CLIENT;

			break;
		case STATE_SEND_ACK_TO_CLIENT:
			send_command_ack_to_client();
			sleep_ms(500);

			state = STATE_SEND_STREAM_PARAMS_TO_CLIENT;

			break;
		case STATE_SEND_STREAM_PARAMS_TO_CLIENT:
			send_command_stream_params_to_client();
			sleep_ms(200);

			state = STATE_STREAM_VIDEO;

			break;
		case STATE_STREAM_VIDEO:
			size = receive_data(buf_command);
			if (size == 0)
				break;

			switch (buf_command[0])
			{
				case CMD_SEND_COLOR_DEPTH_TO_SERVER:
					if (size != CMD_SEND_COLOR_DEPTH_TO_SERVER_SIZE)
						break;

					ov2640_switch_color_depth(&config, &color_depth, buf_command[1]);

					break;
				case CMD_SEND_SKIP_ROWS_TO_SERVER:
					if (size != CMD_SEND_SKIP_ROWS_TO_SERVER_SIZE)
						break;

					skip_rows = buf_command[1];

					break;
				case CMD_SEND_MIN_PACKET_SIZE_TO_SERVER:
					if (size != CMD_SEND_MIN_PACKET_SIZE_TO_SERVER_SIZE)
						break;

					min_packet_size = ((uint16_t)(buf_command[1]) << 8) + buf_command[2];

					break;
				case CMD_SEND_TARGET_FPS_TO_SERVER:
					if (size != CMD_SEND_TARGET_FPS_TO_SERVER_SIZE)
						break;

					set_target_fps((uint8_t)(buf_command[1]));

					break;
				case CMD_CONNECT_TO_SERVER:
					if (size != CMD_CONNECT_TO_SERVER_SIZE)
						break;

					if (buf_command[0] == CMD_CONNECT_TO_SERVER)
					{
						ov2640_stop_dma_channel(&config);
						state = STATE_SEND_ACK_TO_CLIENT;
					}

					break;
				default:
					break;
			}

			break;
		default:
			break;
	}

	mutex_exit(&socket_mutex);
}

int main()
{
	stdio_init_all();
	uart_set_baudrate(uart0, 460800);

	// Wait until USB is ready for serial output
	// while (!tud_cdc_connected())
	// 	sleep_ms(10);

	PRINTF("\n\nBooted!\n");

	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);

	config.sccb = I2C;
	config.pin_sioc = PIN_CAM_SIOC;
	config.pin_siod = PIN_CAM_SIOD;

	config.pin_resetb = PIN_CAM_RESETB;
	config.pin_xclk = PIN_CAM_XCLK;
	config.pin_vsync = PIN_CAM_VSYNC;
	config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

	config.pio = get_pio_by_color_depth(color_depth);
	config.pio_sm = get_pio_sm_by_color_depth(color_depth);

	config.dma_channel = get_dma_channel_by_color_depth(color_depth);
	config.image_buf = image_buf;
	config.image_buf_size = sizeof(image_buf);
	config.frame_buf_size = sizeof(frame_buffers[0]);

	sleep_ms(2000);
	
	if (transfer_type == TRANSFER_ETHERNET)
	{
		bool success = ethernet_init();
		if (!success)
		{
			PRINTF("Ethernet init failed!\n");

			return 1;
		}
	}
	
	sleep_ms(1000);
	
	ov2640_init(&config);
	PRINTF("Camera initialized\n");

	uint64_t start_time = time_us_64();
	fill_fps_data();
	uint64_t end_time = time_us_64();
	PRINTF("FPS line adjustment processed in %llu millis\n", (end_time - start_time) / 1000);

	// test_ethernet_transfer();

	mutex_init(&socket_mutex);
	multicore_launch_core1(core1_main);

	ov2640_init_capture(&config, full_frame_capturing);
	PRINTF("Camera streaming initialized\n");

	start_time = time_us_64();
	uint16_t frame_count = 0;
	uint16_t missed_rows = 0;
	uint16_t prev_row = current_row;
	uint64_t row_time = 0;
	uint64_t prev_row_time = 0;

	memset(image_buf, 0, sizeof(image_buf));
	reset_stats(&stats);

	absolute_time_t next_frame_time = 0;
	absolute_time_t last_normal_frame_time = time_us_64();

	while (true)
	{
		gpio_put(PIN_LED, !gpio_get(PIN_LED));

		if (!full_frame_capturing && (gpio_get(config.pin_vsync) == false))
		{
			// Period between frames data. No image data is coming now.
			// Let's do some useful stuff!

			// Underflow is intentional. After first increment current row must become 0.
			current_row = -1;

			io_rw_32 transfer_count = dma_channel_get_transfer_count(config.dma_channel);

			// Check if frame is fractured and left part of the frame appears
			// to the right
			if (transfer_count < FRAME_WIDTH - CRITICAL_FRAME_SHIFT)
			{
				if (time_us_64() - last_normal_frame_time > FRACTURED_FRAME_TIME_MS * 1000)
				{
					PRINTF("Frame is fractured, resetting DMA channel...\n");

					ov2640_restart_dma_channel(&config);
					last_normal_frame_time = time_us_64();
				}
			}
			else
			{
				last_normal_frame_time = time_us_64();
			}

			++stats.frame_count;
			
			uint64_t start = time_us_64();
			process_communication();
			stats.total_process_communication_micros += time_us_64() - start;
		}
		else if (state == STATE_STREAM_VIDEO)
		{
			if (full_frame_capturing)
			{
				if (time_reached(next_frame_time))
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
					next_frame_time = make_timeout_time_ms(2000);
					// do_process = false;
				}
			}
			else
			{
				if (frame_ready_buf)
				{
					++stats.received_rows;

					if (current_row == 0)
						++current_frame;

					if ((current_frame + current_row) % (skip_rows + 1) == 0)
					{
						process_data_t* process_buf = &process_buffers[active_process_buffer];

						row_time = time_us_64();
						if (prev_row_time > 0)
						{
							//PRINTF("Micros passed since prev row data: %llu\n", row_time - prev_row_time);
							stats.total_receive_micros += row_time - prev_row_time;
							stats.processed_rows++;
						}
						prev_row_time = row_time;

						switch (pixel_format)
						{
							case PIXEL_FORMAT_YUV:
								uint64_t start = time_us_64();
								bool first_chunk = (process_buf->length == 0);
								uint16_t len = prepare_yuv_packet(frame_ready_buf, CHUNK_SIZE, &process_buf->data[process_buf->length], first_chunk);
								process_buf->length += len;

								if (process_buf->length >= min_packet_size)
								{
									active_process_buffer = (active_process_buffer + 1) % BUFFERS_COUNT;
									process_buffers[active_process_buffer].length = 0;
									
									transfer_data.pixel_data = process_buf->data;
									transfer_data.pixel_data_size = process_buf->length;

									transfer_data.ready = true;
								}
								uint64_t end = time_us_64();
								stats.total_send_len += process_buf->length;
								stats.total_process_micros += end - start;

								// PRINTF("Processed row data in %llu micros\n", end - start);
								// PRINTF("Processed buffer length for %u row: %u\n", current_row, len);
								// PRINTF("Received %u row data\n", current_row);

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

			if (time_us_64() - start_time > 1 * 1000 * 1000)
			{
				if (full_frame_capturing)
				{
					PRINTF("FPS: %u\n", frame_count);
				}
				else
				{
					uint64_t receive_micros = stats.total_receive_micros / stats.processed_rows;
					uint64_t process_micros = stats.total_process_micros / stats.processed_rows;
					uint64_t send_len = stats.total_send_len / stats.processed_rows;
					uint64_t send_micros = stats.total_send_micros / stats.sent_packets;
					uint64_t packet_prepare_micros = stats.total_packet_prepare_micros / stats.total_spi_send_count;
					uint64_t packet_free_tx_micros = stats.total_free_tx_micros / stats.total_spi_send_count;
					uint64_t spi_send_micros = stats.total_spi_send_micros / stats.total_spi_send_count;
					uint64_t core_spi_send_micros = stats.total_core_spi_send_micros / stats.total_spi_send_count;
					uint64_t packet_finalize_micros = stats.total_packet_finalize_micros / stats.total_spi_send_count;
					uint64_t process_communication_micros = stats.total_process_communication_micros / stats.frame_count;
					uint16_t frame_count = stats.received_rows / FRAME_HEIGHT;

					PRINTF("FPS: %u\n", frame_count);
					PRINTF("Received rows from camera: %u\n", stats.received_rows);
					PRINTF("Missed rows: %u\n", missed_rows);
					PRINTF("Sent packets: %u\n", stats.sent_packets);
					PRINTF("Retrieved single row data in %llu micros\n", receive_micros);
					PRINTF("Processed single row data in %llu micros\n", process_micros);
					PRINTF("\n");
					PRINTF("Sent single packet data in %llu micros\n", send_micros);
					PRINTF("Single packet prepare in %llu micros\n", packet_prepare_micros);
					PRINTF("Single packet free TX buffer in %llu micros\n", packet_free_tx_micros);
					PRINTF("Single packet data SPI transfer done in %llu micros\n", spi_send_micros);
					PRINTF("Single packet data core SPI transfer done in %llu micros\n", core_spi_send_micros);
					PRINTF("Single packet finalize in %llu micros\n", packet_finalize_micros);
					PRINTF("Network communicatoin handled in %llu micros\n", process_communication_micros);
					PRINTF("\n");
					PRINTF("Average udp packet length is %llu bytes\n", send_len);
					PRINTF("Packet count (attempt\\sending) is %u\\%u\n", stats.total_packet_attempt_send_count, stats.total_spi_send_count);
				}

				reset_stats(&stats);

				start_time = time_us_64();
				frame_count = 0;
				missed_rows = 0;
			}
		}
	}

	return 0;
}
