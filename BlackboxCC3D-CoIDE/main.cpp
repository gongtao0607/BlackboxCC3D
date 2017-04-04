#include <stdio.h>
#include <stm32f10x.h>
#include "pwm.h"
#include "imu.h"
#include "log.h"
#include "delay.h"
#include "receiver.h"
#include "usb.h"
#include "morsecode.h"

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
	pwm_init();
	led_init();
	setvbuf(stdout, NULL, _IONBF, 0);
	delay_init();
	imu_init();
	receiver_init();
	usb_init();
	log_init();
	morse_error_code(ERROR_NOERROR);
	while(1){
		morse_send();
	}
}
