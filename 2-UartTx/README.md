# 2-UartTx - Bare-Metal USART2 Transmit Test

This project is a minimal bare-metal UART transmit example for an STM32F091RC / NUCLEO-F091RC target. It configures USART2 on PA2/PA3 and repeatedly transmits the ASCII character `A` (`0x41`) at `9600 baud`.

The project is intentionally register-level: no HAL, no CubeMX `.ioc`, and no operating system.

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
2-UartTx/
  Inc/
    uart.h
  Src/
    main.c
    uart.c
  Startup/
    startup_stm32f091rctx.s
  STM32F091RCTX_FLASH.ld
```

CMSIS device headers are stored outside the project in:

```console
chip_headers/CMSIS/Device/ST/STM32F0xx/Include
chip_headers/CMSIS/Include
```

## STM32CubeIDE Import

Import the project as an existing Eclipse/STM32CubeIDE project:

```console
File > Import... > General > Existing Projects into Workspace
```

Screenshot placeholder:

![STM32CubeIDE import existing projects](../docs/images/stm32cubeide-import-existing-projects.png)

Import both:

```console
2-UartTx
chip_headers
```

Screenshot placeholder:

![STM32CubeIDE import 2-UartTx and chip_headers](../docs/images/stm32cubeide-import-uart-and-chip-headers.png)

The `chip_headers` project/folder is required because `uart.h` includes:

```c
#include "stm32f0xx.h"
```

## Required Compiler Include Paths

The most important include-path location is under `C/C++ Build`, because this is what the real compiler uses.

Open:

```console
Right-click 2-UartTx
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Include paths
```

Screenshot placeholder:

![STM32CubeIDE compiler include paths page](../docs/images/stm32cubeide-compiler-include-paths-page.png)

Set:

```console
Configuration: All configurations
```

Screenshot placeholder:

![STM32CubeIDE all configurations selected](../docs/images/stm32cubeide-all-configurations.png)

Add:

```console
../Inc
/chip_headers/CMSIS/Device/ST/STM32F0xx/Include
/chip_headers/CMSIS/Include
```

Screenshot placeholder:

![STM32CubeIDE required compiler include paths](../docs/images/stm32cubeide-required-compiler-include-paths.png)

If workspace-relative paths do not resolve correctly, use absolute paths:

```console
<repo-root>/2-UartTx/Inc
<repo-root>/chip_headers/CMSIS/Device/ST/STM32F0xx/Include
<repo-root>/chip_headers/CMSIS/Include
```

Important:

```console
Wrong: /chip_headers/CMSIS/Device/ST/STM32F0xx
Right: /chip_headers/CMSIS/Device/ST/STM32F0xx/Include
```

The final `/Include` is required because `stm32f0xx.h` is located inside that folder.

## Required Compiler Symbols

Open:

```console
Right-click 2-UartTx
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Preprocessor
```

Screenshot placeholder:

![STM32CubeIDE compiler preprocessor symbols page](../docs/images/stm32cubeide-compiler-preprocessor-symbols-page.png)

Set:

```console
Configuration: All configurations
```

Add:

```console
STM32
STM32F0
STM32F091RCTx
STM32F091xC
```

Screenshot placeholder:

![STM32CubeIDE required compiler preprocessor symbols](../docs/images/stm32cubeide-required-compiler-symbols.png)

The critical symbol is:

```console
STM32F091xC
```

`STM32F091RCTx` is the CubeIDE target name. `STM32F091xC` is the CMSIS device macro used by ST headers. If `STM32F091xC` is missing, `stm32f0xx.h` cannot select the correct device header.

Do not replace it with:

```console
STM32F091RC
```

That name is not recognized by `stm32f0xx.h`.

## Optional Editor/Indexer Include Paths

This section is useful for autocomplete and red squiggles, but it is not the real compiler setup.

Open:

```console
Right-click 2-UartTx
Properties
C/C++ General
Paths and Symbols
Includes
GNU C
```

Screenshot placeholder:

![STM32CubeIDE indexer include paths page](../docs/images/stm32cubeide-indexer-include-paths-page.png)

Add the same include paths:

```console
../Inc
/chip_headers/CMSIS/Device/ST/STM32F0xx/Include
/chip_headers/CMSIS/Include
```

Then:

```console
C/C++ General > Paths and Symbols > Symbols > GNU C
```

Screenshot placeholder:

![STM32CubeIDE indexer symbols page](../docs/images/stm32cubeide-indexer-symbols-page.png)

Add:

```console
STM32F091xC
```

Simple rule:

```console
C/C++ Build   = real compiler
C/C++ General = editor/indexer
```

When fixing build errors, always fix `C/C++ Build` first.

## Clean And Build

After changing paths or symbols:

```console
Project > Clean...
```

Screenshot placeholder:

![STM32CubeIDE clean project dialog](../docs/images/stm32cubeide-clean-project-dialog.png)

Select:

```console
2-UartTx
```

Then build:

```console
Right-click 2-UartTx > Build Project
```

Screenshot placeholder:

![STM32CubeIDE build project command](../docs/images/stm32cubeide-build-project-command.png)

A successful build ends with:

```console
Finished building target: 2-UartTx.elf
```

Warnings such as `_close is not implemented`, `_read is not implemented`, and `_write is not implemented` are normal for this bare-metal project when `nosys.specs` is used. They are not build failures.

## Flashing

Use Debug first while validating hardware.

```console
Right-click 2-UartTx
Build Configurations > Set Active > Debug
```

Screenshot placeholder:

![STM32CubeIDE set active debug configuration](../docs/images/stm32cubeide-set-active-debug-configuration.png)

Then:

```console
Right-click 2-UartTx
Debug As > STM32 Cortex-M C/C++ Application
```

Screenshot placeholder:

![STM32CubeIDE debug as STM32 Cortex-M application](../docs/images/stm32cubeide-debug-as-cortex-m-application.png)

Recommended debug settings:

```console
Application: Debug/2-UartTx.elf
Interface:   SWD
Reset mode:  Connect under reset
SWV:         Disabled
```

Screenshot placeholder:

![STM32CubeIDE debug configuration settings](../docs/images/stm32cubeide-debug-configuration-settings.png)

Before debugging from CubeIDE, STM32CubeProgrammer should be able to detect the ST-LINK. If STM32CubeProgrammer reports `No ST-LINK detected`, CubeIDE will not be able to flash.

## Code Walkthrough

### Application Loop

`main.c` initializes USART2, waits for the transmit data register to become empty, and writes the ASCII character `A`.

```c
#include "uart.h"

int main(void)
{
    uart_tx_init();

    while (1)
    {
        while (!(USART2->ISR & USART_ISR_TXE)) {}

        USART2->TDR = 'A';

        for (volatile int i = 0; i < 100000; i++);
    }

    return 0;
}
```

Important register:

```console
USART2->ISR & USART_ISR_TXE
```

This checks whether the transmit data register is empty.

Important write:

```console
USART2->TDR = 'A'
```

This loads the UART transmit data register with `0x41`.

### UART Initialization

`uart_tx_init()` configures GPIOA pins PA2/PA3 for USART2 alternate function mode, enables the USART2 peripheral clock, programs the baud rate, and enables the USART.

```c
void uart_tx_init(void)
{
    RCC->AHBENR |= GPIOAEN;

    GPIOA->MODER &= ~((1U<<4) | (1U<<6));
    GPIOA->MODER |=  ((1U<<5) | (1U<<7));

    GPIOA->AFR[0] &= ~((0xF << 8) | (0xF << 12));
    GPIOA->AFR[0] |=  ((1 << 8) | (1 << 12));

    RCC->APB1ENR |= UART2EN;

    USART2->CR1 &= ~USART_CR1_UE;

    USART2->BRR = compute_uart_bd(APB1_CLK, UART_BAUDRATE);

    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;

    USART2->CR1 |= USART_CR1_UE;
}
```

Register-level sequence:

```console
1. Enable GPIOA clock
2. Put PA2 and PA3 into alternate function mode
3. Select AF1 for PA2/PA3, which maps them to USART2
4. Enable USART2 peripheral clock
5. Disable USART before configuration
6. Set baud rate register
7. Enable transmitter and receiver
8. Enable USART
```

### Baud Rate

`uart.h` defines:

```c
#define SYS_FREQ      8000000
#define APB1_CLK      SYS_FREQ
#define UART_BAUDRATE 9600
```

The baud register value is computed as:

```c
uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate)
{
    return (periph_clk + (baudrate/2U)) / baudrate;
}
```

For an 8 MHz APB1 clock and 9600 baud:

```console
BRR ~= 8000000 / 9600 ~= 833
```

Each UART bit is:

```console
1 / 9600 = 104.16 us
```

## Hardware Routing On NUCLEO-F091RC

USART2 is available on:

```console
PA2 = USART2_TX
PA3 = USART2_RX
```

On the NUCLEO-F091RC, the default board configuration routes USART2 to the ST-LINK virtual COM port:

```console
SB13, SB14 = ON
SB62, SB63 = OFF
```

This means the UART data can be tested over USB serial on the host computer without soldering or changing board bridges.

Do not expect the signal on Arduino D1/D0 by default. With default solder bridge settings, PA2/PA3 are connected to the ST-LINK VCP path, not necessarily to the Arduino header pins.

## Test Method 1: Host Terminal Using ST-LINK Virtual COM Port

This is the easiest and safest test. No soldering is required.

Connect the board using the ST-LINK USB connector.

Find the serial port on macOS:

```bash
/bin/ls /dev/cu.usbmodem*
```

Open the port:

```bash
screen /dev/cu.usbmodem212403 9600
```

Use the actual port returned by your machine if different.

Expected output:

```console
AAAAAAAAAAAAAAAAAAAAAAAA
```

Exit `screen`:

```console
Ctrl-A
K
Y
```

If the terminal receives `A`, then:

```console
USART2 TX is working
PA2 is transmitting
ST-LINK VCP routing is working
Baud rate is correct
```

## Test Method 2: Logic Analyzer Or Oscilloscope

Connect:

```console
Logic analyzer GND -> Nucleo GND
Logic analyzer D0  -> USART2 TX measurement point
```

For the default Nucleo UART-to-ST-LINK routing, probe the TX path around the ST-LINK USART solder bridges. If unsure which bridge carries TX, connect two analyzer channels:

```console
D0 -> one side of SB13
D1 -> one side of SB14
GND -> board GND
```

Only one of these channels will show the target-to-host traffic for repeated `A`; the other may remain idle unless the PC sends data back to the board.

Logic analyzer decoder:

```console
Analyzer:       Async Serial
Bit rate:       9600
Data bits:      8
Parity:         None
Stop bits:      1
Bit order:      LSB first
Signal:         Non-inverted
Idle level:     High
Sample rate:    1 MS/s minimum, 5 MS/s or 10 MS/s recommended
Logic voltage:  3.3 V
```

Oscilloscope setup:

```console
Coupling:       DC
Voltage scale:  1 V/div
Time scale:     100 us/div or 200 us/div
Trigger:        Falling edge
Trigger level:  Around 1.5 V
```

## Expected UART Frame For `A`

ASCII capital `A` is:

```console
0x41 = binary 0100 0001
```

UART sends data least-significant bit first:

```console
Start  b0 b1 b2 b3 b4 b5 b6 b7  Stop
  0     1  0  0  0  0  0  1  0    1
```

Expected shape:

```console
3.3V  -----+     +-----+                         +-----+     +--------
           |     |     |                         |     |     |
0V         +-----+     +-------------------------+     +-----+
        Idle  Start  b0=1 b1=0 b2=0 b3=0 b4=0 b5=0 b6=1 b7=0 Stop
```

At `9600 baud`, one bit is approximately:

```console
104 us
```

One full UART frame is:

```console
10 bits * 104 us = about 1.04 ms
```

## Logic Analyzer Proof

The following capture shows the UART decoder reading `0x41`, which is ASCII `A`.

![UART logic analyzer proof for ASCII A](../docs/images/uart-a-logic-analyzer-proof.png)

This proves that the firmware is transmitting the expected character with the correct UART framing.

## Troubleshooting

### Build Fails With `stm32f0xx.h: No such file or directory`

The compiler include path is missing or wrong.

Fix under:

```console
C/C++ Build > Settings > Tool Settings > MCU/MPU GCC Compiler > Include paths
```

Required:

```console
/chip_headers/CMSIS/Device/ST/STM32F0xx/Include
/chip_headers/CMSIS/Include
```

### Build Fails With Device Selection Error

If `stm32f0xx.h` asks you to select the target STM32F0 device, add:

```console
STM32F091xC
```

under:

```console
C/C++ Build > Settings > Tool Settings > MCU/MPU GCC Compiler > Preprocessor
```

### Build Shows `_read`, `_write`, `_close`, Or `_lseek` Warnings

These warnings are normal for this project:

```console
_close is not implemented and will always fail
_lseek is not implemented and will always fail
_read is not implemented and will always fail
_write is not implemented and will always fail
```

The project uses `nosys.specs`, so there is no operating system file I/O backend. The UART code writes directly to `USART2->TDR`, so these warnings do not prevent this test from working.

### Flash Fails With `No ST-LINK Detected`

Check STM32CubeProgrammer first. If it cannot detect ST-LINK, CubeIDE cannot flash.

Check:

```console
USB data cable, not charge-only cable
ST-LINK USB connector
Board power LED
Direct USB connection instead of hub
ST-LINK server installed
macOS Privacy & Security approval for STMicroelectronics software
```

### Terminal Shows `A` But Logic Analyzer Shows Nothing

If the terminal shows repeated `A`, UART is working. The analyzer is probably on the wrong point or has no common ground.

Check:

```console
Analyzer GND connected to Nucleo GND
Probe is on the actual TX bridge/pad
Sample rate is at least 1 MS/s
Async Serial decoder is set to 9600 8N1
Signal is non-inverted
Analyzer input threshold is compatible with 3.3 V logic
```

Do not probe Arduino D1/D0 expecting USART2 by default unless the solder bridge configuration routes PA2/PA3 to those headers.

## Final Validation Checklist

```console
[ ] Project imports successfully
[ ] chip_headers is available
[ ] C/C++ Build include paths are configured
[ ] STM32F091xC is defined
[ ] Project builds and creates 2-UartTx.elf
[ ] Board flashes successfully through ST-LINK
[ ] Terminal receives repeated A at 9600 baud
[ ] Logic analyzer decodes 0x41 using Async Serial
```
