#ifndef __DS1302_H
#define __DS1302_H

#include "stdint.h"
#include "gpio.h"

#define DS1302_CLK_Clr()  HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET)
#define DS1302_CLK_Set()  HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_SET)

#define DS1302_DAT_Clr()  HAL_GPIO_WritePin(DS1302_DAT_GPIO_Port, DS1302_DAT_Pin, GPIO_PIN_RESET)
#define DS1302_DAT_Set()  HAL_GPIO_WritePin(DS1302_DAT_GPIO_Port, DS1302_DAT_Pin, GPIO_PIN_SET)

#define DS1302_RST_Clr()  HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin, GPIO_PIN_RESET)
#define DS1302_RST_Set()  HAL_GPIO_WritePin(DS1302_RST_GPIO_Port, DS1302_RST_Pin, GPIO_PIN_SET)

#define DS1302_READ_SECOND   0x81
#define DS1302_WRITE_SECOND  0x80
#define DS1302_READ_MINUTE   0x83
#define DS1302_WRITE_MINUTE  0x82
#define DS1302_READ_HOUR     0x85
#define DS1302_WRITE_HOUR    0x84
#define DS1302_READ_DAY      0x87
#define DS1302_WRITE_DAY     0x86
#define DS1302_READ_MONTH    0x89
#define DS1302_WRITE_MONTH   0x88
#define DS1302_READ_WEEK     0x8B
#define DS1302_WRITE_WEEK    0x8A
#define DS1302_READ_YEAR     0x8D
#define DS1302_WRITE_YEAR    0x8C
#define DS1302_READ_WP       0x8F
#define DS1302_WRITE_WP      0x8E

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DS1302_Time;

void DS1302_Init(void);
void DS1302_WriteByte(uint8_t dat);
uint8_t DS1302_ReadByte(void);
void DS1302_WriteReg(uint8_t addr, uint8_t dat);
uint8_t DS1302_ReadReg(uint8_t addr);
void DS1302_SetTime(DS1302_Time *time);
void DS1302_GetTime(DS1302_Time *time);

#endif
