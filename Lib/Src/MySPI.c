#include "MySPI.h"

/**
 * @brief  软件 SPI 默认引脚电平初始化
 * @note   设置 SPI 模式 0 默认状态：CS 为高电平（未选中），SCK 为低电平（空闲电平为低）
 */
void MySPI_Init(void) {
    MySPI_W_CS(1);   // 默认拉高 CS，不选中芯片
    MySPI_W_SCK(0);  // SPI 模式 0：CPOL = 0，空闲时 SCK 保持低电平
}

/**
 * @brief  产生 SPI 起始信号
 * @note   SPI 通信开始：将 CS 片选引脚拉低
 */
void MySPI_Start(void) {
    MySPI_W_CS(0);
}

/**
 * @brief  产生 SPI 停止信号
 * @note   SPI 通信结束：将 CS 片选引脚拉高，释放总线
 */
void MySPI_Stop(void) {
    MySPI_W_CS(1);
}

/**
 * @brief  SPI 模式 0 交换一个字节数据（CPOL = 0, CPHA = 0，高位 MSB 先发）
 * @param  ByteSend 主机需要发送给从机的 8 位字节数据
 * @return ByteReceive 主机从从机接收到的 8 位字节数据
 * @note   SPI 模式 0 时序说明：
 *         - SCK 空闲为低电平；
 *         - 第一边沿（上升沿）进行数据采样（主机读 MISO，从机读 MOSI）；
 *         - 第二边沿（下降沿）进行数据移位/切换（准备下一个 bit）。
 */
uint8_t MySPI_SwapByte(uint8_t ByteSend) {
    uint8_t ByteReceive = 0x00;
    
    for (uint8_t i = 0; i < 8; i++) {
        /* 1. 下降沿后/采样前：主机向 MOSI 输出当前最高位 (MSB) 数据 */
        MySPI_W_MOSI(ByteSend & (0x80 >> i)); 
        __NOP();                              // 时钟延序建立（防高主频下电平翻转过快）

        /* 2. 时钟线拉高（上升沿）：从机采样 MOSI 数据，主机采样 MISO 数据 */
        MySPI_W_SCK(1);                       
        __NOP();                              // 等待从机 MISO 保持电平稳定

        /* 3. 主机读取从机在 MISO 上输出的 Bit */
        if (MySPI_R_MISO() == GPIO_PIN_SET) {
            ByteReceive |= (0x80 >> i);
        }

        /* 4. 时钟线拉低（下降沿）：从机和主机准备切换输出下一个 Bit */
        MySPI_W_SCK(0);                       
        __NOP();
    }
    
    return ByteReceive;
}