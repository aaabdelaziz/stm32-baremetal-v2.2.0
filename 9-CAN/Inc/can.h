#ifndef CAN_H_
#define CAN_H_

#include "stm32f0xx.h"
#include "clock_config.h"

#define CAN1_BITRATE       125000U

/* Result of a synchronous transmit request. CAN_TX_OK means a bus node
 * acknowledged the frame; a loaded mailbox alone does not mean it was sent.
 */
typedef enum
{
    CAN_TX_OK = 0,
    CAN_TX_NO_MAILBOX,
    CAN_TX_TIMEOUT,
    CAN_TX_ERROR
} can_tx_result_t;

/* One frame copied out of bxCAN FIFO0. A received extended identifier uses
 * all 29 bits; a standard identifier uses only the low 11 bits. For a remote
 * frame, length is the requested DLC and data[] contains no received payload.
 */
typedef struct
{
    uint32_t identifier;
    uint8_t is_extended;
    uint8_t is_remote;
    uint8_t length;
    uint8_t overrun;
    uint8_t data[8];
} can_frame_t;

typedef enum
{
    CAN_RX_FRAME = 0,
    CAN_RX_EMPTY,
    CAN_RX_INVALID
} can_rx_result_t;

/* Set up GPIO pins, CAN clock, bit timing, and normal operating mode.
 * Returns 1 when bxCAN leaves initialization mode, or 0 on timeout.
 */
uint8_t can1_init(void);

/* Send one standard (11-bit ID) data frame using transmit mailbox 0.
 * Waits for completion and returns the bus result. DLC must be 0 through 8.
 */
can_tx_result_t can1_send(uint16_t standard_id,
                          const uint8_t *data,
                          uint8_t length);

/* Poll FIFO0 once. Copies one pending frame and releases its hardware slot.
 * The filter accepts all standard and extended identifiers into FIFO0.
 */
can_rx_result_t can1_receive(can_frame_t *frame);

#endif /* CAN_H_ */
