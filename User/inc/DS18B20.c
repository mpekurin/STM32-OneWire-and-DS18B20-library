#include "DS18B20.h"
#include "OneWireUART.h"

// Команды ROM
#define DS18B20_SEARCH_ROM              0xF0
#define DS18B20_READ_ROM                0x33
#define DS18B20_MATCH_ROM               0x55
#define DS18B20_SKIP_ROM                0xCC
#define DS18B20_ALARM_SEARCH            0xEC
// Команды действий
#define DS18B20_CONVERT_T               0x44
#define DS18B20_WRITE_SCRATCHPAD        0x4E
#define DS18B20_READ_SCRATCHPAD         0xBE
#define DS18B20_COPY_SCRATCHPAD         0x48
#define DS18B20_RECALL_E2               0xB8
#define DS18B20_READ_POWER_SUPPLY       0xB4
// Настройки разрешения
#define DS18B20_SET_RESOLUTION_9_BIT    0x1F
#define DS18B20_SET_RESOLUTION_10_BIT   0x3F
#define DS18B20_SET_RESOLUTION_11_BIT   0x5F
#define DS18B20_SET_RESOLUTION_12_BIT   0x7F
// Значения регистров TH и TL
#define DS18B20_TH_DEFAULT              0
#define DS18B20_TL_DEFAULT              0
// Время преобразования температуры, мс
#define DS18B20_T_CONV_9BIT             94
#define DS18B20_T_CONV_10BIT            188
#define DS18B20_T_CONV_11BIT            375
#define DS18B20_T_CONV_12BIT            750
// Маски для определения знака и значения температуры
#define DS18B20_SIGN_MASK               0xF800
#define DS18B20_VALUE_MASK              0x07FF

uint8_t DS18B20_IsFirstStart;
uint16_t DS18B20_ConversionTime;
uint16_t DS18B20_RequestPeriod;

uint8_t DS18B20_Init(TIM_TypeDef* TIMx, uint16_t requestPeriod_ms)
{
    DS18B20_RequestPeriod = requestPeriod_ms;
    DS18B20_IsFirstStart = 1;
    OWU_Init(TIMx);

    OWU_NewSequence();
    OWU_AddReset();
    OWU_AddWriteByte(DS18B20_SKIP_ROM);
    OWU_AddWriteByte(DS18B20_WRITE_SCRATCHPAD);
    OWU_AddWriteByte(DS18B20_TH_DEFAULT);
    OWU_AddWriteByte(DS18B20_TL_DEFAULT);

    if (DS18B20_RequestPeriod <= DS18B20_T_CONV_10BIT)
    {
        OWU_AddWriteByte(DS18B20_SET_RESOLUTION_9_BIT);
        DS18B20_ConversionTime = DS18B20_T_CONV_9BIT;
    }
    else if (DS18B20_RequestPeriod <= DS18B20_T_CONV_11BIT)
    {
        OWU_AddWriteByte(DS18B20_SET_RESOLUTION_10_BIT);
        DS18B20_ConversionTime = DS18B20_T_CONV_10BIT;
    }
    else if (DS18B20_RequestPeriod <= DS18B20_T_CONV_12BIT)
    {
        OWU_AddWriteByte(DS18B20_SET_RESOLUTION_11_BIT);
        DS18B20_ConversionTime = DS18B20_T_CONV_11BIT;
    }
    else
    {
        OWU_AddWriteByte(DS18B20_SET_RESOLUTION_12_BIT);
        DS18B20_ConversionTime = DS18B20_T_CONV_12BIT;
    }

    // Отправляем настройки
    OWU_ProcessSequence();
    while(!OWU_IsReady()){};

    // Формируем последовательность для первого измерения температуры и отправляем
	OWU_NewSequence();
	OWU_AddReset();
	OWU_AddWriteByte(DS18B20_SKIP_ROM);
	OWU_AddWriteByte(DS18B20_CONVERT_T);
	OWU_AddDelay(DS18B20_ConversionTime);
	OWU_AddReset();
	OWU_AddWriteByte(DS18B20_SKIP_ROM);
	OWU_AddWriteByte(DS18B20_READ_SCRATCHPAD);
	OWU_AddReadByte();
	OWU_AddReadByte();
	OWU_ProcessSequence();

    return OWU_IsDeviceDetected();
}

uint16_t DS18B20_GetRawTemperature()
{
    return OWU_ReadByteFromBuffer() + (OWU_ReadByteFromBuffer() << 8);
}

void DS18B20_ConvertTemperature()
{
    if (!DS18B20_IsFirstStart)
    {
        OWU_ProcessSequence();
    }
    else
    {
        // Добавляем задержку в последовательность, если это первое измерение температуры после инициализации
        DS18B20_IsFirstStart = 0;
        // TODO: убрать в функцию
	    OWU_NewSequence();
        OWU_AddDelay(DS18B20_RequestPeriod - DS18B20_ConversionTime);
	    OWU_AddReset();
	    OWU_AddWriteByte(DS18B20_SKIP_ROM);
	    OWU_AddWriteByte(DS18B20_CONVERT_T);
	    OWU_AddDelay(DS18B20_ConversionTime);
	    OWU_AddReset();
	    OWU_AddWriteByte(DS18B20_SKIP_ROM);
	    OWU_AddWriteByte(DS18B20_READ_SCRATCHPAD);
	    OWU_AddReadByte();
	    OWU_AddReadByte();
	    OWU_ProcessSequence();
    }
}

uint8_t DS18B20_IsReady()
{
    return OWU_IsReady();
}

float DS18B20_GetRealTemperature()
{
    uint16_t rawTemperature = DS18B20_GetRawTemperature();
    int8_t sign = 1;
    if (rawTemperature & DS18B20_SIGN_MASK)
    {
        sign = -1;
    }
    return sign * (rawTemperature & DS18B20_VALUE_MASK) / 16.0;
}

void DS18B20_TIM_Handler()
{
    OWU_TIM_Handler();
}
