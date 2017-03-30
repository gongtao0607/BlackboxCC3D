#include <stdio.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include <misc.h>
#include <string.h>
#include "pwm.h"
#include "mpu.h"
#include "delay.h"
#include "math.h"

void led_init(){
	//Enable remap clock (AFIO)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//Disable JTAG (enable PB3)
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_SET);
	//GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);
}

void uart_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}
extern "C" void UART_PutChar(const char c){
	USART_SendData(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);// Last Version USART_SendData(USART1,(uint16_t) *pucBuffer++);
        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}
#define OPENLOG
#ifdef OPENLOG
void log_1(uint8_t n){
	UARTSend((uint8_t*)&n,sizeof(n));
}
void log_1(uint16_t n){
	UARTSend((uint8_t*)&n,sizeof(n));
}
void log_1(float n){
	UARTSend((uint8_t*)&n,sizeof(n));
}
void log_sync(){
	uint16_t n=0x7f7f;
	UARTSend((uint8_t*)&n,sizeof(n));
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

int main(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	led_init();
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);
	uart_init();
	pwm_init();
	mpu_init();
	while(1){
		static uint16_t pn=0;
		static uint8_t packet[2+2+2*6+3*4]={};
		++pn;
		extern uint32_t millis;
		//pn=(uint16_t)millis;
		log_sync();
		log_1(pn);
		log_1((uint16_t)millis);
		for(uint8_t i=0;i<5;++i){
			log_1(pwm_duty_width[i]);
		}
		log_1(pwm_cycle_width[5]);
		for(uint8_t i=0;i<4;++i){
			log_1(quaternion[i]);
		}
	}
}
