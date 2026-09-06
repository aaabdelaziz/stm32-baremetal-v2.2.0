#include "uart.h"

int main(void)
{
    uart_tx_init();

    while (1)
    {
        uart_transmit("A");

        // Wait a bit before sending next byte
        for (volatile int i = 0; i < 100000; i++);
    }

    return 0;
}
