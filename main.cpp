#include <stdio.h>
#include <stm32f10x.h>
#include "pwm.h"
#include "imu.h"
#include "log.h"
#include "delay.h"
#include "math.h"
#include "receiver.h"
#include "usb.h"

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
int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();
	led_init();
	pwm_init();
	mpu_init();
	log_init();
	receiver_init();
	usb_init();
	while(1){
//#define NOLOG
#ifndef NOLOG
		static uint16_t pn=0;
		uint8_t i;
		++pn;
		extern uint32_t millis;
		//pn=(uint16_t)millis;
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
#endif
	}
}
