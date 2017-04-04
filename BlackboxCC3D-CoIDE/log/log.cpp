#include <stdio.h>
#include <stm32f10x.h>
#include <string.h>
#include "usb.h"

#include "pwm.h"
#include "imu.h"
#include "receiver.h"
#include "usb.h"
#include "delay.h"

void log_send(const unsigned char *pucBuffer, unsigned long ulCount)
{
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}

void log_1(uint8_t n){
	log_send((uint8_t*)&n,sizeof(n));
	printf("%4hhu",n);
}
void log_1(uint16_t n){
	log_send((uint8_t*)&n,sizeof(n));
	printf("%6hu",n);
}
void log_1(float n){
	log_send((uint8_t*)&n,sizeof(n));
	printf("%8.4f", n);
}
void log_sync(){
	uint16_t n=0x7f7f;
	log_send((uint8_t*)&n,sizeof(n));
	printf("\r\n");
}

void tim_config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 100000) - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 800-1; //10ms
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM1, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
}

void log_init()
{
	/*
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);*/

	tim_config();
}
extern "C" void log_putchar(const char c)
{
	USART_SendData(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}
extern "C" void TIM1_UP_IRQHandler(){
	if(TIM_GetITStatus(TIM1, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		static uint16_t pn=0;
		uint8_t i;
		++pn;
		log_sync();
		log_1(pn);
		log_1((uint16_t)millis);
		for(i=0;i<5;++i){
			log_1(pwm_duty_width[i]);
		}
		log_1(pwm_cycle_width[5]);
		for(i=0;i<4;++i){
			log_1(quaternion[i]);
		}
		for(i=0;i<7;++i){
			log_1(receiver_channel[i]);
		}
	}
}
