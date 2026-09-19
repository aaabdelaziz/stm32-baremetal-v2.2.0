#include "uart.h"

int main(void)
{
    char rx_buffer[32];

    uart_tx_rx_init();
    uart_transmit("UART TX/RX ready\r\n");
    uart_transmit("Type text and press Enter:\r\n");

    while (1)
    {
        uart_read_line(rx_buffer, sizeof(rx_buffer));

        uart_transmit("RX: ");
        uart_transmit(rx_buffer);
        uart_transmit("\r\n");
    }

    return 0;
}
