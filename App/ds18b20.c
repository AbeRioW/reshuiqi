#include "ds18b20.h"

void Delay_us(uint32_t us)
{
    uint32_t i;
    while(us--)
    {
        for(i=0; i<10; i++);
    }
}

void DS18B20_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DS18B20_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS18B20_GPIO_Port, &GPIO_InitStruct);
}

void DS18B20_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DS18B20_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_GPIO_Port, &GPIO_InitStruct);
}

void DS18B20_Rst(void)
{
    DS18B20_IO_OUT();
    DS18B20_Clr();
    Delay_us(480);
    DS18B20_Set();
    Delay_us(70);
}

uint8_t DS18B20_Check(void)
{
    uint8_t retry = 0;
    DS18B20_IO_IN();
    
    while (DS18B20_Read() && retry < 200)
    {
        retry++;
        Delay_us(1);
    }
    
    if (retry >= 200)
        return 1;
    else
        retry = 0;
    
    while (!DS18B20_Read() && retry < 240)
    {
        retry++;
        Delay_us(1);
    }
    
    if (retry >= 240)
        return 1;
    
    Delay_us(400);
    return 0;
}

uint8_t DS18B20_Read_Byte(void)
{
    uint8_t i, dat;
    dat = 0;
    
    for (i = 0; i < 8; i++)
    {
        dat >>= 1;
        DS18B20_IO_OUT();
        DS18B20_Clr();
        Delay_us(2);
        DS18B20_Set();
        DS18B20_IO_IN();
        Delay_us(12);
        
        if (DS18B20_Read())
            dat |= 0x80;
        
        Delay_us(50);
    }
    
    return dat;
}

void DS18B20_Write_Byte(uint8_t dat)
{
    uint8_t j;
    uint8_t testb;
    
    DS18B20_IO_OUT();
    
    for (j = 0; j < 8; j++)
    {
        testb = dat & 0x01;
        dat >>= 1;
        
        if (testb)
        {
            DS18B20_Clr();
            Delay_us(2);
            DS18B20_Set();
            Delay_us(60);
        }
        else
        {
            DS18B20_Clr();
            Delay_us(60);
            DS18B20_Set();
            Delay_us(2);
        }
    }
}

void DS18B20_Start(void)
{
}

void DS18B20_Init(void)
{
    DS18B20_IO_OUT();
    DS18B20_Set();
}

float DS18B20_Get_Temperature(void)
{
    uint8_t temp_l, temp_h;
    int16_t temp;
    float temperature;
    
    DS18B20_Rst();
    if (DS18B20_Check() != 0)
    {
        return -999.0f;
    }
    
    DS18B20_Write_Byte(0xcc);
    DS18B20_Write_Byte(0x44);
    
    HAL_Delay(750);
    
    DS18B20_Rst();
    if (DS18B20_Check() != 0)
    {
        return -999.0f;
    }
    
    DS18B20_Write_Byte(0xcc);
    DS18B20_Write_Byte(0xbe);
    
    temp_l = DS18B20_Read_Byte();
    temp_h = DS18B20_Read_Byte();
    
    temp = temp_h;
    temp <<= 8;
    temp |= temp_l;
    
    temperature = (float)temp / 16.0f;
    
    return temperature;
}
