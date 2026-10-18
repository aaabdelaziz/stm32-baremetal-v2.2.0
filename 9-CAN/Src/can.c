#include "can.h"

#define CAN_INIT_TIMEOUT 1000000U
#define CAN_TX_WAIT_LIMIT 2000000U

static uint8_t wait_for_register(volatile uint32_t *reg,
                                 uint32_t mask,
                                 uint8_t set,
                                 uint32_t timeout)
{
    /* Hardware changes status bits asynchronously. Bound each wait so a
     * missing clock or unexpected peripheral state cannot hang forever.
     */
    while (timeout-- > 0U)
    {
        if (((*reg & mask) != 0U) == (set != 0U))
        {
            return 1U;
        }
    }

    return 0U;
}

uint8_t can1_init(void)
{
    /* Step 1: enable GPIOA and CAN1 clocks. CAN1 runs from APB1; GPIOA must
     * be clocked before its mode and alternate-function registers are used.
     */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_CANEN;
    (void)RCC->APB1ENR;

    /* Step 2: route PA11 (CAN_RX) and PA12 (CAN_TX) through AF4. CAN_TX is
     * push-pull; high GPIO speed gives the peripheral output a suitable edge.
     */
    GPIOA->MODER &= ~((3U << (11U * 2U)) | (3U << (12U * 2U)));
    GPIOA->MODER |=  ((2U << (11U * 2U)) | (2U << (12U * 2U)));
    GPIOA->OSPEEDR &= ~((3U << (11U * 2U)) | (3U << (12U * 2U)));
    GPIOA->OSPEEDR |=  ((2U << (11U * 2U)) | (2U << (12U * 2U)));
    GPIOA->OTYPER &= ~((1U << 11U) | (1U << 12U));
    GPIOA->PUPDR &= ~((3U << (11U * 2U)) | (3U << (12U * 2U)));
    GPIOA->AFR[1] &= ~((0xFU << 12U) | (0xFU << 16U));
    GPIOA->AFR[1] |=  ((4U << 12U) | (4U << 16U));

    /* Step 3: enter initialization mode. bxCAN only allows timing changes
     * while INAK is acknowledged in MSR, so wait for that hardware handshake.
     */
    CAN->MCR |= CAN_MCR_INRQ;
    if (!wait_for_register(&CAN->MSR, CAN_MSR_INAK, 1U, CAN_INIT_TIMEOUT))
    {
        return 0U;
    }

    /* Step 4: configure one CAN bit as 16 time quanta:
     *   tq = (BRP + 1) / APB1 = 4 / 8 MHz = 0.5 us
     *   bit = 1 sync + 13 TS1 + 2 TS2 = 16 tq = 8 us (125 kbit/s)
     * The sample point is after sync + TS1: 14/16 = 87.5%.
     * Register fields encode each value minus one, hence BRP=3, TS1=12,
     * and TS2=1. SJW stays at its reset value of one time quantum.
     */
#if APB1_CLOCK_HZ != 8000000U
#error "Recalculate CAN1 BTR when changing APB1_CLOCK_HZ"
#endif
#if (APB1_CLOCK_HZ / (4U * 16U)) != CAN1_BITRATE
#error "CAN1 BTR timing does not match CAN1_BITRATE"
#endif
    CAN->BTR = (3U << CAN_BTR_BRP_Pos) |
               (12U << CAN_BTR_TS1_Pos) |
               (1U << CAN_BTR_TS2_Pos);

    /* Step 5: configure filter bank 0 as a 32-bit identifier-mask filter.
     * A zero ID and zero mask mean "don't care" for every identifier bit, so
     * both standard and extended frames are accepted into receive FIFO0.
     * bxCAN filter registers can only be changed while FINIT is set.
     */
    CAN->FMR |= CAN_FMR_FINIT;
    CAN->FA1R &= ~CAN_FA1R_FACT0;
    CAN->FM1R &= ~CAN_FM1R_FBM0;       /* Mask mode, not identifier-list mode. */
    CAN->FS1R |= CAN_FS1R_FSC0;        /* Use the 32-bit filter scale. */
    CAN->FFA1R &= ~CAN_FFA1R_FFA0;     /* Route filter 0 to FIFO0. */
    CAN->sFilterRegister[0].FR1 = 0U;  /* Identifier value: accept any ID. */
    CAN->sFilterRegister[0].FR2 = 0U;  /* Mask value: compare no ID bits. */
    CAN->FA1R |= CAN_FA1R_FACT0;
    CAN->FMR &= ~CAN_FMR_FINIT;

    /* Step 6: disable automatic retransmission. Each request gets one
     * attempt, so a missing ACK completes with an error and frees the mailbox
     * instead of retrying forever. Then leave initialization mode.
     */
    CAN->MCR |= CAN_MCR_NART;
    CAN->MCR &= ~CAN_MCR_INRQ;

    return wait_for_register(&CAN->MSR, CAN_MSR_INAK, 0U, CAN_INIT_TIMEOUT);
}

can_tx_result_t can1_send(uint16_t standard_id,
                          const uint8_t *data,
                          uint8_t length)
{
    uint32_t word_low = 0U;
    uint32_t word_high = 0U;
    uint32_t status;
    uint8_t i;

    /* Reject values that do not fit the standard frame fields. */
    if ((standard_id > 0x7FFU) || (length > 8U) ||
        ((length > 0U) && (data == (const uint8_t *)0)))
    {
        return CAN_TX_ERROR;
    }

    /* Mailbox registers are writable only while the mailbox is empty. */
    if ((CAN->TSR & CAN_TSR_TME0) == 0U)
    {
        return CAN_TX_NO_MAILBOX;
    }

    /* bxCAN data registers are 32-bit words. Pack each data byte into its
     * byte lane; byte 0 occupies bits 7:0 and is transmitted first.
     */
    for (i = 0U; i < length; ++i)
    {
        if (i < 4U)
        {
            word_low |= (uint32_t)data[i] << (8U * i);
        }
        else
        {
            word_high |= (uint32_t)data[i] << (8U * (i - 4U));
        }
    }

    /* Fill mailbox 0 before setting TXRQ. Standard IDs occupy TIR[31:21];
     * IDE and RTR remain zero, selecting a standard data frame.
     */
    CAN->sTxMailBox[0].TIR = 0U;
    CAN->sTxMailBox[0].TDTR = length;
    CAN->sTxMailBox[0].TDLR = word_low;
    CAN->sTxMailBox[0].TDHR = word_high;
    CAN->sTxMailBox[0].TIR = ((uint32_t)standard_id << 21U) | CAN_TI0R_TXRQ;

    /* Wait for mailbox completion, then check TXOK0. A failed ACK or other
     * bus error completes with TXOK0 clear.
     */
    if (!wait_for_register(&CAN->TSR, CAN_TSR_RQCP0, 1U, CAN_TX_WAIT_LIMIT))
    {
        return CAN_TX_TIMEOUT;
    }

    status = CAN->TSR;
    CAN->TSR = CAN_TSR_RQCP0; /* Write one to clear mailbox 0 completion. */

    return ((status & CAN_TSR_TXOK0) != 0U) ? CAN_TX_OK : CAN_TX_ERROR;
}

can_rx_result_t can1_receive(can_frame_t *frame)
{
    uint32_t identifier_register;
    uint32_t data_low;
    uint32_t data_high;
    uint8_t dlc;
    uint8_t i;

    if (frame == (can_frame_t *)0)
    {
        return CAN_RX_INVALID;
    }

    /* FMP0 counts messages waiting in FIFO0. Return immediately when empty;
     * main can keep polling without blocking for an incoming frame.
     */
    if ((CAN->RF0R & CAN_RF0R_FMP0) == 0U)
    {
        return CAN_RX_EMPTY;
    }

    /* Copy all mailbox registers before releasing the FIFO entry. */
    identifier_register = CAN->sFIFOMailBox[0].RIR;
    dlc = (uint8_t)(CAN->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC);
    data_low = CAN->sFIFOMailBox[0].RDLR;
    data_high = CAN->sFIFOMailBox[0].RDHR;
    frame->overrun = (CAN->RF0R & CAN_RF0R_FOVR0) != 0U;

    frame->is_extended = (identifier_register & CAN_RI0R_IDE) != 0U;
    frame->is_remote = (identifier_register & CAN_RI0R_RTR) != 0U;
    if (frame->is_extended != 0U)
    {
        /* EXID holds the low 18 bits and STID the high 11 bits of a 29-bit ID.
         */
        frame->identifier = ((identifier_register & CAN_RI0R_STID) >> 3U) |
                            ((identifier_register & CAN_RI0R_EXID) >> 3U);
    }
    else
    {
        frame->identifier = (identifier_register & CAN_RI0R_STID) >>
                            CAN_RI0R_STID_Pos;
    }

    /* Classic CAN carries at most eight data bytes. Preserve a valid in-memory
     * length even if a malformed or non-classic DLC value is observed.
     */
    frame->length = (dlc > 8U) ? 8U : dlc;
    if (frame->is_remote == 0U)
    {
        for (i = 0U; i < frame->length; ++i)
        {
            if (i < 4U)
            {
                frame->data[i] = (uint8_t)(data_low >> (8U * i));
            }
            else
            {
                frame->data[i] = (uint8_t)(data_high >> (8U * (i - 4U)));
            }
        }
    }

    /* FOVR0 is write-one-to-clear. RFOM0 releases this receive mailbox so the
     * hardware can store the next accepted frame in FIFO0.
     */
    if (frame->overrun != 0U)
    {
        CAN->RF0R = CAN_RF0R_FOVR0;
    }
    CAN->RF0R = CAN_RF0R_RFOM0;

    return CAN_RX_FRAME;
}
