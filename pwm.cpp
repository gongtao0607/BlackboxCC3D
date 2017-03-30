#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_usart.h>
#include <misc.h>
#include "pwm.h"
uint16_t pwm_duty_width[n_channels]={0xffff,0xffff,0xffff,0xffff,0xffff,0xffff};
uint16_t pwm_cycle_width[n_channels]={};
/*
 * Input -> Output
 * S1 IN	S1 OUT
 * S2 IN	S2 OUT
 * S3 IN	S3 OUT
 * S4 IN	S4 OUT
 * S5 IN	S5 OUT
 * S6 OUT	S6 IN //for RPM sensor
 */
void gpio_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//enable TIM3CH2
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	//Output
	uint16_t Pins_A=GPIO_Pin_1|GPIO_Pin_8;
	uint16_t Pins_B=GPIO_Pin_4|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = Pins_A;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, Pins_A);

	GPIO_InitStructure.GPIO_Pin = Pins_B;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, Pins_B);

	//Input
	Pins_A=GPIO_Pin_0|GPIO_Pin_2;
	Pins_B=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5|GPIO_Pin_6;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = Pins_A;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = Pins_B;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void tim_init(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t)-1;//free run?
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM2, ENABLE);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3, ENABLE);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM4, ENABLE);

	TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x00;

    //S1 IN
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
	TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);

    //S2 IN
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    //S3 IN
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    //S4 IN
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4);
    TIM_ITConfig(TIM3, TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4, ENABLE);

    //S5 IN
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    //S6 OUT
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
	TIM_ClearITPendingBit(TIM2, TIM_IT_CC1|TIM_IT_CC3);
    TIM_ITConfig(TIM2, TIM_IT_CC1|TIM_IT_CC3, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}
struct{
	TIM_TypeDef*tim;
	uint16_t tim_icpolarity;
	uint16_t tim_it;
	void (*tim_polarityconfig)(TIM_TypeDef *,uint16_t);
	uint16_t (*tim_getcapture)(TIM_TypeDef *);
}input_channels[n_channels]={
	{TIM4, TIM_ICPolarity_Rising, TIM_IT_CC1, TIM_OC1PolarityConfig, TIM_GetCapture1},
	{TIM3, TIM_ICPolarity_Rising, TIM_IT_CC2, TIM_OC2PolarityConfig, TIM_GetCapture2},
	{TIM3, TIM_ICPolarity_Rising, TIM_IT_CC3, TIM_OC3PolarityConfig, TIM_GetCapture3},
	{TIM3, TIM_ICPolarity_Rising, TIM_IT_CC4, TIM_OC4PolarityConfig, TIM_GetCapture4},
	{TIM2, TIM_ICPolarity_Rising, TIM_IT_CC1, TIM_OC1PolarityConfig, TIM_GetCapture1},
	{TIM2, TIM_ICPolarity_Rising, TIM_IT_CC3, TIM_OC3PolarityConfig, TIM_GetCapture3}
};
void pwm_edge_trigger(uint8_t ch, uint8_t state, uint16_t capture){
	struct {
		GPIO_TypeDef* port;
		uint16_t pin;
		uint16_t rising_capture;
	}static pwm_out[]={
		{GPIOB, GPIO_Pin_9, 0},
		{GPIOB, GPIO_Pin_8, 0},
		{GPIOB, GPIO_Pin_7, 0},
		{GPIOA, GPIO_Pin_8, 0},
		{GPIOB, GPIO_Pin_4, 0},
		{GPIOA, GPIO_Pin_1, 0}
	};
	if(state){ //0->1
		pwm_cycle_width[ch]=capture-pwm_out[ch].rising_capture;
		pwm_out[ch].rising_capture=capture;
		pwm_out[ch].port->BSRR = pwm_out[ch].pin;
	}else{ //1->0
		pwm_duty_width[ch]=capture-pwm_out[ch].rising_capture;
		pwm_out[ch].port->BRR = pwm_out[ch].pin;
	}
}

void TIM_IRQHandler(){
	for (int i = 0; i < n_channels; ++i) {
		TIM_TypeDef*tim = input_channels[i].tim;
		uint16_t tim_icpolarity = input_channels[i].tim_icpolarity;
		uint16_t tim_it = input_channels[i].tim_it;
		void (*tim_polarityconfig)(TIM_TypeDef *,uint16_t) = input_channels[i].tim_polarityconfig;
		uint16_t (*tim_getcapture)(TIM_TypeDef *) = input_channels[i].tim_getcapture;
		if(TIM_GetITStatus(tim, tim_it)!= RESET){ //captured!
			TIM_ClearITPendingBit(tim, tim_it);
			pwm_edge_trigger(i, tim_icpolarity==TIM_ICPolarity_Rising, tim_getcapture(tim));
			if(tim_icpolarity==TIM_ICPolarity_Rising){
				tim_icpolarity=TIM_ICPolarity_Falling;
			}else{
				tim_icpolarity=TIM_ICPolarity_Rising;
			}
			input_channels[i].tim_icpolarity=tim_icpolarity;
			tim_polarityconfig(tim,tim_icpolarity);
		}

	}
}
extern "C" void TIM2_IRQHandler(){
	TIM_IRQHandler();
}
extern "C" void TIM3_IRQHandler(){
	TIM_IRQHandler();
}
extern "C" void TIM4_IRQHandler(){
	TIM_IRQHandler();
}

void pwm_init(){
	gpio_init();
	tim_init();
}
