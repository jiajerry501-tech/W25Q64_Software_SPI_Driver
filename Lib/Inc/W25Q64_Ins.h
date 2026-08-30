#ifndef __W25Q64_INS_H
#define __W25Q64_INS_H

/* ---------------------------------------------------------------------------
 * W25Q64 SPI Flash 芯片指令集中英文对照宏定义 (Instruction Set)
 * --------------------------------------------------------------------------- */
#define W25Q64_WRITE_ENABLE                 0x06  /**< 写使能指令 (Write Enable)，写入/擦除前必须发送 */
#define W25Q64_WRITE_DISABLE                0x04  /**< 写禁止指令 (Write Disable) */
#define W25Q64_READ_STATUS_REGISTER_1       0x05  /**< 读状态寄存器 1 (Read Status Register 1)，用于查询 BUSY 标志位 */
#define W25Q64_PAGE_PROGRAM                 0x02  /**< 页编程/写入指令 (Page Program)，一次最多写入 256 字节 */
#define W25Q64_SECTOR_ERASE_4KB             0x20  /**< 扇区擦除指令 (Sector Erase)，擦除指定地址所在的 4KB 扇区 */
#define W25Q64_READ_DATA                    0x03  /**< 读数据指令 (Read Data)，从指定 24 位地址读取任意长度字节 */
#define W25Q64_JEDEC_ID                     0x9F  /**< 读取 JEDEC ID 指令 (包含 1 字节厂商 ID 与 2 字节设备 ID) */
#define W25Q64_DUMMY_BYTE                   0xFF  /**< 哑元字节 (Dummy Byte)，主控用于驱动 SPI 时钟读取数据 */

#endif