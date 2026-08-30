#ifndef __MYSPI_H
#define __MYSPI_H

#include "stm32f1xx_hal.h"

/* ---------------------------------------------------------------------------
 * 软件 SPI 引脚定义（根据硬件接线修改）
 * 默认使用 STM32F103 的 PA4~PA7 引脚模拟 SPI 通信
 * --------------------------------------------------------------------------- */
#define SPI_CS_PORT    GPIOA        /**< CS 片选引脚端口 */
#define SPI_CS_PIN     GPIO_PIN_4   /**< CS 片选引脚编号 (PA4) */

#define SPI_SCK_PORT   GPIOA        /**< SCK 时钟引脚端口 */
#define SPI_SCK_PIN    GPIO_PIN_5   /**< SCK 时钟引脚编号 (PA5) */

#define SPI_MISO_PORT  GPIOA        /**< MISO 主机输入/从机输出端口 */
#define SPI_MISO_PIN   GPIO_PIN_6   /**< MISO 引脚编号 (PA6) */

#define SPI_MOSI_PORT  GPIOA        /**< MOSI 主机输出/从机输入端口 */
#define SPI_MOSI_PIN   GPIO_PIN_7   /**< MOSI 引脚编号 (PA7) */

/* ---------------------------------------------------------------------------
 * 软件 SPI 引脚高低电平快速读写控制宏
 * --------------------------------------------------------------------------- */
/**
 * @brief 写 CS 片选引脚电平
 * @param x 1: 拉高 (取消选中); 0: 拉低 (选中设备)
 */
#define MySPI_W_CS(x)   HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/**
 * @brief 写 SCK 时钟引脚电平
 * @param x 1: 输出高电平; 0: 输出低电平
 */
#define MySPI_W_SCK(x)  HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/**
 * @brief 写 MOSI 主控输出数据线电平
 * @param x 1: 输出高电平; 0: 输出低电平
 */
#define MySPI_W_MOSI(x) HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/**
 * @brief 读取 MISO 主控输入数据线电平
 * @return GPIO_PIN_SET (高电平) 或 GPIO_PIN_RESET (低电平)
 */
#define MySPI_R_MISO()  HAL_GPIO_ReadPin(SPI_MISO_PORT, SPI_MISO_PIN)

/* ---------------------------------------------------------------------------
 * 函数声明
 * --------------------------------------------------------------------------- */

/**
 * @brief  软件模拟 SPI 引脚初始化及初始电平配置（配置为 SPI 模式 0 默认电平）
 */
void MySPI_Init(void);

/**
 * @brief  产生 SPI 起始信号（拉低 CS 片选信号线）
 */
void MySPI_Start(void);

/**
 * @brief  产生 SPI 停止信号（拉高 CS 片选信号线）
 */
void MySPI_Stop(void);

/**
 * @brief  SPI 模式 0 全双工交换一个字节数据 (MSB 优先)
 * @param  ByteSend 准备发送给从机的 8 位数据
 * @return 从机返回的 8 位数据
 */
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif