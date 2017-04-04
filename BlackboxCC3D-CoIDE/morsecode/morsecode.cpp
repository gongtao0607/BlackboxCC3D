#include <stdint.h>
#include <stdio.h>
#include <stm32f10x.h>
#include <string.h>
#include "delay.h"
#include <ctype.h>
#include "morsecode.h"
char morse_string[MORSE_STRING_SIZE]={};

static const uint8_t morse_alpha[] = {
        0b10000010, 0b01110100, 0b01010100, 0b01100011,
        0b10000001, 0b11010100, 0b00100011, 0b11110100,
        0b11000010, 0b10000100, 0b01000011, 0b10110100,
        0b00000010, 0b01000010, 0b00000011, 0b10010100,
        0b00100100, 0b10100011, 0b11100011, 0b00000001,
        0b00100011, 0b11100100, 0b10000011, 0b01100100,
        0b01000100, 0b00110100
};
static const uint8_t morse_digit[] = {
		0b00000101,
		0b10000101,
		0b11000101,
		0b11100101,
		0b11110101,
		0b11111101,
		0b01111101,
		0b00111101,
		0b00011101,
		0b00001101
};
//bits 0-2 are used to represent number of dots and dashes that represent character.
//bits 3-7 hold the actual letters representation, 1 represents a ‘dot’, 0 goes for a ‘dash’.

static void led_on(){
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);
}
static void led_off(){
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_SET);
}
static void send_char(const char c){
	/* get code */
	uint8_t code;
	if(isalpha(c)){
		code = morse_alpha[toupper(c) - 'A'];
	}else if(isdigit(c)){
		code = morse_digit[c - '0'];
	}else{
		code = 0;
		delay_ms(DOT_DURATION * 4);
	}
	uint8_t i = code & 0x7;
	/* process every bit of morse code */
	while (i--) {
		led_on();
		/* dot */
		if (code & 0x80) {
			delay_ms(DOT_DURATION);
		/* dash */
		} else {
			delay_ms(DOT_DURATION * 3);
		}
		led_off();
		/* dot space */
		delay_ms(DOT_DURATION);
		/* next bit */
		code = code << 1;
	}
	delay_ms(DOT_DURATION * 3);
}

void morse_send(){
	for(uint8_t i=0;i<MORSE_STRING_SIZE&&morse_string[i]!=0;++i){
		send_char(morse_string[i]);
	}
	delay_ms(DOT_DURATION * 4);
}
void morse_error_code(uint8_t code){
	snprintf(morse_string,4,"%hhd",code);
	morse_string[2]=0;
}
void morse_error_string(const char*s,size_t n){
	if(n>=MORSE_STRING_SIZE)return;
	morse_string[n]=0;
	strncpy(morse_string,s,n);
}
