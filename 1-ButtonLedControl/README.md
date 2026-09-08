# 1-ButtonLedControl - GPIO Input And Output

This project demonstrates basic GPIO input and output. It reads the NUCLEO-F091RC user button on PC13 and controls the user LED on PA5.

## Target

```console
Board:       NUCLEO-F091RC
MCU:         STM32F091RCTx
Input pin:   PC13 user button
Output pin:  PA5 user LED / LD2
Button type: Active-low
Expected:    Press button -> LED on; release button -> LED off
```

## Project Structure

```console
1-ButtonLedControl/
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

This project uses the shared CMSIS `chip_headers` folder. For import, include paths, symbols, build, flash, and ST-LINK troubleshooting, use the shared setup guide in the root README: [STM32CubeIDE Settings](../README.md#required-stm32cubeide-settings) and [Build And Flash](../README.md#build-and-flash).

## Code Walkthrough

The application enables both GPIO ports:

```c
RCC->AHBENR |= GPIOAEN;
RCC->AHBENR |= GPIOCEN;
```

PA5 is configured as output:

```c
GPIOA->MODER &= ~(3U << (2 * LED_PIN));
GPIOA->MODER |=  (1U << (2 * LED_PIN));
```

PC13 is configured as input:

```c
GPIOC->MODER &= ~(3U << (2 * BTN_PIN));
```

The user button is active-low, so pressed means the PC13 input bit reads `0`:

```c
if (!(GPIOC->IDR & BTN_MASK)) {
    GPIOA->BSRR = LED_ON;
} else {
    GPIOA->BSRR = LED_OFF;
}
```

`BSRR` is used instead of writing `ODR` directly:

```c
#define LED_ON  (1U << LED_PIN)
#define LED_OFF (1U << (LED_PIN + 16))
```

Writing to the lower half of `BSRR` sets the pin. Writing to the upper half resets the pin.

## Hardware Test

Flash the project, then press and release the Nucleo user button.

Expected behavior:

```console
Button pressed  -> PA5/LD2 on
Button released -> PA5/LD2 off
```

![Button LED hardware test](../docs/images/1-buttonledcontrol-hardware-test.png)

Optional logic analyzer/oscilloscope check:

```console
Channel 0 -> PA5
Channel 1 -> PC13
Ground    -> Nucleo GND
```

![Button LED analyzer capture](../docs/images/1-buttonledcontrol-analyzer-capture.png)

## Troubleshooting

### `stm32f0xx.h: No such file or directory`

Add the CMSIS device include path:

```console
${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
```

### `core_cm0.h: No such file or directory`

Add the CMSIS core include path:

```console
${workspace_loc:/chip_headers/CMSIS/Include}
```

### Device Selection Error

Add this compiler symbol:

```console
STM32F091xC
```

### LED Logic Is Reversed

The button is active-low. A pressed button reads `0`, not `1`.
