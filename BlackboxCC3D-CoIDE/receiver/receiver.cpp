#include <stdio.h>
#include <stm32f10x.h>
#include "delay.h"
#include "receiver.h"
#include "morsecode.h"

uint16_t receiver_channel[7];

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void USART_Configuration()
{
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

}

void GPIO_Configuration(){
	//TX
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
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
}

void receiver_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1, ENABLE);

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

extern "C" void USART1_IRQHandler(void)
{
	/*
	 * Note:
	 * A bug from library
	 * http://electronics.stackexchange.com/questions/224714/stm32-uart-interrupt-is-triggering-without-any-flags-getting-set
	 * http://electronics.stackexchange.com/questions/222638/clearing-usart-uart-interrupt-flags-in-an-stm32
	 * If EIE is not enabled, GetITStatus won't return ORE as SET
	 * However, ORE is SET (when RXNEIE SET) and triggers the interrupt, and require a ReceiveData to clear
	 *
	 * Workaround:
	 * Check ORE flag by USART_GetFlagStatus(USART_FLAG_ORE)
	 */

	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET){
		uint16_t d = USART_ReceiveData(USART1);
		if(millis-last_rx_ts>5){//starting of a frame
			pos=0;
			if(last_rx_ts!=0 && millis - last_rx_ts >24){
				morse_error_code(ERROR_RECEIVER_RECEIPTION);
			}
		}
		if(pos<sizeof(frame_raw)&&pos>=0){
			frame_raw[pos++]=(uint8_t)d;
		}
		if(pos>=sizeof(frame_raw)){
			process();
			pos=0;
		}
		last_rx_ts=millis;
	} else if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) == SET) {
		USART_ReceiveData(USART1);
	}
}
