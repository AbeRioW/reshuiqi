/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "ds1302.h"
#include "ds18b20.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
DS1302_Time time;
uint8_t time_str[32];
float temperature;
uint8_t temp_str[32];
uint16_t adc_value;
uint8_t water_depth_str[32];
uint8_t water_fill_str[32];
uint8_t heating_str[32];
volatile uint8_t time_update_flag = 0;

// 按键标志
volatile uint8_t key1_pressed = 0;
volatile uint8_t key2_pressed = 0;
volatile uint8_t key3_pressed = 0;

// 设置状态
typedef enum {
    STATE_MAIN,           // 主界面
    STATE_SET_TEMP_HIGH,  // 设置温度高阈值
    STATE_SET_TEMP_LOW,   // 设置温度低阈值
    STATE_SET_WATER_LEVEL, // 设置水位阈值
    STATE_SET_TIME,       // 设置时间
    STATE_SET_WATER_TIME  // 设置注水时刻
} SystemState;

SystemState current_state = STATE_MAIN;
uint8_t setting_index = 0;

// 设置参数
int8_t temp_high_threshold = 60;
int8_t temp_low_threshold = 20;
uint16_t water_level_threshold = 100;
DS1302_Time water_fill_time;

// 定时注水
uint8_t water_fill_active = 0;
uint32_t water_fill_start_time = 0;
uint8_t water_fill_triggered = 0;

// 加热状态
uint8_t heating_active = 0;

// 显示缓冲
uint8_t display_line1[32];
uint8_t display_line2[32];
uint8_t display_line3[32];
uint8_t display_line4[32];

// 用于检测状态变化
SystemState previous_state = STATE_MAIN;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY1_Pin)
    {
        key1_pressed = 1;
    }
    else if (GPIO_Pin == KEY2_Pin)
    {
        key2_pressed = 1;
    }
    else if (GPIO_Pin == KEY3_Pin)
    {
        key3_pressed = 1;
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  DS1302_Init();
  DS18B20_Init();
  
//  time.year = 24;
//  time.month = 6;
//  time.day = 1;
//  time.week = 1;
//  time.hour = 12;
//  time.minute = 0;
//  time.second = 0;
//  DS1302_SetTime(&time);
  
  // 初始化注水时间
  water_fill_time.hour = 12;
  water_fill_time.minute = 0;
  water_fill_time.second = 0;
  
  // 启动定时器
  HAL_TIM_Base_Start_IT(&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // 读取当前时间 - 只有不在设置时间界面时才更新
    if (time_update_flag && current_state != STATE_SET_TIME)
    {
        time_update_flag = 0;
        DS1302_GetTime(&time);
    }
    
    // 检查定时注水
    static uint32_t check_water_fill_timer = 0;
    if (HAL_GetTick() - check_water_fill_timer >= 100)
    {
        check_water_fill_timer = HAL_GetTick();
        
        // 检查是否到达注水时刻
        if (current_state == STATE_MAIN && !water_fill_active)
        {
            if (time.hour == water_fill_time.hour && 
                time.minute == water_fill_time.minute && 
                time.second == water_fill_time.second &&
                !water_fill_triggered)
            {
                // 开始注水
                HAL_GPIO_WritePin(LAY_ZHUSHUI_GPIO_Port, LAY_ZHUSHUI_Pin, GPIO_PIN_SET);
                water_fill_active = 1;
                water_fill_start_time = HAL_GetTick();
                water_fill_triggered = 1;
            }
        }
        
        // 检查注水5秒后关闭
        if (water_fill_active && (HAL_GetTick() - water_fill_start_time >= 5000))
        {
            HAL_GPIO_WritePin(LAY_ZHUSHUI_GPIO_Port, LAY_ZHUSHUI_Pin, GPIO_PIN_RESET);
            water_fill_active = 0;
        }
        
        // 重置triggered标志
        if (time.second != water_fill_time.second)
        {
            water_fill_triggered = 0;
        }
    }
    
    // 处理按键
    if (key1_pressed)
    {
        key1_pressed = 0;
        
        // 切换状态
        switch (current_state)
        {
            case STATE_MAIN:
                current_state = STATE_SET_TEMP_HIGH;
                setting_index = 0;
                break;
            case STATE_SET_TEMP_HIGH:
                current_state = STATE_SET_TEMP_LOW;
                break;
            case STATE_SET_TEMP_LOW:
                current_state = STATE_SET_WATER_LEVEL;
                break;
            case STATE_SET_WATER_LEVEL:
                current_state = STATE_SET_TIME;
                setting_index = 0;
                // 进入设置时间页面时，先读取DS1302的当前时间作为初始值
                DS1302_GetTime(&time);
                break;
            case STATE_SET_TIME:
                if (setting_index < 5)
                {
                    setting_index++;
                }
                else
                {
                    current_state = STATE_SET_WATER_TIME;
                    // 保存时间到DS1302
                    DS1302_SetTime(&time);
                    setting_index = 0;
                }
                break;
            case STATE_SET_WATER_TIME:
                if (setting_index < 2)
                {
                    setting_index++;
                }
                else
                {
                    current_state = STATE_MAIN;
                }
                break;
        }
    }
    
    if (key2_pressed)
    {
        key2_pressed = 0;
        
        switch (current_state)
        {
            case STATE_SET_TEMP_HIGH:
                if (temp_high_threshold < 100) temp_high_threshold++;
                break;
            case STATE_SET_TEMP_LOW:
                if (temp_low_threshold < temp_high_threshold - 1) temp_low_threshold++;
                break;
            case STATE_SET_WATER_LEVEL:
                if (water_level_threshold < 4500) water_level_threshold += 100;
                break;
            case STATE_SET_TIME:
                switch (setting_index)
                {
                    case 0: if (time.year < 99) time.year++; break;
                    case 1: if (time.month < 12) time.month++; else time.month = 1; break;
                    case 2: if (time.day < 31) time.day++; else time.day = 1; break;
                    case 3: if (time.hour < 23) time.hour++; else time.hour = 0; break;
                    case 4: if (time.minute < 59) time.minute++; else time.minute = 0; break;
                    case 5: if (time.second < 59) time.second++; else time.second = 0; break;
                }
                break;
            case STATE_SET_WATER_TIME:
                switch (setting_index)
                {
                    case 0: if (water_fill_time.hour < 23) water_fill_time.hour++; else water_fill_time.hour = 0; break;
                    case 1: if (water_fill_time.minute < 59) water_fill_time.minute++; else water_fill_time.minute = 0; break;
                    case 2: if (water_fill_time.second < 59) water_fill_time.second++; else water_fill_time.second = 0; break;
                }
                break;
            default:
                break;
        }
    }
    
    if (key3_pressed)
    {
        key3_pressed = 0;
        
        switch (current_state)
        {
            case STATE_MAIN:
                // 主界面不处理
                break;
            case STATE_SET_TEMP_HIGH:
                if (temp_high_threshold > temp_low_threshold + 1) temp_high_threshold--;
                break;
            case STATE_SET_TEMP_LOW:
                if (temp_low_threshold > 0) temp_low_threshold--;
                break;
            case STATE_SET_WATER_LEVEL:
                if (water_level_threshold > 0) water_level_threshold -= 100;
                break;
            case STATE_SET_TIME:
                switch (setting_index)
                {
                    case 0: if (time.year > 0) time.year--; break;
                    case 1: if (time.month > 1) time.month--; else time.month = 12; break;
                    case 2: if (time.day > 1) time.day--; else time.day = 31; break;
                    case 3: if (time.hour > 0) time.hour--; else time.hour = 23; break;
                    case 4: if (time.minute > 0) time.minute--; else time.minute = 59; break;
                    case 5: if (time.second > 0) time.second--; else time.second = 59; break;
                }
                break;
            case STATE_SET_WATER_TIME:
                switch (setting_index)
                {
                    case 0: if (water_fill_time.hour > 0) water_fill_time.hour--; else water_fill_time.hour = 23; break;
                    case 1: if (water_fill_time.minute > 0) water_fill_time.minute--; else water_fill_time.minute = 59; break;
                    case 2: if (water_fill_time.second > 0) water_fill_time.second--; else water_fill_time.second = 59; break;
                }
                break;
        }
    }
    
    // 读取传感器数据
    static uint32_t sensor_update_timer = 0;
    if (HAL_GetTick() - sensor_update_timer >= 1000)
    {
        sensor_update_timer = HAL_GetTick();
        
        // 读取温度
        temperature = DS18B20_Get_Temperature();
        
        // 读取ADC
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            adc_value = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);
        
        // 温度控制（仅在主界面）
        if (current_state == STATE_MAIN && temperature > -100)
        {
            if (temperature < temp_low_threshold)
            {
                // 低于低阈值，开启加热
                heating_active = 1;
                HAL_GPIO_WritePin(LAY_JIARE_GPIO_Port, LAY_JIARE_Pin, GPIO_PIN_SET);
            }
            else if (temperature >= temp_high_threshold)
            {
                // 达到高阈值，停止加热
                heating_active = 0;
                HAL_GPIO_WritePin(LAY_JIARE_GPIO_Port, LAY_JIARE_Pin, GPIO_PIN_RESET);
            }
            else
            {
                // 温度在低阈值和高阈值之间，保持当前状态
                if (heating_active)
                {
                    HAL_GPIO_WritePin(LAY_JIARE_GPIO_Port, LAY_JIARE_Pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(LAY_JIARE_GPIO_Port, LAY_JIARE_Pin, GPIO_PIN_RESET);
                }
            }
        }
        
        // 水位控制（仅在主界面且不在定时注水时）
        if (current_state == STATE_MAIN && !water_fill_active)
        {
            if (adc_value < water_level_threshold)
            {
                HAL_GPIO_WritePin(LAY_ZHUSHUI_GPIO_Port, LAY_ZHUSHUI_Pin, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(LAY_ZHUSHUI_GPIO_Port, LAY_ZHUSHUI_Pin, GPIO_PIN_RESET);
            }
        }
    }
    
    // 显示界面 - 只在状态切换时清屏
    if (current_state != previous_state)
    {
        OLED_Clear();
        previous_state = current_state;
    }
    
    if (current_state == STATE_MAIN)
    {
        // 主界面 - 每次都完整绘制（避免静态变量问题）
        sprintf((char*)display_line1, "Water Temp:%.1f C", temperature);
        OLED_ShowString(0, 0, display_line1, 8, 1);
        
        sprintf((char*)display_line2, "Water Depth:%4d", adc_value);
        OLED_ShowString(0, 8, display_line2, 8, 1);
        
        if (water_fill_active)
        {
            sprintf((char*)display_line3, "Water Fill:YES*");
        }
        else if (HAL_GPIO_ReadPin(LAY_ZHUSHUI_GPIO_Port, LAY_ZHUSHUI_Pin) == GPIO_PIN_SET)
        {
            sprintf((char*)display_line3, "Water Fill:YES");
        }
        else
        {
            sprintf((char*)display_line3, "Water Fill:NO ");
        }
        OLED_ShowString(0, 16, display_line3, 8, 1);
        
        if (HAL_GPIO_ReadPin(LAY_JIARE_GPIO_Port, LAY_JIARE_Pin) == GPIO_PIN_SET)
        {
            sprintf((char*)display_line4, "Heating:YES");
        }
        else
        {
            sprintf((char*)display_line4, "Heating:NO ");
        }
        OLED_ShowString(0, 24, display_line4, 8, 1);
        
        sprintf((char*)time_str, "20%02d-%02d-%02d %02d:%02d:%02d",
                time.year, time.month, time.day,
                time.hour, time.minute, time.second);
        OLED_ShowString(0, 32, time_str, 8, 1);
        
        OLED_Refresh();
    }
    else if (current_state == STATE_SET_TEMP_HIGH || current_state == STATE_SET_TEMP_LOW)
    {
        // 温度阈值设置界面 - 有变化才更新
        static int8_t prev_temp_high = -1;
        static int8_t prev_temp_low = -1;
        static SystemState prev_temp_state = STATE_MAIN;
        
        if (temp_high_threshold != prev_temp_high || temp_low_threshold != prev_temp_low || current_state != prev_temp_state)
        {
            sprintf((char*)display_line1, "Temp Threshold");
            sprintf((char*)display_line2, "High:%3d C", temp_high_threshold);
            sprintf((char*)display_line3, "Low :%3d C", temp_low_threshold);
            
            if (current_state == STATE_SET_TEMP_HIGH)
            {
                sprintf((char*)display_line4, "Set High ->");
            }
            else
            {
                sprintf((char*)display_line4, "Set Low  ->");
            }
            
            OLED_ShowString(0, 0, display_line1, 8, 1);
            OLED_ShowString(0, 8, display_line2, 8, 1);
            OLED_ShowString(0, 16, display_line3, 8, 1);
            OLED_ShowString(0, 24, display_line4, 8, 1);
            OLED_Refresh();
            
            prev_temp_high = temp_high_threshold;
            prev_temp_low = temp_low_threshold;
            prev_temp_state = current_state;
        }
    }
    else if (current_state == STATE_SET_WATER_LEVEL)
    {
        // 水位阈值设置界面 - 有变化才更新
        static uint16_t prev_water_thresh = 0xFFFF;
        static uint16_t prev_water_val = 0xFFFF;
        
        if (water_level_threshold != prev_water_thresh || adc_value != prev_water_val)
        {
            sprintf((char*)display_line1, "Water Level");
            sprintf((char*)display_line2, "Threshold:%4d", water_level_threshold);
            sprintf((char*)display_line3, "Current :%4d", adc_value);
            
            OLED_ShowString(0, 0, display_line1, 8, 1);
            OLED_ShowString(0, 8, display_line2, 8, 1);
            OLED_ShowString(0, 16, display_line3, 8, 1);
            OLED_Refresh();
            
            prev_water_thresh = water_level_threshold;
            prev_water_val = adc_value;
        }
    }
    else if (current_state == STATE_SET_TIME)
    {
        // 时间设置界面 - 每次都显示
        OLED_Clear();
        
        sprintf((char*)display_line1, "Set Time");
        sprintf((char*)display_line2, "20%02d-%02d-%02d %02d:%02d:%02d", 
                time.year, time.month, time.day, time.hour, time.minute, time.second);
        
        char cursor[32] = "                                ";
        if (setting_index == 0) cursor[2] = '^';   // 年份位置
        else if (setting_index == 1) cursor[5] = '^';  // 月份位置
        else if (setting_index == 2) cursor[8] = '^';  // 日期位置
        else if (setting_index == 3) cursor[11] = '^'; // 小时位置
        else if (setting_index == 4) cursor[14] = '^'; // 分钟位置
        else if (setting_index == 5) cursor[17] = '^'; // 秒位置
        
        OLED_ShowString(0, 0, display_line1, 8, 1);
        OLED_ShowString(0, 8, display_line2, 8, 1);
        OLED_ShowString(0, 16, (uint8_t*)cursor, 8, 1);
        OLED_Refresh();
    }
    else if (current_state == STATE_SET_WATER_TIME)
    {
        // 注水时刻设置界面 - 有变化才更新
        static DS1302_Time prev_water_fill;
        static uint8_t prev_fill_idx = 0xFF;
        
        if (water_fill_time.hour != prev_water_fill.hour || 
            water_fill_time.minute != prev_water_fill.minute ||
            water_fill_time.second != prev_water_fill.second ||
            setting_index != prev_fill_idx)
        {
            OLED_Clear();
            
            sprintf((char*)display_line1, "Set Water Time");
            sprintf((char*)display_line2, "%02d:%02d:%02d", water_fill_time.hour, water_fill_time.minute, water_fill_time.second);
            
            char cursor[16] = "          ";
            if (setting_index == 0) cursor[1] = '^';
            else if (setting_index == 1) cursor[4] = '^';
            else if (setting_index == 2) cursor[7] = '^';
            
            OLED_ShowString(0, 0, display_line1, 8, 1);
            OLED_ShowString(0, 8, display_line2, 8, 1);
            OLED_ShowString(0, 16, (uint8_t*)cursor, 8, 1);
            OLED_Refresh();
            
            prev_water_fill = water_fill_time;
            prev_fill_idx = setting_index;
        }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        time_update_flag = 1;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
