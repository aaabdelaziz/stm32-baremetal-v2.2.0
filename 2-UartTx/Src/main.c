#include "uart.h"

int main(void)
{
    uart_tx_init();

    while (1)
    {
        uart_transmit("sensor_A:10");

        // Wait a bit before sending next byte
        for (volatile int i = 0; i < 100000; i++);
    }

    return 0;
}
