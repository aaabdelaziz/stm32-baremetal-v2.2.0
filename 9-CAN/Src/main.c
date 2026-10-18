#include "can.h"
#include "uart.h"

#define CAN_TX_ID 0x123U
#define CAN_TX_INTERVAL_LOOPS 3000000U

static void uart_write_hex(uint32_t value, uint8_t digits)
{
    uint8_t i;

    /* Print a fixed number of hexadecimal digits so standard and extended
     * identifiers have predictable widths in terminal output.
     */
    for (i = 0U; i < digits; ++i)
    {
        uint8_t shift = (uint8_t)((digits - 1U - i) * 4U);
        uint8_t nibble = (uint8_t)((value >> shift) & 0x0FU);
        char digit = (nibble < 10U) ? (char)('0' + nibble)
                                    : (char)('A' + (nibble - 10U));
        uart_write_byte((uint8_t)digit);
    }
}

static void print_received_frame(const can_frame_t *frame)
{
    uint8_t i;
    uint8_t id_digits = (frame->is_extended != 0U) ? 8U : 3U;

    uart_transmit("CAN RX ID=0x");
    uart_write_hex(frame->identifier, id_digits);
    uart_transmit((frame->is_extended != 0U) ? " EXT" : " STD");
    uart_transmit((frame->is_remote != 0U) ? " RTR" : " DATA");
    uart_transmit(" DLC=");
    uart_write_byte((uint8_t)('0' + frame->length));

    /* Remote frames request data but contain no payload bytes. */
    if (frame->is_remote == 0U)
    {
        uart_transmit(" DATA=");
        for (i = 0U; i < frame->length; ++i)
        {
            if (i != 0U)
            {
                uart_write_byte(' ');
            }
            uart_write_hex(frame->data[i], 2U);
        }
    }

    if (frame->overrun != 0U)
    {
        uart_transmit(" FIFO_OVERRUN");
    }
    uart_transmit("\r\n");
}

int main(void)
{
    /* This node periodically sends the fixed frame below. Its receive filter
     * accepts other standard and extended frames into FIFO0.
     */
    static const uint8_t payload[8] = {
        0xCAU, 0x4EU, 0x00U, 0x01U, 0xDEU, 0xADU, 0xBEU, 0xEFU
    };
    can_frame_t received_frame;
    volatile uint32_t tx_countdown = 0U;

    /* USART2 is used for human-readable status; CAN carries the bus frames. */
    uart_tx_rx_init();
    uart_transmit("STM32F091 bxCAN send/receive demo\r\n");
    uart_transmit("CAN1: 125 kbit/s, standard TX ID 0x123\r\n");

    if (!can1_init())
    {
        uart_transmit("CAN initialization failed\r\n");
        while (1) {}
    }

    uart_transmit("CAN initialized; transmitting and polling RX FIFO0\r\n");

    while (1)
    {
        /* Poll receive on each pass. can1_receive() returns immediately when
         * FIFO0 is empty and releases the hardware mailbox after copying it.
         */
        if (can1_receive(&received_frame) == CAN_RX_FRAME)
        {
            print_received_frame(&received_frame);
        }

        /* Transmit on startup and then after an approximate loop-count
         * interval. bxCAN, not this delay, determines the CAN wire bitrate.
         */
        if (tx_countdown == 0U)
        {
            can_tx_result_t result = can1_send(CAN_TX_ID, payload, sizeof(payload));

            if (result == CAN_TX_OK)
            {
                uart_transmit("CAN TX ID=0x123 acknowledged\r\n");
            }
            else if (result == CAN_TX_NO_MAILBOX)
            {
                uart_transmit("CAN TX mailbox busy\r\n");
            }
            else if (result == CAN_TX_TIMEOUT)
            {
                uart_transmit("CAN TX timed out\r\n");
            }
            else
            {
                uart_transmit("CAN TX error; check transceiver, bitrate, and ACK node\r\n");
            }

            tx_countdown = CAN_TX_INTERVAL_LOOPS;
        }
        else
        {
            --tx_countdown;
        }
    }
}
