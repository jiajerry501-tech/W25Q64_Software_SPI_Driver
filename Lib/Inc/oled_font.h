#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

/**
  * @brief 8x16 像素全 ASCII 点阵字模表 (包含 ASCII 32 ' ' 至 126 '~' 共 95 个字符)
  * @note  每个字符占用 16 字节，前 8 字节为上半部分(8像素高)，后 8 字节为下半部分(8像素高)
  */
extern const uint8_t OLED_F8x16[95][16];

#endif