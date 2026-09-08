# 0-LedToggling - GPIO Output

This project is the first bare-metal GPIO output example. It enables the GPIOA peripheral clock, configures PA5 as a digital output, and toggles the NUCLEO-F091RC user LED forever.

## Target

```console
Board:      NUCLEO-F091RC
MCU:        STM32F091RCTx
Peripheral: GPIOA
Pin:        PA5
Function:   Digital output
Expected:   LD2/user LED blinks
```

## Project Structure

```console
0-LedToggling/
  Inc/
    stm32f091rc_gpioa.h
    stm32f091rc_peripherals.h
    stm32f091rc_rcc.h
  Src/
    main.c
    syscalls.c
    sysmem.c
  Startup/
    startup_stm32f091rctx.s
  STM32F091RCTX_FLASH.ld
```

## STM32CubeIDE Setup

For import, include paths, symbols, build, flash, and ST-LINK troubleshooting, use the shared setup guide in the root README: [STM32CubeIDE Settings](../README.md#required-stm32cubeide-settings) and [Build And Flash](../README.md#build-and-flash).

## Code Walkthrough

The application first enables the GPIOA clock:

```c
RCC->AHBENR |= GPIOAEN;
```

Then it configures PA5 as output mode. Each GPIO pin uses two bits in `MODER`. For PA5, the mode bits are `MODER[11:10]`.

```c
GPIOA->MODER |=  (1U << 10);
GPIOA->MODER &= ~(1U << 11);
```

This produces mode `01`, which means general-purpose output.

The main loop toggles PA5 by XORing the output data register:

```c
while (1)
{
    GPIOA->ODR ^= LED_PIN_NUM;
    for (int i = 0; i < 500000; i++);
}
```

`LED_PIN_NUM` is a bit mask for PA5:

```c
#define LED_PIN_NUM (1U << 5)
```

## Hardware Test

After flashing, LD2 should blink continuously.

![LED hardware output](../docs/images/0-ledtoggling-hardware-output.png)

If you use an oscilloscope or logic analyzer:

```console
Probe/signal -> PA5
Ground       -> Nucleo GND
```

![LED PA5 oscilloscope capture](../docs/images/0-ledtoggling-pa5-oscilloscope.png)

## Troubleshooting

### LED Does Not Blink

Check:

```console
Board is powered
Firmware flashed successfully
PA5 is connected to LD2 on the board
GPIOA clock is enabled before GPIOA register access
PA5 MODER bits are 01
```

### Build Cannot Find Local Header

Make sure `../Inc` exists under:

```console
C/C++ Build > Settings > Tool Settings > MCU/MPU GCC Compiler > Include paths
```

## Professional Note

The delay loop is acceptable for a first register-level exercise, but in optimized Release builds an empty loop can be shortened or removed. For a simple learning delay, prefer:

```c
for (volatile int i = 0; i < 500000; i++);
```

For production firmware, use a hardware timer or SysTick-based delay.
