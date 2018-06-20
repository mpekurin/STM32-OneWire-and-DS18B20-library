#ifndef __OneWireUART_H
#define __OneWireUART_H

#include "stm32f10x.h"

#define OWU_USART               USART1              // Define what USART to use
#define OWU_USART_IRQn          USART1_IRQn         // Interrupt number of OWU_USART
#define OWU_USART_IRQHandler    USART1_IRQHandler   // Exception handler of OWU_USART
#define OWU_GPIO                GPIOB               // GPIOx used by OWU_USART
#define OWU_TX_PIN              GPIO_Pin_6          // OWU_USART TX pin
#define OWU_RX_PIN              GPIO_Pin_7          // OWU_USART RX pin

#define OWU_SEQUENSE_MAX_SIZE   53                  // Maximum sequence size, bytes
#define OWU_BUFFER_MAX_SIZE     16                  // Maximum buffer size, bytes; every readed bit requires 1 byte in buffer

void OWU_Init(TIM_TypeDef* TIMx);                   // OWU_USART peripheral clock must be enabled before call
void OWU_AddWriteBit(uint8_t bit);                  // Add writing bit operation to sequence,      requires 1 byte
void OWU_AddWriteByte(uint8_t data);                // Add writing byte operation to sequence,     requires 8 bytes
void OWU_AddReadBit();                              // Add reading bit operation to sequence,      requires 1 byte
void OWU_AddReadByte();                             // Add reading byte operation to sequence,     requires 8 bytes
void OWU_AddDelay(uint16_t delay_ms);               // Add delay in ms to sequence,                requires 3 bytes
void OWU_AddReset();                                // Add Reset command to sequence,              requires 1 byte
void OWU_NewSequence();                             // Start writing new sequence
void OWU_ProcessSequence();                         // Start sequence processing
uint8_t OWU_ReadBitFromBuffer();                    // Return bit from buffer
uint8_t OWU_ReadByteFromBuffer();                   // Return byte from buffer
uint8_t OWU_IsDeviceDetected();                     // Return 1 if at least one device is found, otherwise return 0
uint8_t OWU_IsReady();                              // Return 1 if sequence processing completed, otherwise return 0
void OWU_TIM_Handler();

#endif // __OneWireUART_H
