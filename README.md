# STM32 Bare-Metal Projects for STM32CubeIDE 2.2.0

This repository contains focused bare-metal examples for the STM32F091RC / NUCLEO-F091RC board. Each project demonstrates one peripheral concept using direct register programming, CMSIS headers, and STM32CubeIDE project metadata.

## Target Platform

```console
Board:      NUCLEO-F091RC
MCU:        STM32F091RCTx
Core:       Arm Cortex-M0
IDE:        STM32CubeIDE 2.2.0
Toolchain:  GNU Tools for STM32
Debugger:   ST-LINK over SWD
Style:      Bare-metal register programming
```

## Repository Layout

```console
.
├── 0-LedToggling/
├── 1-ButtonLedControl/
├── 2-UartTx/
├── 3-ADC/
├── 4-UartTxRx/
├── chip_headers/
├── docs/
│   └── images/
└── README.md
```

`chip_headers` contains the CMSIS core and STM32F0 device headers required by projects that include `stm32f0xx.h`.

## Peripheral Guides

| Project | Peripheral | What It Demonstrates | Dedicated Guide |
| --- | --- | --- | --- |
| `0-LedToggling` | GPIO output | Enable GPIOA and toggle PA5 / LD2 | [0-LedToggling/README.md](0-LedToggling/README.md) |
| `1-ButtonLedControl` | GPIO input/output | Read PC13 user button and drive PA5 LED | [1-ButtonLedControl/README.md](1-ButtonLedControl/README.md) |
| `2-UartTx` | USART2 transmit | Send ASCII `A` on PA2 at 9600 baud | [2-UartTx/README.md](2-UartTx/README.md) |
| `3-ADC` | ADC1 | Read internal temperature sensor channel 16 | [3-ADC/README.md](3-ADC/README.md) |
| `4-UartTxRx` | USART2 transmit/receive | Receive terminal text on PA3 and echo it on PA2 | [4-UartTxRx/README.md](4-UartTxRx/README.md) |

Each peripheral README includes:

```console
Purpose and hardware behavior
STM32CubeIDE import/configuration steps
Required include paths and symbols
Build, flash, and test procedure
Register-level code walkthrough
Troubleshooting notes
Screenshot placeholders under docs/images
```

## Troubleshooting History

The full warning/error log with root causes and fixes is documented here:

[docs/troubleshooting-and-warnings.md](docs/troubleshooting-and-warnings.md)

## Quick Start

Use an external STM32CubeIDE workspace folder. Do not use this repository root as the workspace.

```console
File > Import... > General > Existing Projects into Workspace
```

![Import existing projects](docs/images/stm32cubeide-import-existing-projects.png)

Select the repository root and import:

```console
0-LedToggling
1-ButtonLedControl
2-UartTx
3-ADC
4-UartTxRx
chip_headers
```

![Import selected projects](docs/images/stm32cubeide-import-selected-projects.png)

## Required STM32CubeIDE Settings

For projects that include `stm32f0xx.h`, configure the real compiler settings here:

```console
Right-click project
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Include paths
```

![Compiler include paths page](docs/images/stm32cubeide-compiler-include-paths-page.png)

Use `Configuration: All configurations`, then add:

```console
../Inc
${workspace_loc:/chip_headers/CMSIS/Device/ST/STM32F0xx/Include}
${workspace_loc:/chip_headers/CMSIS/Include}
```

![Required compiler include paths](docs/images/stm32cubeide-required-compiler-include-paths.png)

Configure symbols here:

```console
Right-click project
Properties
C/C++ Build
Settings
Tool Settings
MCU/MPU GCC Compiler
Preprocessor
```

![Compiler preprocessor symbols page](docs/images/stm32cubeide-compiler-preprocessor-symbols-page.png)

Add:

```console
STM32
STM32F0
STM32F091RCTx
STM32F091xC
NUCLEO_F091RC
```

![Required compiler symbols](docs/images/stm32cubeide-required-compiler-symbols.png)

Important distinction:

```console
C/C++ Build   = real compiler and linker settings
C/C++ General = editor/indexer/autocomplete settings only
```

## Build And Flash

After changing include paths or symbols:

```console
Project > Clean...
Right-click project > Build Project
Right-click project > Debug As > STM32 Cortex-M C/C++ Application
```

![Clean project dialog](docs/images/stm32cubeide-clean-project-dialog.png)
![Build project command](docs/images/stm32cubeide-build-project-command.png)
![Debug as STM32 Cortex-M application](docs/images/stm32cubeide-debug-as-cortex-m-application.png)

Recommended debug settings:

```console
Application: Debug/<project-name>.elf
Interface:   SWD
Reset mode:  Connect under reset
SWV:         Disabled
```

![Debug configuration settings](docs/images/stm32cubeide-debug-configuration-settings.png)

## Documentation Images

All screenshots and hardware captures are expected under:

```console
docs/images
```

The Markdown image links are intentionally kept even when a file is not present yet. Add future screenshots using the same filenames to complete the visual documentation.

## Repository Hygiene

Commit source, project metadata, linker scripts, startup files, CMSIS headers, README files, and documentation images.

Do not commit generated workspace or build output:

```console
.metadata/
Debug/
Release/
*.o
*.elf
*.hex
*.bin
*.map
*.list
*.d
*.su
*.cyclo
```

The repository `.gitignore` is configured for this STM32CubeIDE workflow.
