#include "OneWireUART.h"

#define OWU_BIT_0_PULSE         0x00
#define OWU_BIT_1_PULSE         0xFF
#define OWU_RESET_PULSE         0xF0
#define OWU_READ_BIT_PULSE      0xFF

#define OWU_WRITE_0_MARK        0x00
#define OWU_WRITE_1_MARK        0x01
#define OWU_READ_BIT_MARK       0x02
#define OWU_RESET_MARK          0x03
#define OWU_WAIT_MARK           0x04

#define OWU_RESET_BAUDRATE      9600
#define OWU_DATA_BAUDRATE       115200
#define OWU_BYTE_SIZE           8

#define OWU_WAIT_TARGET_FRQ     1000.0

struct
{
    unsigned Ready: 1;
    unsigned DeviceDetected: 1;
    unsigned Waiting: 1;
} OWU_Flag = {0, 0, 0};

uint8_t OWU_Sequence[OWU_SEQUENSE_MAX_SIZE];
uint8_t OWU_SequenceWritePosition = 0;
uint8_t OWU_SequenceProcessPosition = 0;
uint8_t OWU_Buffer[OWU_BUFFER_MAX_SIZE];
uint8_t OWU_BufferWritePosition = 0;
uint8_t OWU_BufferReadPosition = 0;
volatile uint32_t OWU_WaitCounter;
float OWU_WaitCounterScaler;

uint8_t OWU_IsDeviceDetected()
{
    return OWU_Flag.DeviceDetected;
}

uint8_t OWU_IsReady()
{
    return OWU_Flag.Ready;
}

void OWU_USART_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = OWU_TX_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(OWU_GPIO, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = OWU_RX_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(OWU_GPIO, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = OWU_DATA_BAUDRATE;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_0_5;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(OWU_USART, &USART_InitStruct);
}

void OWU_WaitCounterScalerConfig(TIM_TypeDef* TIMx)
{
    SystemCoreClockUpdate();
    OWU_WaitCounterScaler = SystemCoreClock / ((TIMx->ARR + 1) * (TIMx->PSC + 1) * OWU_WAIT_TARGET_FRQ);
}

void OWU_Init(TIM_TypeDef* TIMx)
{
    OWU_USART_Init();
    USART_Cmd(OWU_USART, ENABLE);   // TODO: turn off to minimize power consumption?

    USART_ClearITPendingBit(OWU_USART, USART_IT_TC);
    NVIC_EnableIRQ(OWU_USART_IRQn);
	USART_ITConfig(OWU_USART, USART_IT_TC, ENABLE);

    OWU_WaitCounterScalerConfig(TIMx);
}

// TODO: refactoring
void OWU_USART_ChangeBaudRate(uint32_t USART_BaudRate)
{
    uint16_t CR1_OVER8_Set = 0x8000;
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    uint32_t usartxbase = 0;
    RCC_ClocksTypeDef RCC_ClocksStatus;
    usartxbase = (uint32_t)OWU_USART;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    if (usartxbase == USART1_BASE)
    {
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    }
    else
    {
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }

    /* Determine the integer part */
    if ((OWU_USART->CR1 & CR1_OVER8_Set) != 0)
    {
        /* Integer part computing in case Oversampling mode is 8 Samples */
        integerdivider = ((25 * apbclock) / (2 * (USART_BaudRate)));
    }
    else /* if ((OWU_USART->CR1 & CR1_OVER8_Set) == 0) */
    {
        /* Integer part computing in case Oversampling mode is 16 Samples */
        integerdivider = ((25 * apbclock) / (4 * (USART_BaudRate)));
    }
    tmpreg = (integerdivider / 100) << 4;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

    /* Implement the fractional part in the register */
    if ((OWU_USART->CR1 & CR1_OVER8_Set) != 0)
    {
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
    }
    else /* if ((OWU_USART->CR1 & CR1_OVER8_Set) == 0) */
    {
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    }

    /* Write to USART BRR */
    OWU_USART->BRR = (uint16_t)tmpreg;
}

void OWU_AddWriteBit(uint8_t bit)
{
    OWU_Sequence[OWU_SequenceWritePosition++] = bit ? OWU_WRITE_1_MARK : OWU_WRITE_0_MARK;
}

void OWU_AddWriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < OWU_BYTE_SIZE; ++i)
    {
        OWU_AddWriteBit(data >> i & 1);
    }
}

void OWU_AddReadBit()
{
    OWU_Sequence[OWU_SequenceWritePosition++] = OWU_READ_BIT_MARK;
}

void OWU_AddReadByte()
{
    for (uint8_t i = 0; i < OWU_BYTE_SIZE; ++i)
    {
        OWU_AddReadBit();
    }
}

void OWU_AddDelay(uint16_t delay_ms)
{
    OWU_Sequence[OWU_SequenceWritePosition++] = OWU_WAIT_MARK;
    OWU_Sequence[OWU_SequenceWritePosition++] = (uint8_t) (delay_ms >> OWU_BYTE_SIZE);
    OWU_Sequence[OWU_SequenceWritePosition++] = (uint8_t) delay_ms;
}

void OWU_AddReset()
{
    OWU_Sequence[OWU_SequenceWritePosition++] = OWU_RESET_MARK;
}

void OWU_NewSequence()
{
    OWU_SequenceWritePosition = 0;
}

void OWU_ProcessCurrentElement()
{
    if (OWU_SequenceProcessPosition < OWU_SequenceWritePosition)
    {
        switch (OWU_Sequence[OWU_SequenceProcessPosition])
        {
            case OWU_WRITE_0_MARK:
                USART_SendData(OWU_USART, OWU_BIT_0_PULSE);
                break;
            case OWU_WRITE_1_MARK:
                USART_SendData(OWU_USART, OWU_BIT_1_PULSE);
                break;
            case OWU_READ_BIT_MARK:
                USART_SendData(OWU_USART, OWU_READ_BIT_PULSE);
                break;
            case OWU_RESET_MARK:
                OWU_USART_ChangeBaudRate(OWU_RESET_BAUDRATE);
                USART_SendData(OWU_USART, OWU_RESET_PULSE);
                break;
            case OWU_WAIT_MARK:
                OWU_WaitCounter = (uint32_t) (OWU_WaitCounterScaler * ((((uint16_t) OWU_Sequence[OWU_SequenceProcessPosition + 1]) << OWU_BYTE_SIZE) + (uint16_t) OWU_Sequence[OWU_SequenceProcessPosition + 2])) + 1; // TODO: solve the problem with rounding down
                OWU_SequenceProcessPosition += 2;
                OWU_Flag.Waiting = 1;
                break;
        }
        // FIXME: interrupt handler call is possible after processing, but before the increment - everything will break
        ++OWU_SequenceProcessPosition;
    }
    else
    {
        // TODO: move to handler?
        OWU_Flag.Ready = 1;
    }
}

// TODO: rename function or unite with OWU_ProcessCurrentElement
void OWU_ProcessPreviousElement()
{
    switch (OWU_Sequence[OWU_SequenceProcessPosition - 1])
    {
        case OWU_READ_BIT_MARK:
            OWU_Buffer[OWU_BufferWritePosition++] = USART_ReceiveData(OWU_USART);
            break;
        case OWU_RESET_MARK:
            if (USART_ReceiveData(OWU_USART) != OWU_READ_BIT_PULSE)
            {
                OWU_Flag.DeviceDetected = 1;
            }
            else
            {
                OWU_Flag.DeviceDetected = 0;
            }
            OWU_USART_ChangeBaudRate(OWU_DATA_BAUDRATE);
            break;
    }
}

void OWU_ProcessSequence()
{
    OWU_SequenceProcessPosition = 0;
    OWU_BufferWritePosition = 0;
    OWU_BufferReadPosition = 0;
    OWU_Flag.Ready = 0;
    OWU_ProcessCurrentElement();
}

uint8_t OWU_ReadBitFromBuffer()
{
    return (OWU_Buffer[OWU_BufferReadPosition++] == OWU_READ_BIT_PULSE) ? 1 : 0;
}

uint8_t OWU_ReadByteFromBuffer()
{
    uint8_t data = 0;

    for (uint8_t bitIndex = 0; bitIndex < OWU_BYTE_SIZE ; ++bitIndex)
    {
        data |= OWU_ReadBitFromBuffer() << bitIndex;
    }

    return data;
}

void OWU_USART_IRQHandler()
{
    if (USART_GetITStatus(OWU_USART, USART_IT_TC) == SET)
    {
        USART_ClearITPendingBit(OWU_USART, USART_IT_TC);
        OWU_ProcessPreviousElement();
        OWU_ProcessCurrentElement();
    }
}

// TODO: use register value instead of using counter?
void OWU_TIM_Handler()
{
    if (OWU_Flag.Waiting)
    {
        if (!(--OWU_WaitCounter))
        {
            OWU_Flag.Waiting = 0;
            OWU_ProcessCurrentElement();
        }
    }
}
