#ifndef COMMON_H
#define COMMON_H

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 600
#define CRITICAL_FRAME_SHIFT 20
#define FRACTURED_FRAME_TIME_MS 2000
#define SEND_FRAME_HEIGHT 70
#define BYTES_PER_PIXEL 1

#define CHUNK_SIZE FRAME_WIDTH * CHUNK_HEIGHT * BYTES_PER_PIXEL
#define BUFFERS_COUNT 4

// #define PRINTF(...) printf(__VA_ARGS__)
#define PRINTF(...)

typedef struct
{
	uint64_t total_send_micros;
	uint64_t total_packet_prepare_micros;
	uint64_t total_free_tx_micros;
	uint64_t total_spi_send_micros;
	uint64_t total_core_spi_send_micros;
	uint64_t total_packet_finalize_micros;
	uint16_t total_spi_send_count;
	uint16_t total_packet_attempt_send_count;
	uint64_t total_receive_micros;
	uint64_t total_process_micros;
	uint64_t total_send_len;
	uint64_t row_count;
    uint64_t received_rows;
    uint64_t sent_packets;
} stats_t;

#endif // COMMON_H