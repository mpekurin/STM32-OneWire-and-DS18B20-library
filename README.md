# STM32 OneWire library
OneWire library allows you to write and read data to/from slave devices using any USART. Also it allows you to use delays between data transmissions using any timer so processor can do something useful at that time.

## How to initialize:
1. Configure USART and pins in OneWireUART.h file.
2. Enable OWU_USART peripheral clock.
3. Call OWU_Init.

## How to use:
1. Start writing new sequence using OWU_NewSequence function.
2. Write required commands and delays using OWU_Add... functions.
3. Start processing sequence using OWU_ProcessSequence function.
4. Wait for OWU_IsReady returns 1.
5. Read data using OWU_Read... functions if required.
6. Send same sequence as many times as you like using OWU_ProcessSequence again.

# STM32 DS18B20 library
This library allows you to periodically get temperature from DS18B20 thermometer using OneWire library with configurable period.

## How to initialize:
1. Call DS18B20_Init.

## How to use:
1. Wait for DS18B20_IsReady returns 1.
2. Get measured temperature using DS18B20_GetRawTemperature or DS18B20_GetRealTemperature.
3. Start new temperature measurement using DS18B20_ConvertTemperature if required.