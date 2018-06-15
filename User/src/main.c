#include "stm32f10x.h"
#include "DS18B20.h"

void TIM2_Init()
{
    // TODO: от чего тактируется таймер?
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    SystemCoreClockUpdate();
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Period = SystemCoreClock / 7200000 - 1;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 40 - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
}

int main()
{
	// TODO: перенести
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	TIM2_Init();
	TIM_Cmd(TIM2, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	DS18B20_Init(TIM2, 95);

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

void TIM2_IRQHandler()
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		DS18B20_TIM_Handler();
    }
}
