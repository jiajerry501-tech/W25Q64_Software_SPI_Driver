#include "W25Q64.h"
#include "W25Q64_Ins.h"
#include "MySPI.h"

/**
 * @brief  初始化 W25Q64 接口
 */
void W25Q64_Init(void) {
    MySPI_Init();
}

/**
 * @brief  读取 W25Q64 芯片的 JEDEC ID
 * @param  MID 厂商 ID (Winbond 为 0xEF)
 * @param  DID 设备 ID (W25Q64 Memory Type 0x40 + Capacity 0x17 -> 0x4017)
 * @note   指令序列: 0x9F -> 读 MID -> 读 DID高字节 -> 读 DID低字节
 */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID) {
    MySPI_Start();                             // 拉低 CS 片选，开始通信
    MySPI_SwapByte(W25Q64_JEDEC_ID);           // 发送 JEDEC ID 读取指令 (0x9F)
    *MID = MySPI_SwapByte(W25Q64_DUMMY_BYTE);  // 发送哑元字节，交换接收 1 字节厂商 ID (0xEF)
    *DID = MySPI_SwapByte(W25Q64_DUMMY_BYTE) << 8; // 接收 1 字节设备类型 (0x40) 并左移到高字节
    *DID |= MySPI_SwapByte(W25Q64_DUMMY_BYTE); // 接收 1 字节容量标识 (0x17) 并合成低字节
    MySPI_Stop();                              // 拉高 CS 片选，结束通信
}

/**
 * @brief  发送写使能指令 (Write Enable)
 * @note   在任何写入（Page Program）或擦除（Sector Erase）指令发送前，必须先发送写使能
 */
void W25Q64_WriteEnable(void) {
    MySPI_Start();                             // 拉低 CS
    MySPI_SwapByte(W25Q64_WRITE_ENABLE);       // 发送写使能指令 (0x06)
    MySPI_Stop();                              // 拉高 CS
}

/**
 * @brief  等待 W25Q64 芯片内部状态恢复空闲 (BUSY 标志位清零)
 * @note   芯片在执行擦除或写入期间，Status Register 1 的 Bit 0 (BUSY) 会维持 1。
 *         连续读取 Status Register 1 状态直至 BUSY 位为 0。
 */
void W25Q64_WaitBusy(void) {
    uint32_t Timeout = 1000000;                // 设置防死锁超时计数值
    MySPI_Start();                             // 拉低 CS
    MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1); // 发送读状态寄存器 1 指令 (0x05)
    
    /* 保持 CS 为低电平，发送 Dummy Byte，从机将持续输出最新的 Status Register 1 字节 */
    while ((MySPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01) {
        if (--Timeout == 0) break;              // 超时退出机制
    }
    MySPI_Stop();                              // 拉高 CS
}

/**
 * @brief  4KB 扇区擦除 (Sector Erase)
 * @param  Address 擦除的目标 24 位地址 (如 0x000000)
 * @note   Flash 擦除特点：只能将 bit 1 改写为 0 (0xFF 表示全被擦除)，写入前必须先擦除
 */
void W25Q64_SectorErase(uint32_t Address) {
    W25Q64_WriteEnable();                      // 1. 发送写使能
    MySPI_Start();                             // 2. 拉低 CS
    MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);   // 3. 发送扇区擦除指令 (0x20)
    MySPI_SwapByte((Address >> 16) & 0xFF);    // 4. 发送 24 位地址最高字节 (Address[23:16])
    MySPI_SwapByte((Address >> 8) & 0xFF);     // 5. 发送 24 位地址中间字节 (Address[15:8])
    MySPI_SwapByte(Address & 0xFF);            // 6. 发送 24 位地址最低字节 (Address[7:0])
    MySPI_Stop();                              // 7. 拉高 CS 触发内部擦除
    W25Q64_WaitBusy();                         // 8. 阻塞等待擦除完成
}

/**
 * @brief  页编程写入 (Page Program)
 * @param  Address 写入的目标 24 位首地址
 * @param  DataArray 要写入的数据数组
 * @param  Count 写入的字节数 (1~256 字节)
 * @note   注意：一次页编程不可跨越 256 字节页边界！
 */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count) {
    W25Q64_WriteEnable();                      // 1. 发送写使能
    MySPI_Start();                             // 2. 拉低 CS
    MySPI_SwapByte(W25Q64_PAGE_PROGRAM);       // 3. 发送页编程指令 (0x02)
    MySPI_SwapByte((Address >> 16) & 0xFF);    // 4. 发送 24 位地址最高字节
    MySPI_SwapByte((Address >> 8) & 0xFF);     // 5. 发送 24 位地址中间字节
    MySPI_SwapByte(Address & 0xFF);            // 6. 发送 24 位地址最低字节
    
    /* 7. 循环发送准备写入的数据 */
    for (uint16_t i = 0; i < Count; i++) {
        MySPI_SwapByte(DataArray[i]);
    }
    
    MySPI_Stop();                              // 8. 拉高 CS 触发内部编程
    W25Q64_WaitBusy();                         // 9. 阻塞等待写入完成
}

/**
 * @brief  连续读取 Flash 数据 (Read Data)
 * @param  Address 读取的 24 位首地址
 * @param  DataArray 接收数据缓冲区
 * @param  Count 读取的字节数
 */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count) {
    MySPI_Start();                             // 1. 拉低 CS
    MySPI_SwapByte(W25Q64_READ_DATA);          // 2. 发送读数据指令 (0x03)
    MySPI_SwapByte((Address >> 16) & 0xFF);    // 3. 发送 24 位地址高字节
    MySPI_SwapByte((Address >> 8) & 0xFF);     // 4. 发送 24 位地址中字节
    MySPI_SwapByte(Address & 0xFF);            // 5. 发送 24 位地址低字节
    
    /* 6. 连续发送 Dummy Byte 驱动 SPI 时钟并读取数据 */
    for (uint32_t i = 0; i < Count; i++) {
        DataArray[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
    }
    
    MySPI_Stop();                              // 7. 拉高 CS 结束读取
}