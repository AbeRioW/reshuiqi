#include "ds1302.h"

static void DS1302_DAT_Mode_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS1302_DAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS1302_DAT_GPIO_Port, &GPIO_InitStruct);
}

static void DS1302_DAT_Mode_In(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS1302_DAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS1302_DAT_GPIO_Port, &GPIO_InitStruct);
}

void DS1302_Delay_us(uint32_t us)
{
    uint32_t i;
    while(us--)
    {
        for(i=0; i<10; i++);
    }
}

void DS1302_Init(void)
{
    DS1302_RST_Clr();
    DS1302_CLK_Clr();
    DS1302_DAT_Mode_Out();
    DS1302_WriteReg(DS1302_WRITE_WP, 0x00);
}

void DS1302_WriteByte(uint8_t dat)
{
    uint8_t i;
    DS1302_DAT_Mode_Out();
    for(i=0; i<8; i++)
    {
        if(dat & 0x01)
        {
            DS1302_DAT_Set();
        }
        else
        {
            DS1302_DAT_Clr();
        }
        DS1302_Delay_us(2);
        DS1302_CLK_Set();
        DS1302_Delay_us(2);
        DS1302_CLK_Clr();
        DS1302_Delay_us(2);
        dat >>= 1;
    }
}

uint8_t DS1302_ReadByte(void)
{
    uint8_t i, dat = 0;
    DS1302_DAT_Mode_In();
    for(i=0; i<8; i++)
    {
        dat >>= 1;
        DS1302_CLK_Clr();
        DS1302_Delay_us(2);
        if(HAL_GPIO_ReadPin(DS1302_DAT_GPIO_Port, DS1302_DAT_Pin) == GPIO_PIN_SET)
        {
            dat |= 0x80;
        }
        DS1302_CLK_Set();
        DS1302_Delay_us(2);
    }
    return dat;
}

void DS1302_WriteReg(uint8_t addr, uint8_t dat)
{
    DS1302_RST_Clr();
    DS1302_CLK_Clr();
    DS1302_Delay_us(2);
    DS1302_RST_Set();
    DS1302_Delay_us(2);
    DS1302_WriteByte(addr);
    DS1302_WriteByte(dat);
    DS1302_RST_Clr();
    DS1302_Delay_us(2);
}

uint8_t DS1302_ReadReg(uint8_t addr)
{
    uint8_t dat;
    DS1302_RST_Clr();
    DS1302_CLK_Clr();
    DS1302_Delay_us(2);
    DS1302_RST_Set();
    DS1302_Delay_us(2);
    DS1302_WriteByte(addr);
    dat = DS1302_ReadByte();
    DS1302_RST_Clr();
    DS1302_Delay_us(2);
    return dat;
}

uint8_t DS1302_DecToBcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

uint8_t DS1302_BcdToDec(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void DS1302_SetTime(DS1302_Time *time)
{
    uint8_t sec = DS1302_ReadReg(DS1302_READ_SECOND);
    DS1302_WriteReg(DS1302_WRITE_SECOND, DS1302_DecToBcd(time->second) | (sec & 0x80));
    DS1302_WriteReg(DS1302_WRITE_MINUTE, DS1302_DecToBcd(time->minute));
    DS1302_WriteReg(DS1302_WRITE_HOUR, DS1302_DecToBcd(time->hour));
    DS1302_WriteReg(DS1302_WRITE_DAY, DS1302_DecToBcd(time->day));
    DS1302_WriteReg(DS1302_WRITE_MONTH, DS1302_DecToBcd(time->month));
    DS1302_WriteReg(DS1302_WRITE_WEEK, DS1302_DecToBcd(time->week));
    DS1302_WriteReg(DS1302_WRITE_YEAR, DS1302_DecToBcd(time->year));
}

void DS1302_GetTime(DS1302_Time *time)
{
    time->second = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_SECOND) & 0x7F);
    time->minute = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_MINUTE));
    time->hour = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_HOUR) & 0x3F);
    time->day = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_DAY));
    time->month = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_MONTH));
    time->week = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_WEEK));
    time->year = DS1302_BcdToDec(DS1302_ReadReg(DS1302_READ_YEAR));
}
