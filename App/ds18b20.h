#ifndef __DS18B20_H
#define __DS18B20_H

#include "stdint.h"
#include "gpio.h"

#define DS18B20_Clr()  HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_RESET)
#define DS18B20_Set()  HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_SET)
#define DS18B20_Read() HAL_GPIO_ReadPin(DS18B20_GPIO_Port, DS18B20_Pin)

void DS18B20_IO_OUT(void);
void DS18B20_IO_IN(void);
void DS18B20_Rst(void);
uint8_t DS18B20_Check(void);
uint8_t DS18B20_Read_Byte(void);
void DS18B20_Write_Byte(uint8_t dat);
void DS18B20_Start(void);
void DS18B20_Init(void);
float DS18B20_Get_Temperature(void);

#endif
