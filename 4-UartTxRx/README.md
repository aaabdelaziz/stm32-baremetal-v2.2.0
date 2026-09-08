# 4-UartTxRx - Bare-Metal USART2 Transmit And Receive

This project extends the USART2 transmit example into a simple blocking UART RX/TX application. The firmware prints a ready message, waits for text from the host computer, then echoes the received line back with an `RX:` prefix.

## Target

```console
Board: NUCLEO-F091RC
MCU:   STM32F091RCTx
Core:  Arm Cortex-M0
UART:  USART2
TX:    PA2
RX:    PA3
Baud:  9600
Frame: 8 data bits, no parity, 1 stop bit
Logic: 3.3 V, non-inverted UART, idle high
```

## Project Structure

```console
4-UartTxRx/
  Inc/
    uart.h
  Src/
    main.c
    syscalls.c
    sysmem.c
    uart.c
  Startup/
    startup_stm32f091rctx.s
  STM32F091RCTX_FLASH.ld
```

CMSIS headers are stored in the shared repository folder:

```console
chip_headers/CMSIS/Device/ST/STM32F0xx/Include
chip_headers/CMSIS/Include
```

## STM32CubeIDE Import

```console
File > Import... > General > Existing Projects into Workspace
```

![Import existing projects](../docs/images/stm32cubeide-import-existing-projects.png)

Import both:

```console
4-UartTxRx
chip_headers
```

![Import UART TX/RX project](../docs/images/4-uarttxrx-import-project.png)

## Required Compiler Include Paths

Configure the real compiler include paths under `C/C++ Build`:

```console
Right-click 4-UartTxRx
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Include paths
```

![UART TX/RX compiler include paths page](../docs/images/4-uarttxrx-compiler-include-paths-page.png)

Use:

```console
Configuration: All configurations
```

Add:

```console
../Inc
${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
${workspace_loc:/chip_headers/CMSIS/Include}
```

![UART TX/RX compiler include paths](../docs/images/4-uarttxrx-compiler-include-paths.png)

## Required Compiler Symbols

Configure symbols under:

```console
Right-click 4-UartTxRx
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Preprocessor
```

![UART TX/RX compiler symbols page](../docs/images/4-uarttxrx-preprocessor-symbols-page.png)

Add:

```console
STM32
STM32F0
STM32F091RCTx
STM32F091xC
NUCLEO_F091RC
```

![UART TX/RX compiler symbols](../docs/images/4-uarttxrx-preprocessor-symbols.png)

`STM32F091xC` is the important CMSIS macro. `STM32F091RC` is not the device-selection macro used by `stm32f0xx.h`.

## Build And Flash

```console
Project > Clean...
Right-click 4-UartTxRx > Build Project
Right-click 4-UartTxRx > Debug As > STM32 Cortex-M C/C++ Application
```

![UART TX/RX clean project](../docs/images/4-uarttxrx-clean-project.png)
![UART TX/RX build project](../docs/images/4-uarttxrx-build-project.png)
![UART TX/RX debug configuration](../docs/images/4-uarttxrx-debug-configuration.png)

Recommended debug settings:

```console
Application: Debug/4-UartTxRx.elf
Interface:   SWD
Reset mode:  Connect under reset
SWV:         Disabled
```

## Code Walkthrough

`main.c` initializes USART2, prints a startup message, waits for a line from the terminal, then echoes it back:

```c
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
}
```

USART2 TX/RX initialization configures PA2 and PA3 as alternate function 1:

```c
GPIOA->MODER &= ~((3U << (2U * 2U)) | (3U << (2U * 3U)));
GPIOA->MODER |=  ((2U << (2U * 2U)) | (2U << (2U * 3U)));

GPIOA->AFR[0] &= ~((0xFU << (4U * 2U)) | (0xFU << (4U * 3U)));
GPIOA->AFR[0] |=  ((1U   << (4U * 2U)) | (1U   << (4U * 3U)));
```

USART2 is configured for 9600 baud and both transmitter and receiver are enabled:

```c
RCC->APB1ENR |= UART2EN;

USART2->CR1 &= ~USART_CR1_UE;
USART2->BRR = compute_uart_bd(APB1_CLK, UART_BAUDRATE);
USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
USART2->CR2 = 0U;
USART2->CR3 = 0U;
USART2->CR1 |= USART_CR1_UE;
```

String transmit sends one byte at a time until the null terminator:

```c
void uart_transmit(const char *send)
{
    while (*send != '\0')
    {
        uart_write_byte((uint8_t)*send);
        send++;
    }
}
```

Byte receive waits until the RX data register is not empty, then reads `RDR`:

```c
uint8_t uart_read_byte(void)
{
    while (!uart_data_available()) {}

    return (uint8_t)(USART2->RDR & 0xFFU);
}
```

Line receive stores characters until Enter is pressed or the buffer is full:

```c
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
```

## Terminal Test

Connect the Nucleo board through the ST-LINK USB connector. On macOS, find the serial port:

```bash
/bin/ls /dev/cu.usbmodem*
```

Open the port:

```bash
screen /dev/cu.usbmodem212403 9600
```

Use your actual `/dev/cu.usbmodem*` name.

Expected startup text:

```console
UART TX/RX ready
Type text and press Enter:
```

Type:

```console
hello
```

Expected echo:

```console
RX: hello
```

![UART TX/RX terminal echo](../docs/images/4-uarttxrx-terminal-echo.png)

Exit `screen`:

```console
Ctrl-A
K
Y
```

## Logic Analyzer Test

Connect:

```console
Analyzer GND -> Nucleo GND
Analyzer D0  -> PA2 / USART2_TX
Analyzer D1  -> PA3 / USART2_RX
```

Decoder settings:

```console
Analyzer:       Async Serial
Bit rate:       9600
Data bits:      8
Parity:         None
Stop bits:      1
Bit order:      LSB first
Signal:         Non-inverted
Idle level:     High
Sample rate:    1 MS/s minimum, 5-10 MS/s recommended
Logic voltage:  3.3 V
```

When the board prints the prompt, activity appears on TX. When you type into the terminal, activity appears on RX. When the firmware echoes the message, TX becomes active again.

![UART TX/RX logic analyzer capture](../docs/images/4-uarttxrx-logic-analyzer-capture.png)

## Troubleshooting

### Terminal Shows Startup Text But Echo Does Not Work

Check that your terminal sends a line ending when Enter is pressed. The firmware stops reading on `\r` or `\n`.

### Nothing Appears In Terminal

Check:

```console
Correct serial port
Baud rate is 9600
Board is flashed with 4-UartTxRx
USART2 PA2/PA3 are routed to ST-LINK virtual COM port
```

### Logic Analyzer Sees TX But Not RX

RX activity only appears when the host sends data to the board. Type in the terminal while the analyzer is capturing.

### Build Fails With `stm32f0xx.h: No such file or directory`

Add:

```console
${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
```

### Build Fails With Device Selection Error

Add:

```console
STM32F091xC
```
