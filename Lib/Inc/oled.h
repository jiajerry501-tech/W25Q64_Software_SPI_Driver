#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdint.h>

/**
  * @brief  初始化 OLED 显示屏 (SSD1306 芯片控制器配置)
  * @retval 无
  */
void OLED_Init(void);

/**
  * @brief  OLED 全屏清屏 (将所有显存 Page 写入 0x00)
  * @retval 无
  */
void OLED_Clear(void);

/**
  * @brief  在 OLED 指定位置显示单个字符 (8x16 字体)
  * @param  line: 行号，取值范围 1~4
  * @param  column: 列号，取值范围 1~16
  * @param  ch: 要显示的 ASCII 字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t line, uint8_t column, char ch);

/**
  * @brief  在 OLED 指定位置显示字符串 (8x16 字体)
  * @param  line: 起始行号，取值范围 1~4
  * @param  column: 起始列号，取值范围 1~16
  * @param  str: 要显示的字符串指针
  * @retval 无
  */
void OLED_ShowString(uint8_t line, uint8_t column, char *str);

/**
  * @brief  在 OLED 指定位置显示带符号的十进制整数 (8x16 字体)
  * @param  line: 行号，取值范围 1~4
  * @param  column: 列号，取值范围 1~16
  * @param  num: 要显示的带符号整数，如 -100 ~ 100
  * @param  length: 数字位长 (不包含正负号)
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int16_t num, uint8_t length);

/**
  * @brief  在 OLED 指定位置显示无符号十进制数字
  */
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length);

/**
  * @brief  在 OLED 指定位置显示十六进制数字
  */
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length);

/**
  * @brief  在 OLED 指定位置显示二进制数字
  */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length);

#endif