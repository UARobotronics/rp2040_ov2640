#include "ethernet.h"

#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include "network/network.h"
#include "network/drivers/w5500.h"

#define ETHERNET_W5500_MISO_GPIO					16
#define ETHERNET_W5500_MOSI_GPIO					19
#define ETHERNET_W5500_SCLK_GPIO					18
#define ETHERNET_W5500_CS_GPIO						17

#define ETHERNET_W5500_RESET_GPIO					22

static struct ETHERNET_INTERFACE_s _ethernetInterface = { 0 };

struct __attribute__((packed)) CONFIG_s {
	struct MAC_ADDRESS_s mac;
	struct IPv4_ADDRESS_s ip;
	struct IPv4_NETMASK_s mask;
	struct IPv4_ADDRESS_s gw;
	uint16_t port;
	uint8_t crc;
};

static struct CONFIG_s _config = {
	.mac = 	{ .address = { 0x00, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE } },

	.ip = 	{ .a = 192, .b = 168, .c = 50, .d = 122 },
	.gw = 	{ .a = 192, .b = 168, .c = 50, .d = 001 },
	.mask = { .a = 255, .b = 255, .c = 255, .d = 000 },

	.port = 6666,
};

static struct IPv4_ADDRESS_s dst_address = {.a = 192, .b = 168, .c = 50, .d = 12};
static uint16_t dst_port = 8080;
static uint16_t max_packet_size = 1700;
// static uint16_t max_packet_size = 704;

static SOCKET socket = {0};

const uint spi_speed = 60 * 1000 * 1000;

bool ethernet_init()
{
	uint baud = spi_init(spi0, spi_speed);

	gpio_set_function(ETHERNET_W5500_MOSI_GPIO, GPIO_FUNC_SPI);
	gpio_set_function(ETHERNET_W5500_MISO_GPIO, GPIO_FUNC_SPI);
	gpio_set_function(ETHERNET_W5500_SCLK_GPIO, GPIO_FUNC_SPI);

	if(!w5500_network_init(&_ethernetInterface, spi0, ETHERNET_W5500_CS_GPIO, ETHERNET_W5500_RESET_GPIO)) {
		printf("w5500_network_init - FAILED\n");

        return false;
	}

	if(!ethernet_set_mac_address(&_ethernetInterface, &_config.mac)) {
		printf("ethernet_set_mac_address - FAILED\n");

        return false;
	}

	if(!ethernet_set_ipv4_address(&_ethernetInterface, &_config.ip, &_config.mask, &_config.gw)) {
		printf("ethernet_set_ipv4_address - FAILED\n");

        return false;
	}

	if(!ethernet_print_info(&_ethernetInterface)) {
		printf("ethernet_print_info - FAILED\n");
		
        return false;
	}

	if(!network_socket(&_ethernetInterface, &socket, SOCKET_PROTOCOL_UDP, 0)) {
		printf("network_socket - FAILED\n");

        return false;
	}

	if(!network_socket_bind(&_ethernetInterface, &socket, &_config.ip, _config.port)) {
		printf("network_socket_bind - FAILED\n");

        return false;
	}

    return true;
}

bool ethernet_send_udp(uint8_t* data, uint32_t data_len)
{
	uint32_t shift = 0;
	while (shift < data_len)
	{
		uint32_t left_len = data_len - shift;
		uint16_t size = (left_len < max_packet_size) ? left_len : max_packet_size;
		network_socket_sendto(&_ethernetInterface, &socket, data+shift, size, &dst_address, dst_port);
		//sleep_ms(5);

		shift += size;
	}
}