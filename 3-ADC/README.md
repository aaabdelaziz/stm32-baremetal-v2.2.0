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

## STM32CubeIDE Setup

This project uses the shared CMSIS `chip_headers` folder. For import, include paths, symbols, build, flash, and ST-LINK troubleshooting, use the shared setup guide in the root README: [STM32CubeIDE Settings](../README.md#required-stm32cubeide-settings) and [Build And Flash](../README.md#build-and-flash).

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
