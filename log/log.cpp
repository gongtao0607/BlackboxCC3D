#include <stdio.h>
#include <stm32f10x.h>
#include <string.h>
#include "usb.h"
#define cdc_string_size 64
int16_t cdc_string_pos=0;
char cdc_string[cdc_string_size];

#define str_tmp_size 16
char str_tmp[str_tmp_size];
uint16_t str_tmp_len;

void log_send(const unsigned char *pucBuffer, unsigned long ulCount)
{
    while(ulCount--)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}
void cdc_send(const char *pucBuffer, uint16_t ulCount)
{
	if(ulCount>cdc_string_size)return;
	if(cdc_string_pos+ulCount>=cdc_string_size){
		CDC_Send_DATA ((uint8_t*)cdc_string, cdc_string_pos);
		cdc_string_pos=0;
	}
	memcpy(cdc_string+cdc_string_pos ,pucBuffer, ulCount);
	cdc_string_pos+=ulCount;
}

void log_1(uint8_t n){
	log_send((uint8_t*)&n,sizeof(n));
	str_tmp_len=snprintf(str_tmp, str_tmp_size, "%4hhu",n);
	cdc_send(str_tmp,str_tmp_len);
}
void log_1(uint16_t n){
	log_send((uint8_t*)&n,sizeof(n));
	str_tmp_len=snprintf(str_tmp, str_tmp_size, "%6hu",n);
	cdc_send(str_tmp,str_tmp_len);
}
void log_1(float n){
	log_send((uint8_t*)&n,sizeof(n));
	str_tmp_len=snprintf(str_tmp, str_tmp_size, "%8.4f", n);
	cdc_send(str_tmp,str_tmp_len);
}
void log_sync(){
	uint16_t n=0x7f7f;
	log_send((uint8_t*)&n,sizeof(n));
	str_tmp_len=snprintf(str_tmp, str_tmp_size, "\r\n");
	cdc_send(str_tmp,str_tmp_len);
}

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
