# 3-ADC - ADC1 Internal Temperature Sensor

This project demonstrates bare-metal ADC configuration on STM32F091RC. It enables ADC1, selects the internal temperature sensor channel, starts conversions, and calculates an approximate temperature value.

## Target

```console
Board:       NUCLEO-F091RC
MCU:         STM32F091RCTx
Peripheral:  ADC1
Channel:     ADC_IN16
Signal:      Internal temperature sensor
Resolution:  12-bit
Alignment:   Right-aligned
Expected:    ADC conversion completes repeatedly in firmware
```

## Project Structure

```console
3-ADC/
  Inc/
    adc.h
  Src/
    main.c
    adc.c
    syscalls.c
    sysmem.c
  Startup/
    startup_stm32f091rctx.s
  STM32F091RCTX_FLASH.ld
```

This project includes:

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
3-ADC
chip_headers
```

![Import ADC project and chip headers](../docs/images/3-adc-import-project.png)

## Required Compiler Include Paths

Configure the real compiler include paths:

```console
Right-click 3-ADC
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Include paths
```

![ADC compiler include paths page](../docs/images/3-adc-compiler-include-paths-page.png)

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

![ADC compiler include paths](../docs/images/3-adc-compiler-include-paths.png)

## Required Compiler Symbols

Configure symbols under:

```console
Right-click 3-ADC
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Preprocessor
```

![ADC preprocessor symbols page](../docs/images/3-adc-preprocessor-symbols-page.png)

Add:

```console
STM32
STM32F0
STM32F091RCTx
STM32F091xC
NUCLEO_F091RC
```

![ADC preprocessor symbols](../docs/images/3-adc-preprocessor-symbols.png)

## Optional Editor/Indexer Settings

Configure the editor indexer only after the compiler settings are correct:

```console
Right-click 3-ADC
Properties
C/C++ General
Paths and Symbols
Includes
GNU C
```

![ADC indexer include paths](../docs/images/3-adc-indexer-include-paths.png)

Add the same include paths and add `STM32F091xC` under:

```console
C/C++ General > Paths and Symbols > Symbols > GNU C
```

![ADC indexer symbols](../docs/images/3-adc-indexer-symbols.png)

## Build And Flash

```console
Project > Clean...
Right-click 3-ADC > Build Project
Right-click 3-ADC > Debug As > STM32 Cortex-M C/C++ Application
```

![ADC clean project](../docs/images/3-adc-clean-project.png)
![ADC build project](../docs/images/3-adc-build-project.png)
![ADC debug configuration](../docs/images/3-adc-debug-configuration.png)

## Code Walkthrough

The application initializes ADC once, then reads repeatedly:

```c
int main(void)
{
    pa1_adc_init();

    while (1)
    {
        adc_internal_temp_read();
    }
}
```

The initialization enables the GPIOA clock and configures PA1 analog mode:

```c
RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
GPIOA->MODER |= GPIO_MODER_MODER1;
```

For the internal temperature sensor channel, PA1 is not the measured input. The active ADC channel is internal channel 16:

```c
ADC1->CHSELR = ADC_CHSELR_CHSEL16;
```

ADC1 peripheral clock is enabled on APB2:

```c
RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
```

The internal temperature sensor and voltage reference are enabled:

```c
ADC->CCR |= ADC_CCR_TSEN | ADC_CCR_VREFEN;
```

ADC calibration is started and the firmware waits until it completes:

```c
ADC1->CR |= ADC_CR_ADCAL;
while (ADC1->CR & ADC_CR_ADCAL);
```

ADC resolution and alignment are configured:

```c
ADC1->CFGR1 &= ~ADC_CFGR1_RES;
ADC1->CFGR1 &= ~ADC_CFGR1_ALIGN;
```

ADC1 is enabled and the code waits for the ready flag:

```c
ADC1->CR |= ADC_CR_ADEN;
while (!(ADC1->ISR & ADC_ISR_ADRDY));
```

Each read starts a conversion, waits for end-of-conversion, and reads the data register:

```c
ADC1->CR |= ADC_CR_ADSTART;
while (!(ADC1->ISR & ADC_ISR_EOC));
uint16_t adc_value = ADC1->DR;
```

The example calculates an approximate temperature:

```c
temp_celsius = ((adc_value * 3.3 / 4095.0) - 1.43) / 0.0043 + 25;
```

## Debug Test

Use a breakpoint inside `adc_internal_temp_read()` after:

```c
uint16_t adc_value = ADC1->DR;
```

Watch:

```console
adc_value
ADC1->ISR
ADC1->DR
```

![ADC debugger live expression](../docs/images/3-adc-debugger-live-expression.png)

Expected behavior:

```console
ADC_ISR_EOC becomes set after conversion
ADC1->DR changes with the conversion result
adc_value receives a 12-bit sample from 0 to 4095
```

## Troubleshooting

### `stm32f0xx.h: No such file or directory`

Add:

```console
${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
```

### `core_cm0.h: No such file or directory`

Add:

```console
${workspace_loc:/chip_headers/CMSIS/Include}
```

### Device Selection Error

Add:

```console
STM32F091xC
```

### Cannot Watch `temp_celsius`

`temp_celsius` is currently a local variable inside `adc_internal_temp_read()`. For easier debugger inspection, make it global and volatile:

```c
volatile float temp_celsius;
```

Then remove the local declaration:

```c
float temp_celsius = 0.0;
```

## Professional Notes

The function name `pa1_adc_init()` is misleading because the selected channel is the internal temperature sensor, not PA1. A clearer future name would be:

```c
adc_internal_temperature_init()
```

For accurate temperature measurement, use the STM32 factory calibration constants instead of fixed voltage/slope assumptions.
