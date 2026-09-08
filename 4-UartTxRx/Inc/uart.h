/*
 * uart.h
 *
 *  Created on: Jul 15, 2025
 *      Author: ahmedabdelaziz
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f0xx.h"

#define GPIOAEN       (1U << 17)
#define UART2EN       (1U << 17)

#define SYS_FREQ      8000000U
#define APB1_CLK      SYS_FREQ
#define UART_BAUDRATE 9600U

void uart_tx_rx_init(void);
uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);

void uart_write_byte(uint8_t data);
void uart_transmit(const char *send);

uint8_t uart_read_byte(void);
uint8_t uart_data_available(void);
void uart_read_line(char *buffer, uint32_t length);


#endif /* UART_H_ */
