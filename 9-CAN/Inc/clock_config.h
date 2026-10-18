#ifndef CLOCK_CONFIG_H_
#define CLOCK_CONFIG_H_

/* Startup leaves the MCU on the reset HSI clock: 8 MHz. APB1 has no
 * prescaler here, so CAN1 and USART2 both receive an 8 MHz peripheral clock.
 * Keep the CAN bit timing and UART baud calculation consistent with this.
 */
#define SYSCLK_HZ      8000000U
#define APB1_CLOCK_HZ  SYSCLK_HZ
#define UART_BAUDRATE  9600U

#endif /* CLOCK_CONFIG_H_ */
