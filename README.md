# STM32 Optical PAM4 Abstraction Layer Driver

[![License: Apache2.0](https://img.shields.io/badge/License-Apache2.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![Maintainer](https://img.shields.io/badge/Maintainer-Luc_Creeper74-orange)](https://github.com/Lucreeper74)

This project is a driver, it provide an abstraction layer alongside with the HAL provided by STMicroelectronics for optical communication using the PAM4 modulation scheme.

Copyright (c) 2025 Luc_Creeper74 - STM32_OPAL_Driver is under the Apache 2.0 License.

## Currently supported STM32 MCU Famillies
| MCU Model Familly | Status                     |
| :---------------- | :--------------------------|
| STM32L0xx         | Supported ✅              |
| STM32F3xx         | Supported ✅              |

## Installation

> [!IMPORTANT]
> Template projects already contain the OPAL_Driver as a `git` submodule and can be found here :<br>
> > https://github.com/Lucreeper74/LIFI-Project-Emitter for the Emitter system.<br>
> > https://github.com/Lucreeper74/LIFI-Project-Receiver for the Receiver system.<br>

Make sure to install the latest version of `git`: <br>
_See here for Git installation : https://git-scm.com/book/en/v2/Getting-Started-Installing-Git_ 

In the Driver directory of a STM32 project, get the latest version of this repository using `git`: <br>

```bash
$ git clone https://github.com/Lucreeper74/STM32_OPAL_Driver
```
_Or download manual project files using the Green "Code" button above and unzip the file into the Driver directory._

> [!IMPORTANT]
> You need to include all the driver source and header files into `CMakeList.txt` if you using `Cmake`.<br>
> Make sure to define the definition in the [CMakeList.txt](https://github.com/Lucreeper74/LIFI-Project-Emitter/blob/main/CMakeLists.txt) in the `target_compile_definitions` in your project.
