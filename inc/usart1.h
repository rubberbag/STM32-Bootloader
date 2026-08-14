#ifndef USART1_H
#define USART1_H

#include <stdint.h>


void USART1_Init(void);
void USART1_RX_Start(void);

void USART1_SendChar(char c);
void USART1_SendString(const char *str);

void USART1_RX_Byte(uint8_t byte);

//uint16_t RX_Process(uint8_t **data);
void USART1_RX_Complete(void);

#endif