#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f10x.h"

uint8_t DS18B20_Init(TIM_TypeDef* TIMx, uint16_t requestPeriod_ms); // Minimal period is 95 ms; return 1 if at least one device is found, otherwise return 0
void DS18B20_ConvertTemperature();                                  // Start temperature conversion
uint8_t DS18B20_IsReady();                                          // Return 1, if conversion is completed, otherwise returns 0
uint16_t DS18B20_GetRawTemperature();                               // Return raw temperature value (see Figure 4 in Datasheet); function returns right value only once after conversion
float DS18B20_GetRealTemperature();                                 // Return signed float temperature value; function returns right value only once after conversion
void DS18B20_TIM_Handler();                                         // Must be inserted into TIMx_IRQHandler, where TIMx is timer passed to DS18B20_Init

#endif // __DS18B20_H
