#include "transfer.h"

#include <stdint.h>

#include "hardware/uart.h"

#include "ethernet.h"

extern enum transfer_type_s transfer_type;

void send_data_uart(uint8_t* data, uint32_t data_len)
{
    uart_write_blocking(uart0, data, data_len);
}

void send_data_spi(uint8_t* data, uint32_t data_len)
{

}

void send_data_ethernet(uint8_t* data, uint32_t data_len)
{
    ethernet_send_udp(data, data_len);
}

uint16_t receive_data_ethernet(uint8_t* data)
{
    return ethernet_receive_udp(data);
}

void send_data(uint8_t* data, uint32_t data_len)
{
    switch(transfer_type)
    {
        case TRANSFER_UART:
            send_data_uart(data, data_len);
            break;
        case TRANSFER_SPI:
            send_data_spi(data, data_len);
            break;
        case TRANSFER_ETHERNET:
            send_data_ethernet(data, data_len);
        default:
            break;
    }
}

uint16_t receive_data(uint8_t* data)
{
    switch(transfer_type)
    {
        case TRANSFER_ETHERNET:
            return receive_data_ethernet(data);
        default:
            break;
    }

    return 0;
}