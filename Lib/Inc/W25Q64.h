#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f1xx_hal.h"

/**
 * @brief  初始化 W25Q64 SPI 接口及底层 GPIO
 */
void W25Q64_Init(void);

/**
 * @brief  读取 W25Q64 的 JEDEC ID (厂商 ID 与 设备 ID)
 * @param  MID 输出参数：厂商 ID (Winbond 为 0xEF)
 * @param  DID 输出参数：设备 ID (W25Q64 为 0x4017)
 */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);

/**
 * @brief  发送写使能指令 (0x06)，将芯片 WEL 标志位置 1
 * @note   所有页编程 (Page Program) 与 扇区擦除 (Sector Erase) 前必须调用此函数
 */
void W25Q64_WriteEnable(void);

/**
 * @brief  等待 W25Q64 内部擦除或写入操作完成 (等待 Status Register 1 的 BUSY 标志位变为 0)
 */
void W25Q64_WaitBusy(void);

/**
 * @brief  向 W25Q64 指定 24 位地址写入数据 (页编程 Page Program)
 * @param  Address 写入的目标首地址 (范围 0x000000 ~ 0x7FFFFF)
 * @param  DataArray 要写入的数据数组首地址
 * @param  Count 写入的数据字节数 (单次不超过 256 字节，且不可跨越页边界)
 */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);

/**
 * @brief  擦除 W25Q64 指定 24 位地址所在的 4KB 扇区 (Sector Erase)
 * @param  Address 目标扇区内部的任意 24 位地址
 */
void W25Q64_SectorErase(uint32_t Address);

/**
 * @brief  从 W25Q64 指定 24 位地址连续读取数据 (Read Data)
 * @param  Address 读取的目标首地址 (范围 0x000000 ~ 0x7FFFFF)
 * @param  DataArray 存放读取数据的接收数组首地址
 * @param  Count 要读取的数据字节数
 */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

#endif