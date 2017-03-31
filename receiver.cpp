#include <stdio.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include <misc.h>
#include "delay.h"
#include "receiver.h"

uint16_t receiver_channel[7];

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RECEIVER_UARTIRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void GPIO_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void USART_Configuration()
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(RECEIVER_UART, &USART_InitStructure);
	USART_Cmd(RECEIVER_UART, ENABLE);
	USART_ITConfig(RECEIVER_UART, USART_IT_RXNE, ENABLE);
}
void receiver_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	GPIO_Configuration();
	USART_Configuration();
	NVIC_Configuration();
}
static uint8_t frame_raw[16];
static uint8_t pos=0;
static uint32_t last_rx_ts=0;
void process()
{
	uint8_t i;
	for(i=2;i<16;i+=2){
		uint16_t servo = (((uint16_t)frame_raw[i])<<8)+frame_raw[i+1];
		uint16_t id=(servo>>10)&0xF;
		uint16_t value=servo&((1<<10)-1);
		if(id<7&&id>=0)
			receiver_channel[id]=value;
	}
}

extern "C" void RECEIVER_UART_HANDLER(void)
{
	if (USART_GetITStatus(RECEIVER_UART,USART_IT_RXNE) == SET){
		uint16_t d = USART_ReceiveData(RECEIVER_UART);
		if(millis-last_rx_ts>5){//starting of a frame
			pos=0;
		}
		if(pos<sizeof(frame_raw)&&pos>=0){
			frame_raw[pos++]=(uint8_t)d;
		}
		if(pos>=sizeof(frame_raw)){
			process();
			pos=0;
		}
		last_rx_ts=millis;
	}
}
