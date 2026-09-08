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

## Import Old Projects Into STM32CubeIDE 2.2.0

Use this flow when importing older STM32CubeIDE projects, for example projects created with STM32CubeIDE v1.13 and opened later in STM32CubeIDE v2.2.0.

Important workspace rule:

```console
Use a separate STM32CubeIDE workspace folder.
Do not use the Git repository root as the workspace itself.
```

### Step 1: Open The Import Menu

From the top menu:

```console
File > Import...
```

![STM32CubeIDE File Import menu](docs/images/stm32cubeide-import-01-file-menu.png)

### Step 2: Select Existing Eclipse Projects

In the Import dialog, choose:

```console
General > Existing Projects into Workspace
```

Then press:

```console
Next
```

![STM32CubeIDE Existing Projects into Workspace](docs/images/stm32cubeide-import-02-existing-projects-wizard.png)

### Step 3: Browse To The Old Project Repository Root

Choose:

```console
Select root directory
Browse...
```

Then select the folder that contains the project folders, for example:

```console
bare-metal-stm32/
```

![STM32CubeIDE select import root folder](docs/images/stm32cubeide-import-04-select-root-folder.png)

### Step 4: Select Projects To Import

Enable:

```console
Search for nested projects
Copy projects into workspace
```

Select the firmware projects and the shared header project:

```console
0-LedToggling
1-ButtonLedControl
2-UartTx
3-ADC
4-UartTxRx
chip_headers
```

Then press:

```console
Finish
```

![STM32CubeIDE selected projects for import](docs/images/stm32cubeide-import-05-select-projects-copy-to-workspace.png)

### Step 5: If CubeIDE Says Projects Already Exist

If the Import dialog says:

```console
Some projects cannot be imported because they already exist in the workspace
```

then the project names are already present in the current workspace. You have three clean options:

```console
Use a fresh workspace
Delete the old project from the workspace without deleting files from disk
Uncheck projects that already exist and import only the missing ones
```

![STM32CubeIDE projects already exist warning](docs/images/stm32cubeide-import-03-projects-already-exist.png)

### Step 6: Confirm Project Explorer

After import, Project Explorer should show the project folders and `chip_headers`.

![STM32CubeIDE Project Explorer after import](docs/images/stm32cubeide-import-06-project-explorer-result.png)

For this repository, `chip_headers` is important because projects using CMSIS include:

```c
#include "stm32f0xx.h"
```

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
