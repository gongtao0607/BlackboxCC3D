#include <stdio.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include <misc.h>
#include "log.h"

#define OPENLOG
#ifdef OPENLOG
void log_1(uint8_t n){
	log_send((uint8_t*)&n,sizeof(n));
}
void log_1(uint16_t n){
	log_send((uint8_t*)&n,sizeof(n));
}
void log_1(float n){
	log_send((uint8_t*)&n,sizeof(n));
}
void log_sync(){
	uint16_t n=0x7f7f;
	log_send((uint8_t*)&n,sizeof(n));
}
#else
void log_1(uint8_t n){
	printf("%4hhu",n);
}
void log_1(uint16_t n){
	printf("%6hu",n);
}
void log_1(float n){
	printf(" %4.4f",n);
}
void log_sync(){
	printf("\r\n");
}
#endif


void log_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1, ENABLE);

	//TX
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//INV
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOB, GPIO_Pin_2, Bit_RESET);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}
extern "C" void log_putchar(const char c)
{
	USART_SendData(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

void log_send(const unsigned char *pucBuffer, unsigned long ulCount)
{
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}
