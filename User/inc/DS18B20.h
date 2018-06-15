#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f10x.h"

uint8_t DS18B20_Init(TIM_TypeDef* TIMx, uint16_t requestPeriod_ms); // Настройка всего нужного; минимальный период 95 мс; возвращает 1, если на линии обнаружено хотя бы одно устройство, иначе 0
void DS18B20_ConvertTemperature();                                  // Запуск преобразования температуры
uint8_t DS18B20_IsReady();                                          // Возвращает 1, если термометр опрошен, иначе 0
uint16_t DS18B20_GetRawTemperature();                               // Возвращает необработанное значение температуры (Figure 4 in Datasheet); функция возвращает правильный результат только 1 раз после конвертации
float DS18B20_GetRealTemperature();                                 // Возвращает обработанное значение температуры; функция возвращает правильный результат только 1 раз после конвертации
void DS18B20_TIM_Handler();

#endif // __DS18B20_H
