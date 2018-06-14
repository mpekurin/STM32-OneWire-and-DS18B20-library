#ifndef __OneWireUART_H
#define __OneWireUART_H

#include "stm32f10x.h"

#define OWU_TIM                 TIM2                // Defines what TIM to use
#define OWU_TIM_IRQn            TIM2_IRQn           // Interrupt number of OWU_TIM
#define OWU_TIM_IRQHandler      TIM2_IRQHandler     // Exception handler of OWU_TIM

#define OWU_USART               USART2              // Defines what USART to use
#define OWU_USART_IRQn          USART2_IRQn         // Interrupt number of OWU_USART
#define OWU_USART_IRQHandler    USART2_IRQHandler   // Exception handler of OWU_USART
#define OWU_GPIO                GPIOA               // GPIOx, используемый USART
#define OWU_TX_PIN              GPIO_Pin_2
#define OWU_RX_PIN              GPIO_Pin_3

#define OWU_SEQUENSE_MAX_SIZE   53                  // Максимальный размер последовательности в байтах
#define OWU_BUFFER_MAX_SIZE     16                  // Максимальный размер буфера в байтах; каждый прочтенный с устройства бит занимает 1 байт в буфере

void OWU_Init();                                    // Настраивает перефирию; включить тактирование нужной переферии необходимо до вызова функции
void OWU_AddWriteBit(uint8_t bit);                  // Добавляет запись бита в последовательность,    занимает 1 байт последовательности
void OWU_AddWriteByte(uint8_t data);                // Добавляет запись байта в последовательность,   занимает 8 байт последовательности
void OWU_AddReadBit();                              // Добавляет чтение бита в последовательность,    занимает 1 байт последовательности
void OWU_AddReadByte();                             // Добавляет чтение байта в последовательность,   занимает 8 байт последовательности
void OWU_AddDelay(uint16_t delay_ms);               // Добавляет задержку в мс в последовательность,  занимает 3 байта последовательности
void OWU_AddReset();                                // Добавляет Reset в последовательность,          занимает 1 байт последовательности
void OWU_NewSequence();                             // Начинает запись новой последовательности
void OWU_ProcessSequence();                         // Начинает выполнение последовательности
uint8_t OWU_ReadBitFromBuffer();                    // Возвращает бит из буфера
uint8_t OWU_ReadByteFromBuffer();                   // Возвращает байт из буфера
uint8_t OWU_IsDeviceDetected();                     // Возвращает 1, если на линии обнаружено хотя бы одно устройство, иначе 0
uint8_t OWU_IsReady();                              // Возвращает 1, если выполнение последовательности завершено, иначе 0

#endif // __OneWireUART_H
