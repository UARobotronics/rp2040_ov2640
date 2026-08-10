#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdint.h>
#include <stdbool.h>

typedef void* SOCKET;

typedef enum {
	SOCKET_PROTOCOL_TCP,
	SOCKET_PROTOCOL_UDP,
	SOCKET_PROTOCOL_IPRAW
} socket_protocol_t;

typedef enum {
	SOCKET_STATE_CLOSED = 0x00,
	SOCKET_STATE_READY_AS_UDP = 0x22,
	SOCKET_STATE_INIT = 0x13,
	SOCKET_STATE_LISTEN = 0x14,
	SOCKET_STATE_SYNSENT = 0x15,
	SOCKET_STATE_SYNRECV = 0x16,
	SOCKET_STATE_ESTABLISHED = 0x17,
	SOCKET_STATE_FIN_WAIT = 0x18,
	SOCKET_STATE_CLOSING = 0x1A,
	SOCKET_STATE_TIME_WAIT = 0x1B,
	SOCKET_STATE_CLOSE_WAIT = 0x1C,
	SOCKET_STATE_LAST_ACK = 0x1D
} socket_state_t;

typedef enum {
	SOCKET_ERROR_OK,
	SOCKET_ERROR_BUS_REGISTER,
	SOCKET_ERROR_BUS_DATA,
	SOCKET_ERROR_MODE,
	SOCKET_ERROR_BUFFER_SIZE,
	SOCKET_ERROR_BUFFER_EMPTY,
	SOCKET_ERROR_ADDRESS,
	SOCKET_ERROR_PORT,
	SOCKET_ERROR_CLOSED,
	SOCKET_ERROR_TIMEOUT,
} socket_error_t;

typedef enum {
	SOCKET_OPTION_FRAGMENTATION,
	SOCKET_OPTION_WAKE_ON_LAN,

} socket_option_t;

struct __attribute__((packed)) MAC_ADDRESS_s {
	union {
		uint8_t address[6];
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t c;
			uint8_t d;
			uint8_t e;
			uint8_t f;
		};
	};
};

struct __attribute__((packed)) IPv4_ADDRESS_s {
	union {
		uint32_t ip;
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t c;
			uint8_t d;
		};
	};
};

struct __attribute__((packed)) IPv4_NETMASK_s {
	union {
		uint32_t mask;
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t c;
			uint8_t d;
		};
	};
};

struct __attribute__((packed)) ETHERNET_INTERFACE_s {
	uint8_t private_data[256];

	bool (*reset_hw)(void* private_data);
	bool (*reset_sw)(void* private_data);

	bool (*set_mac_address)(const struct MAC_ADDRESS_s* address, void* private_data);
	bool (*get_mac_address)(struct MAC_ADDRESS_s* address, void* private_data);

	bool (*set_ipv4_address)(const struct IPv4_ADDRESS_s* ip, const struct IPv4_NETMASK_s* mask, const struct IPv4_ADDRESS_s* gw, void* private_data);
	bool (*get_ipv4_address)(struct IPv4_ADDRESS_s* ip, struct IPv4_NETMASK_s* mask, struct IPv4_ADDRESS_s* gw, void* private_data);

	bool (*socket)(SOCKET* socket, socket_protocol_t protocol, uint8_t flags, void* private_data);
	bool (*socket_bind)(SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port, void* private_data);
	bool (*socket_connect)(SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port, uint32_t timeout_ms, void* private_data);
	bool (*socket_close)(SOCKET socket, void* private_data);
	bool (*socket_listen)(SOCKET socket, void* private_data);
	bool (*socket_state)(SOCKET socket, socket_state_t* state, void* private_data);
	bool (*socket_recvfrom)(SOCKET socket, uint8_t* buffer, uint16_t buffer_size, uint16_t* actual_size, struct IPv4_ADDRESS_s* client_address, uint16_t* client_port, void* private_data);
	bool (*socket_sendto)(SOCKET socket, const uint8_t* buffer, uint16_t size, const struct IPv4_ADDRESS_s* address, uint16_t port, void* private_data);

	bool (*socket_rx_buffer_clear)(SOCKET socket, void* private_data);
	bool (*socket_tx_buffer_clear)(SOCKET socket, void* private_data);

	bool (*socket_rx_buffer_max_size)(SOCKET socket, uint16_t* size, void* private_data);
	bool (*socket_tx_buffer_max_size)(SOCKET socket, uint16_t* size, void* private_data);

	bool (*socket_rx_buffer_size)(SOCKET socket, uint16_t* size, void* private_data);
	bool (*socket_tx_buffer_size)(SOCKET socket, uint16_t* size, void* private_data);

	bool (*socket_rx_buffer_free_size)(SOCKET socket, uint16_t* size, void* private_data);
	bool (*socket_tx_buffer_free_size)(SOCKET socket, uint16_t* size, void* private_data);

	bool (*socket_local_ipv4_info)(SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port, void* private_data);
	bool (*socket_remote_ipv4_info)(SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port, void* private_data);

	socket_error_t (*socket_last_error)(SOCKET socket, void* private_data);

	bool (*socket_set_option)(SOCKET socket, socket_option_t option, const void* value, void* private_data);

	bool (*is_initialized)(void* private_data);
};

bool ethernet_set_mac_address(struct ETHERNET_INTERFACE_s* interface, const struct MAC_ADDRESS_s* address);
bool ethernet_get_mac_address(struct ETHERNET_INTERFACE_s* interface, struct MAC_ADDRESS_s* address);

bool ethernet_set_ipv4_address(struct ETHERNET_INTERFACE_s* interface, const struct IPv4_ADDRESS_s* ip, const struct IPv4_NETMASK_s* mask, const struct IPv4_ADDRESS_s* gw);
bool ethernet_get_ipv4_address(struct ETHERNET_INTERFACE_s* interface, struct IPv4_ADDRESS_s* ip, struct IPv4_NETMASK_s* mask, struct IPv4_ADDRESS_s* gw);

bool ethernet_print_info(struct ETHERNET_INTERFACE_s* interface);

bool network_reset_interface_hw(struct ETHERNET_INTERFACE_s* interface);
bool network_reset_interface_sw(struct ETHERNET_INTERFACE_s* interface);

bool network_socket(struct ETHERNET_INTERFACE_s* interface, SOCKET* socket, socket_protocol_t protocol, uint8_t flags);
bool network_socket_bind(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port);
bool network_socket_connect(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port, uint32_t timeout_ms);
bool network_socket_close(struct ETHERNET_INTERFACE_s* interface, SOCKET socket);
bool network_socket_listen(struct ETHERNET_INTERFACE_s* interface, SOCKET socket);
bool network_socket_state(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, socket_state_t* state);

bool network_socket_recvfrom(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint8_t* buffer, uint16_t buffer_length, uint16_t* actual_length, struct IPv4_ADDRESS_s* client_address, uint16_t* client_port);
bool network_socket_sendto(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const uint8_t* buffer, uint16_t size, const struct IPv4_ADDRESS_s* address, uint16_t port);

bool network_socket_local_ipv4_info(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port);
bool network_socket_remote_ipv4_info(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port);

bool network_socket_rx_buffer_clear(struct ETHERNET_INTERFACE_s* interface, SOCKET socket);
bool network_socket_tx_buffer_clear(struct ETHERNET_INTERFACE_s* interface, SOCKET socket);

bool network_socket_rx_buffer_max_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);
bool network_socket_tx_buffer_max_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);

bool network_socket_rx_buffer_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);
bool network_socket_tx_buffer_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);

bool network_socket_rx_buffer_free_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);
bool network_socket_tx_buffer_free_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size);

socket_error_t network_socket_last_error(struct ETHERNET_INTERFACE_s* interface, SOCKET socket);

bool network_socket_set_option(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, socket_option_t option, const void* value);

bool network_is_initialized(struct ETHERNET_INTERFACE_s* interface);

/*
int8_t network_disconnect(SOCKET* socket);
int32_t network_send(SOCKET* socket, uint8_t* buffer, uint16_t length);
int32_t network_recv(SOCKET* socket, uint8_t* buffer, uint16_t length);
*/
#endif
