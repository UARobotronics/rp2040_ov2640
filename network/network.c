#include "network.h"
#include <stdio.h>
#include <assert.h>

bool ethernet_set_mac_address(struct ETHERNET_INTERFACE_s* interface, const struct MAC_ADDRESS_s* address) {
	assert(interface && address && interface->set_mac_address);
	return interface->set_mac_address(address, interface->private_data);
}

bool ethernet_get_mac_address(struct ETHERNET_INTERFACE_s* interface, struct MAC_ADDRESS_s* address) {
	assert(interface && address && interface->get_mac_address);
	return interface->get_mac_address(address, interface->private_data);
}

bool ethernet_set_ipv4_address(struct ETHERNET_INTERFACE_s* interface, const struct IPv4_ADDRESS_s* ip, const struct IPv4_NETMASK_s* mask, const struct IPv4_ADDRESS_s* gw) {
	assert(interface && interface->set_ipv4_address);
	return interface->set_ipv4_address(ip, mask, gw, interface->private_data);
}

bool ethernet_get_ipv4_address(struct ETHERNET_INTERFACE_s* interface, struct IPv4_ADDRESS_s* ip, struct IPv4_NETMASK_s* mask, struct IPv4_ADDRESS_s* gw) {
	assert(interface && interface->get_ipv4_address);
	return interface->get_ipv4_address(ip, mask, gw, interface->private_data);
}

bool ethernet_print_info(struct ETHERNET_INTERFACE_s* interface) {
	assert(interface);

	struct MAC_ADDRESS_s mac;
	if(!ethernet_get_mac_address(interface, &mac)) {
		printf("ethernet_get_mac_address - FAILED\n");
		return false;
	}

	struct IPv4_ADDRESS_s ip = { 0 };
	struct IPv4_NETMASK_s mask = { 0 };
	struct IPv4_ADDRESS_s gw = { 0 };
	if(!ethernet_get_ipv4_address(interface, &ip, &mask, &gw)) {
		printf("ethernet_get_ipv4_address - FAILED\n");
		return false;
	}
	
	printf("MAC    : %02X:%02X:%02X:%02X:%02X:%02X\n", mac.a, mac.b, mac.c, mac.d, mac.e, mac.f);
	printf("IP     : %d.%d.%d.%d\n", ip.a, ip.b, ip.c, ip.d);
	printf("Gateway: %d.%d.%d.%d\n", gw.a, gw.b, gw.c, gw.d);
	printf("Mask   : %d.%d.%d.%d\n", mask.a, mask.b, mask.c, mask.d);
	return true;
}

bool network_reset_interface_sw(struct ETHERNET_INTERFACE_s* interface) {
	assert(interface && interface->reset_sw);
	return interface->reset_sw(interface->private_data);
}

bool network_socket(struct ETHERNET_INTERFACE_s* interface, SOCKET* socket, socket_protocol_t protocol, uint8_t flags) {
	assert(interface && interface->socket);
	return interface->socket(socket, protocol, flags, interface->private_data);
}

bool network_socket_bind(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port) {
	assert(interface && interface->socket_bind);
	return interface->socket_bind(socket, ip, port, interface->private_data);
}

bool network_socket_connect(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const struct IPv4_ADDRESS_s* ip, uint16_t port, uint32_t timeout_ms) {
	assert(interface && interface->socket_connect);
	return interface->socket_connect(socket, ip, port, timeout_ms, interface->private_data);
}

bool network_socket_close(struct ETHERNET_INTERFACE_s* interface, SOCKET socket) {
	assert(interface && interface->socket_close);
	return interface->socket_close(socket, interface->private_data);
}

bool network_socket_listen(struct ETHERNET_INTERFACE_s* interface, SOCKET socket) {
	assert(interface && interface->socket_listen);
	return interface->socket_listen(socket, interface->private_data);
}

bool network_socket_state(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, socket_state_t* state) {
	assert(interface && interface->socket_state);
	return interface->socket_state(socket, state, interface->private_data);
}

bool network_socket_recvfrom(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint8_t* buffer, uint16_t buffer_size, uint16_t* actual_size, struct IPv4_ADDRESS_s* client_address, uint16_t* client_port) {
	assert(interface && interface->socket_recvfrom);
	return interface->socket_recvfrom(socket, buffer, buffer_size, actual_size, client_address, client_port, interface->private_data);
}

bool network_socket_sendto(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, const uint8_t* buffer, uint16_t size, const struct IPv4_ADDRESS_s* address, uint16_t port) {
	assert(interface && interface->socket_sendto);
	return interface->socket_sendto(socket, buffer, size, address, port, interface->private_data);
}

bool network_socket_local_ipv4_info(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port) {
	assert(interface && interface->socket_local_ipv4_info);
	return interface->socket_local_ipv4_info(socket, ip, port, interface->private_data);
}

bool network_socket_remote_ipv4_info(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, struct IPv4_ADDRESS_s* ip,  uint16_t* port) {
	assert(interface && interface->socket_remote_ipv4_info);
	return interface->socket_remote_ipv4_info(socket, ip, port, interface->private_data);
}

bool network_socket_rx_buffer_max_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_rx_buffer_max_size && size);
	return interface->socket_rx_buffer_max_size(socket, size, interface->private_data);
}

bool network_socket_tx_buffer_max_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_tx_buffer_max_size && size);
	return interface->socket_tx_buffer_max_size(socket, size, interface->private_data);
}

bool network_socket_rx_buffer_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_rx_buffer_size && size);
	return interface->socket_rx_buffer_size(socket, size, interface->private_data);
}

bool network_socket_tx_buffer_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_tx_buffer_size && size);
	return interface->socket_tx_buffer_size(socket, size, interface->private_data);
}

bool network_socket_rx_buffer_free_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_rx_buffer_free_size && size);
	return interface->socket_rx_buffer_free_size(socket, size, interface->private_data);
}

bool network_socket_tx_buffer_free_size(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, uint16_t* size) {
	assert(interface && interface->socket_tx_buffer_free_size && size);
	return interface->socket_tx_buffer_free_size(socket, size, interface->private_data);
}
  
bool network_socket_clear_rx_buffer(struct ETHERNET_INTERFACE_s* interface, SOCKET socket) {
	assert(interface && interface->socket_rx_buffer_clear);
	return interface->socket_rx_buffer_clear(socket, interface->private_data);
}

socket_error_t network_socket_last_error(struct ETHERNET_INTERFACE_s* interface, SOCKET socket) {
	assert(interface && interface->socket_last_error);
	return interface->socket_last_error(socket, interface->private_data);
}

bool network_socket_set_option(struct ETHERNET_INTERFACE_s* interface, SOCKET socket, socket_option_t option, const void* value) {
	assert(interface && interface->socket_set_option);
	return interface->socket_set_option(socket, option, value, interface->private_data);
}

bool network_is_initialized(struct ETHERNET_INTERFACE_s* interface) {
	assert(interface && interface->is_initialized);
	return interface->is_initialized(interface->private_data);
}
