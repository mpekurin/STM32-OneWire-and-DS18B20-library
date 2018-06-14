#include "stm32f10x.h"
#include "DS18B20.h"

int main()
{
	// TODO: remap USART1 to PB7 and PB6
	// TODO: use custom timers
	// TODO: перенести
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	DS18B20_Init(1000);

	float temperature = 0;

	while (1)
	{
		if (DS18B20_IsReady())
		{
			temperature = DS18B20_GetRealTemperature();
			DS18B20_ConvertTemperature();
		}
	}
}
