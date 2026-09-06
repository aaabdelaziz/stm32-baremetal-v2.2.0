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

This project includes the CMSIS device header:

```c
#include "stm32f0xx.h"
```

So it requires the shared `chip_headers` project/folder to be imported.

## STM32CubeIDE Import

```console
File > Import... > General > Existing Projects into Workspace
```

![Import existing projects](../docs/images/stm32cubeide-import-existing-projects.png)

Import:

```console
1-ButtonLedControl
chip_headers
```

![Import Button LED project and chip headers](../docs/images/1-buttonledcontrol-import-project.png)

## Required Compiler Include Paths

Configure the real compiler include paths under `C/C++ Build`:

```console
Right-click 1-ButtonLedControl
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Include paths
```

![Button LED compiler include paths page](../docs/images/1-buttonledcontrol-compiler-include-paths-page.png)

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

![Button LED compiler include paths](../docs/images/1-buttonledcontrol-compiler-include-paths.png)

Important:

```console
Wrong: ${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx}
Right: ${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
```

The final `/Include` is required because `stm32f0xx.h` is inside that folder.

## Required Compiler Symbols

Configure symbols under:

```console
Right-click 1-ButtonLedControl
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Preprocessor
```

![Button LED preprocessor symbols page](../docs/images/1-buttonledcontrol-preprocessor-symbols-page.png)

Add:

```console
STM32
STM32F0
STM32F091RCTx
STM32F091xC
NUCLEO_F091RC
```

![Button LED preprocessor symbols](../docs/images/1-buttonledcontrol-preprocessor-symbols.png)

`STM32F091xC` is the important CMSIS device macro. `STM32F091RC` is not the macro used by `stm32f0xx.h`.

## Optional Editor/Indexer Settings

The editor indexer is configured separately:

```console
Right-click 1-ButtonLedControl
Properties
C/C++ General
Paths and Symbols
Includes
GNU C
```

![Button LED indexer include paths](../docs/images/1-buttonledcontrol-indexer-include-paths.png)

Add the same include paths, then add `STM32F091xC` under:

```console
C/C++ General > Paths and Symbols > Symbols > GNU C
```

![Button LED indexer symbols](../docs/images/1-buttonledcontrol-indexer-symbols.png)

## Build And Flash

```console
Project > Clean...
Right-click 1-ButtonLedControl > Build Project
Right-click 1-ButtonLedControl > Debug As > STM32 Cortex-M C/C++ Application
```

![Button LED clean project](../docs/images/1-buttonledcontrol-clean-project.png)
![Button LED build project](../docs/images/1-buttonledcontrol-build-project.png)
![Button LED debug configuration](../docs/images/1-buttonledcontrol-debug-configuration.png)

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
