#include <stm32f10x_rcc.h>
#include <misc.h>
volatile uint32_t millis;
#define reload_ticks (SystemCoreClock/1000)
extern "C" void SysTick_Handler (void)
{
	millis++;
}
void delay_init()
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config (reload_ticks);
	NVIC_SetPriority(SysTick_IRQn,NVIC_EncodePriority(4,1,0));
}

void delay_ms(uint32_t t)
{
  uint32_t start, end;
  start = millis;
  end = start + t;
  if (start < end) {
    while ((millis >= start) && (millis < end));
  } else {
    while ((millis >= start) || (millis < end));
  }
}

void delay_us(float t)
{
	uint32_t start, end;
	start = SysTick->VAL;
	end = (start - t * (SystemCoreClock/1000000));
	if(end > start) end += reload_ticks;
	if (end < start) {
	  while ((SysTick->VAL <= start) && (SysTick->VAL > end));
	} else {
	  while ((SysTick->VAL <= start) || (SysTick->VAL > end));
	}
}
