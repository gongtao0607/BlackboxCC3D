#ifndef _RECEIVER_H_
#define _RECEIVER_H_
#define RECEIVER_MAIN_PORT

#ifdef RECEIVER_MAIN_PORT
	#define RECEIVER_UART USART1
	#define RECEIVER_UARTIRQ USART1_IRQn
	#define RECEIVER_UART_HANDLER USART1_IRQHandler
#else
	#define RECEIVER_UART USART3
	#define RECEIVER_UARTIRQ USART3_IRQn
	#define RECEIVER_UART_HANDLER USART3_IRQHandler
#endif
extern void receiver_init();
extern uint16_t receiver_channel[7];
#endif
