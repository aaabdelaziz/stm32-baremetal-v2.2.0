#include "uart.h"

int main(void)
{

    uart_tx_rx_init();
    uart_transmit("UART TX/RX ready\r\n");
    uart_transmit("Type text and press Enter:\r\n");

    while (1)
    {
    	printf("From STM32F4 ....\n\r");
    }

    return 0;
}
