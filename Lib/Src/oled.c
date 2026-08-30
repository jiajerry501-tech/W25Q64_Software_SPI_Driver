#include "oled.h"
#include "oled_font.h"

/* GPIO 引脚操作宏定义：软件模拟 I2C 的 SCL 时钟线和 SDA 数据线 */
#define OLED_SCL_Clr()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET) // SCL 引脚拉低
#define OLED_SCL_Set()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET)   // SCL 引脚拉高
#define OLED_SDA_Clr()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET) // SDA 引脚拉低
#define OLED_SDA_Set()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET)   // SDA 引脚拉高

/**
 * @brief  软件 I2C 微秒级微延时函数
 * @note   用于满足 SSD1306 控制芯片对于 I2C 通信时序的最小高/低电平持续时间要求 (如 400kHz 速度)
 */
static void I2C_Delay(void)
{
    uint32_t i = 10;
    while (i--)
    {
        __NOP();
    }
}

/**
 * @brief  产生 I2C 通信起始信号 (START)
 * @note   时序：在 SCL 为高电平期间，SDA 产生一个从高到低的跳变
 */
static void I2C_Start(void)
{
    OLED_SDA_Set();
    OLED_SCL_Set();
    I2C_Delay();
    OLED_SDA_Clr();
    I2C_Delay();
    OLED_SCL_Clr();
    I2C_Delay();
}

/**
 * @brief  产生 I2C 通信停止信号 (STOP)
 * @note   时序：在 SCL 为高电平期间，SDA 产生一个从低到高的跳变
 */
static void I2C_Stop(void)
{
    OLED_SDA_Clr();
    OLED_SCL_Set();
    I2C_Delay();
    OLED_SDA_Set();
    I2C_Delay();
}

/**
 * @brief  软件模拟 I2C 发送 1 个字节 (8 比特) 数据
 * @param  byte: 待发送的 8 位数据
 */
static void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        /* 从高位 (MSB) 开始依次发送 */
        if (byte & 0x80)
            OLED_SDA_Set();
        else
            OLED_SDA_Clr();
        I2C_Delay();
        OLED_SCL_Set(); // 拉高时钟，告知从机读取 SDA
        I2C_Delay();
        OLED_SCL_Clr(); // 拉低时钟，准备下一个 Bit
        I2C_Delay();
        byte <<= 1;
    }
    /* 发送第 9 个 ACK 时钟周期 (SSD1306 模拟应答) */
    OLED_SCL_Set();
    I2C_Delay();
    OLED_SCL_Clr();
    I2C_Delay();
}

/**
 * @brief  向 OLED 写入控制命令
 * @param  cmd: 命令字节
 * @note   写入 0x78 (器件写地址) -> 写入 0x00 (控制字节，代表后续字节为 Command) -> 写入命令
 */
static void OLED_WriteCommand(uint8_t cmd)
{
    I2C_Start();
    I2C_SendByte(0x78); // OLED 7位从机地址 0x3C 左移 1 位 + 写方向 (0)
    I2C_SendByte(0x00); // Co=0, D/C#=0: 标志后续为控制命令
    I2C_SendByte(cmd);
    I2C_Stop();
}

/**
 * @brief  向 OLED 显存 (RAM) 写入显示数据
 * @param  data: 数据字节 (对应像素点阵)
 * @note   写入 0x78 (器件写地址) -> 写入 0x40 (控制字节，代表后续字节为 Data) -> 写入数据
 */
static void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_SendByte(0x78); // OLED 从机写地址
    I2C_SendByte(0x40); // Co=0, D/C#=1: 标志后续为显存显示数据
    I2C_SendByte(data);
    I2C_Stop();
}

/**
 * @brief  设置 OLED 读写游标 (光标) 位置
 * @param  page: 页地址 (0 ~ 7，对应屏幕垂直方向 8 个 Page)
 * @param  column: 列地址 (0 ~ 127，对应屏幕水平方向 128 列)
 */
static void OLED_SetCursor(uint8_t page, uint8_t column)
{
    OLED_WriteCommand(0xB0 | page);                   // 设置页地址 (0xB0 ~ 0xB7)
    OLED_WriteCommand(0x10 | ((column & 0xF0) >> 4)); // 设置列地址高 4 位
    OLED_WriteCommand(0x00 | (column & 0x0F));        // 设置列地址低 4 位
}

/**
 * @brief  OLED 清屏操作 (将整个 128x64 屏幕的所有像素点均置 0/熄灭)
 */
void OLED_Clear(void)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_SetCursor(i, 0); // 遍历所有 8 个页 (Page 0 ~ Page 7)
        for (uint8_t j = 0; j < 128; j++)
        {
            OLED_WriteData(0x00); // 写入 0x00 清空该页下 128 列的内容
        }
    }
}

/**
 * @brief  初始化 OLED (SSD1306 控制芯片寄存器配置)
 */
void OLED_Init(void)
{
    HAL_Delay(100);          // 上电复位等待延时
    OLED_WriteCommand(0xAE); // 关闭显示 (Display OFF)
    OLED_WriteCommand(0xD5); // 设置显示时钟分频系数/振荡器频率
    OLED_WriteCommand(0x80); // 默认设置
    OLED_WriteCommand(0xA8); // 设置驱动路数/复用率 (Multiplex Ratio)
    OLED_WriteCommand(0x3F); // 1/64 驱动 (64 行)
    OLED_WriteCommand(0xD3); // 设置显示偏移 (Display Offset)
    OLED_WriteCommand(0x00); // 无偏移
    OLED_WriteCommand(0x40); // 设置屏幕显示起始行 (Start Line) 0x40~0x7F
    OLED_WriteCommand(0x8D); // 电荷泵升压 Pump Set
    OLED_WriteCommand(0x14); // 开启内部电荷泵 (0x14 开启, 0x10 关闭)
    OLED_WriteCommand(0x20); // 设置显存内存寻址模式 (Memory Addressing Mode)
    OLED_WriteCommand(0x02); // 设为页寻址模式 (Page Addressing Mode)
    OLED_WriteCommand(0xA1); // 段重映射设置 (Segment Re-map) 0xA1: 0~127映射到列地址; 0xA0: 左右反转
    OLED_WriteCommand(0xC8); // COM 输出扫描方向 0xC8: 正常方向; 0xC0: 上下反转
    OLED_WriteCommand(0xDA); // 设置 COM 硬件引脚配置 (COM Pins Hardware Configuration)
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); // 对比度控制指令 (Contrast Control)
    OLED_WriteCommand(0xCF); // 亮度数值设定 (0x00~0xFF)
    OLED_WriteCommand(0xD9); // 设置预充电周期 (Pre-charge Period)
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); // 设置 VCOMH 脱扣电压选择
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4); // 全局显示开启 (0xA4 依照RAM内容显示, 0xA5 全亮)
    OLED_WriteCommand(0xA6); // 设置正常显示 (0xA6 正常, 0xA7 反色显)
    OLED_WriteCommand(0xAF); // 开启显示 (Display ON)
    OLED_Clear();            // 初始化后清屏
}

/**
 * @brief  在指定位置显示 1 个 8x16 字符
 * @param  line: 行号 (1 ~ 4，每行 16 像素高)
 * @param  column: 列号 (1 ~ 16，每列 8 像素宽)
 * @param  ch: ASCII 字符 (' ' ~ '~')
 */
void OLED_ShowChar(uint8_t line, uint8_t column, char ch)
{
    uint8_t idx = 0;
    /* 计算在字模表 OLED_F8x16 中的相对偏移索引 */
    if (ch >= ' ' && ch <= '~')
    {
        idx = ch - ' '; // 以 ASCII 32 (' ') 为基准点偏移
    }
    else
    {
        idx = 0; // 超出范围退化为空格
    }

    /* 8x16 字符占用 2 个 Page (高 16 点阵 = 2 个 8 位 Page) */
    uint8_t page = (line - 1) * 2;
    uint8_t col = (column - 1) * 8;

    /* 写入上半部分 8 位点阵 (Page) */
    OLED_SetCursor(page, col);
    for (uint8_t i = 0; i < 8; i++) OLED_WriteData(OLED_F8x16[idx][i]);

    /* 写入下半部分 8 位点阵 (Page + 1) */
    OLED_SetCursor(page + 1, col);
    for (uint8_t i = 0; i < 8; i++) OLED_WriteData(OLED_F8x16[idx][i + 8]);
}

/**
 * @brief  在指定位置显示字符串
 * @param  line: 行号 (1 ~ 4)
 * @param  column: 起始列号 (1 ~ 16)
 * @param  str: 字符串首地址指针
 */
void OLED_ShowString(uint8_t line, uint8_t column, char *str)
{
    while (*str)
    {
        OLED_ShowChar(line, column++, *str++);
    }
}

/**
 * @brief  在指定位置显示带符号的十进制数值 (如: +050, -100)
 * @param  line: 行号 (1 ~ 4)
 * @param  column: 起始列号 (1 ~ 16)
 * @param  num: 要显示的数值 (范围: -32768 ~ 32767)
 * @param  length: 数字有效长度 (不含正负号位)
 */
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int16_t num, uint8_t length)
{
    uint32_t temp;
    if (num >= 0)
    {
        OLED_ShowChar(line, column, '+'); // 正数打印 '+'
        temp = num;
    }
    else
    {
        OLED_ShowChar(line, column, '-'); // 负数打印 '-'
        temp = -((int32_t)num);           // 取绝对值（避免 -32768 溢出）
    }

    /* 按高位到低位逐位分解数字并显示 */
    for (uint8_t i = 0; i < length; i++)
    {
        uint32_t divisor = 1;
        for (uint8_t j = 0; j < length - 1 - i; j++) divisor *= 10;
        OLED_ShowChar(line, column + 1 + i, (temp / divisor) % 10 + '0');
    }
}

/**
 * @brief  内部辅助求幂函数 (X 的 Y 次方)
 * @param  X: 底数
 * @param  Y: 指数
 * @retval 结果数值
 */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief  在指定位置显示无符号十进制数字
 * @param  line: 行号 (1 ~ 4)
 * @param  column: 起始列号 (1 ~ 16)
 * @param  num: 要显示的数值 (范围: 0 ~ 4294967295)
 * @param  length: 显示数字的长度 (高位不足补 '0')
 */
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        OLED_ShowChar(line, column + i, (num / OLED_Pow(10, length - 1 - i)) % 10 + '0');
    }
}

/**
 * @brief  在指定位置显示十六进制数字 (使用位移优化，避免 Pow 超出 uint32 范围导致的除零)
 * @param  line: 行号 (1 ~ 4)
 * @param  column: 起始列号 (1 ~ 16)
 * @param  num: 要显示的数值 (范围: 0 ~ 0xFFFFFFFF)
 * @param  length: 显示十六进制数字的长度 (如 MID 为 2, DID 为 4)
 */
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length)
{
    uint8_t single_num;
    for (uint8_t i = 0; i < length; i++)
    {
        single_num = (num >> (4 * (length - 1 - i))) & 0x0F;
        if (single_num < 10)
        {
            OLED_ShowChar(line, column + i, single_num + '0');
        }
        else
        {
            OLED_ShowChar(line, column + i, single_num - 10 + 'A');
        }
    }
}

/**
 * @brief  在指定位置显示二进制数字
 * @param  line: 行号 (1 ~ 4)
 * @param  column: 起始列号 (1 ~ 16)
 * @param  num: 要显示的数值 (范围: 0 ~ 0xFFFFFFFF)
 * @param  length: 显示二进制位数的长度
 */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t num, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        OLED_ShowChar(line, column + i, (num / OLED_Pow(2, length - 1 - i)) % 2 + '0');
    }
}