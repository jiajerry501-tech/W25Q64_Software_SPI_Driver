# W25Q64 STM32 软件模拟 SPI 驱动程序 (Software SPI Driver)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-STM32F103-orange.svg)](https://www.st.com/)
[![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)](CMakeLists.txt)

基于 STM32 HAL 库开发的 **W25Q64 Flash 存储芯片软件模拟 SPI 驱动程序**。本项目由 STM32CubeMX 生成框架，采用 CMake 进行构建，硬件底层驱动完全解耦，可快速移植至 STM32 全系列单片机或其他嵌入式平台。

---

## 🌟 项目特性

- **软件模拟 SPI 协议 (Bit-Banging)**：采用 SPI Mode 0 (CPOL = 0, CPHA = 0)，引脚定义灵活，无需依赖硬件 SPI 外设。
- **完整的 W25Q64 操作 API**：
  - 读取 JEDEC ID（校验厂商 ID `0xEF` 与设备 ID `0x4017`）
  - 扇区擦除（4KB Sector Erase `0x20`）
  - 页编程写入（Page Program `0x02`，单次最高 256 字节）
  - 连续数据读取（Read Data `0x03`）
  - 状态寄存器查询与 Busy 标志等待（带超时防死锁保护）
- **自带 OLED 显示验证 demo**：集成 0.96 吋 OLED 驱动，直观展示 JEDEC ID 读取结果与读写数据校验结果。
- **工程规范**：代码结构清晰，提供完整的 Doxygen 中文注释与 CMake 构建支持。

---

## 📁 目录结构

```text
W25Q64_Software_SPI_Driver/
├── CMakeLists.txt          # CMake 构建配置文件
├── W25Q64_Software_SPI_Driver.ioc # STM32CubeMX 工程配置文件
├── Core/                   # STM32CubeMX 核心代码 (main, gpio, clock 等)
├── Drivers/                # STM32 HAL 库及 CMSIS 驱动文件
└── Lib/                    # 扩展驱动库 (W25Q64 & OLED)
    ├── Inc/
    │   ├── MySPI.h         # 软件 SPI 底层引脚定义与宏函数
    │   ├── W25Q64.h        # W25Q64 驱动头文件及 API 声明
    │   ├── W25Q64_Ins.h    # W25Q64 指令集定义
    │   ├── oled.h          # OLED 显示屏驱动头文件
    │   └── oled_font.h     # OLED 字库文件
    └── Src/
        ├── MySPI.c         # 软件 SPI 字节交换与时序实现
        ├── W25Q64.c        # W25Q64 读写/擦除/ID读取逻辑实现
        ├── oled.c          # OLED 驱动实现
        └── oled_font.c     # OLED 字库数据
```

---

## 🔌 硬件连接 (Pinout)

默认硬件引脚配置如下（可在 `Lib/Inc/MySPI.h` 中根据需求自由修改）：

### W25Q64 Flash 模块连接

| W25Q64 引脚 | STM32F103 引脚 | 功能描述 |
| :--- | :--- | :--- |
| **VCC** | 3.3V | 供电正极 |
| **GND** | GND | 供电负极 |
| **CS** | PA4 | 片选引脚 (Software CS) |
| **CLK / SCK** | PA5 | 时钟引脚 (Software SCK) |
| **DO / MISO** | PA6 | 主机输入/从机输出 (Software MISO) |
| **DI / MOSI** | PA7 | 主机输出/从机输入 (Software MOSI) |

---

## 🚀 快速上手 (Quick Start)

### 1. 克隆仓库

```bash
git clone https://github.com/jiajerry501-tech/W25Q64_Software_SPI_Driver.git
cd W25Q64_Software_SPI_Driver
```

### 2. 编译工程

推荐使用 **VS Code + CMake Tools + arm-none-eabi-gcc** 工具链构建：

```bash
# 生成构建文件
cmake -B build -G Ninja

# 编译项目
cmake --build build
```

编译完成后，会在 `build/` 目录下生成 `W25Q64_Software_SPI_Driver.elf`、`.hex` 和 `.bin` 文件。

---

## 💻 代码示例 (Usage Example)

在 `Core/Src/main.c` 中调用的核心验证逻辑：

```c
#include "W25Q64.h"
#include "oled.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* 1. 初始化外设 */
    OLED_Init();
    W25Q64_Init();

    uint8_t MID = 0;
    uint16_t DID = 0;
    uint8_t ArrayWrite[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t ArrayRead[4] = {0};

    /* 2. 读取芯片 ID */
    W25Q64_ReadID(&MID, &DID); // 正常获取: MID=0xEF, DID=0x4017

    /* 3. 擦除扇区与写入测试 */
    W25Q64_SectorErase(0x000000);               // 擦除首扇区 (4KB)
    W25Q64_PageProgram(0x000000, ArrayWrite, 4); // 页编程写入 4 字节数据

    /* 4. 读回校验 */
    W25Q64_ReadData(0x000000, ArrayRead, 4);

    /* 5. 显示测试结果 */
    OLED_ShowHexNum(1, 1, MID, 2);   // 显示厂商 ID: EF
    OLED_ShowHexNum(1, 4, DID, 4);   // 显示设备 ID: 4017
    OLED_ShowHexNum(2, 1, ArrayRead[0], 2); // 01
    OLED_ShowHexNum(2, 4, ArrayRead[1], 2); // 02
    OLED_ShowHexNum(2, 7, ArrayRead[2], 2); // 03
    OLED_ShowHexNum(2, 10, ArrayRead[3], 2); // 04

    while (1) {
    }
}
```

---

## 🛠 API 接口说明

### `MySPI.h` (底层 SPI 时序)
- `void MySPI_Init(void)`：初始化 GPIO 引脚为 Mode 0 默认状态。
- `uint8_t MySPI_SwapByte(uint8_t ByteSend)`：软件模拟 SPI 全双工字节交换。

### `W25Q64.h` (Flash 应用接口)
- `void W25Q64_Init(void)`：初始化 W25Q64 通信接口。
- `void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)`：读取 JEDEC 厂商与设备 ID。
- `void W25Q64_WriteEnable(void)`：发送 0x06 写使能指令。
- `void W25Q64_WaitBusy(void)`：等待 Status Register 1 的 BUSY 标志位清零。
- `void W25Q64_SectorErase(uint32_t Address)`：4KB 扇区擦除。
- `void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)`：页编程写入（最多 256 字节/页）。
- `void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)`：连续数据读取。

---

## 📄 开源许可证

本项目基于 [MIT License](LICENSE) 开源，欢迎自由使用、修改和二次分发。
