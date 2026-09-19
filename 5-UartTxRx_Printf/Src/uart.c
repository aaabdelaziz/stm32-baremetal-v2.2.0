/*
 * uart.c
 *
 *  Created on: Jul 15, 2025
 *      Author: ahmedabdelaziz
 */


#include "uart.h"


void uart_write_byte(uint8_t data)
{
    // Wait until TXE (Transmit data register empty) flag is set
    while (!(USART2->ISR & USART_ISR_TXE)) {}

    // Send one byte, for example ASCII 'A' = 0x41
    USART2->TDR = data;
}

void uart_transmit(const char *send)
{
    while (*send != '\0')
    {
        uart_write_byte((uint8_t)*send);
        send++;
    }
}

uint8_t uart_data_available(void)
{
    return (USART2->ISR & USART_ISR_RXNE) != 0U;
}

uint8_t uart_read_byte(void)
{
    while (!uart_data_available()) {}

    return (uint8_t)(USART2->RDR & 0xFFU);
}

void uart_read_line(char *buffer, uint32_t length)
{
    uint32_t index = 0U;

    if (length == 0U)
    {
        return;
    }

    while (index < (length - 1U))
    {
        char received = (char)uart_read_byte();

        if ((received == '\r') || (received == '\n'))
        {
            break;
        }

        buffer[index] = received;
        index++;
    }

    buffer[index] = '\0';
}

void uart_tx_rx_init(void)
{
    /************ Configure UART GPIO pins ************/
    // Enable clock access to GPIOA
    RCC->AHBENR |= GPIOAEN;

    // PA2 = USART2_TX, PA3 = USART2_RX, both on AF1.
    GPIOA->MODER &= ~((3U << (2U * 2U)) | (3U << (2U * 3U)));
    GPIOA->MODER |=  ((2U << (2U * 2U)) | (2U << (2U * 3U)));

    GPIOA->AFR[0] &= ~((0xFU << (4U * 2U)) | (0xFU << (4U * 3U)));
    GPIOA->AFR[0] |=  ((1U   << (4U * 2U)) | (1U   << (4U * 3U)));

    /************ Configure USART2 module ************/
    // Enable clock access to USART2
    RCC->APB1ENR |= UART2EN;

    // Disable USART before configuration
    USART2->CR1 &= ~USART_CR1_UE;

    // Set baudrate
    USART2->BRR = compute_uart_bd(APB1_CLK, UART_BAUDRATE);

    // Set word length to 8 bits, no parity, 1 stop bit (default)
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART2->CR2 = 0U;
    USART2->CR3 = 0U;

    // Enable USART2
    USART2->CR1 |= USART_CR1_UE;
}

uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate)
{
    return (uint16_t)((periph_clk + (baudrate / 2U)) / baudrate);
}
