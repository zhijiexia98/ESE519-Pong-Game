/*
 * ESE519_Lab4_Pong_Starter.c
 *
 * Created: 9/21/2021 21:21:21 AM
 * Author : Zhijie Xia
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include "uart.h"
#include "stdio.h"

char String[25];
uint8_t pos_start = 52;
uint8_t pos_end = 76;

uint8_t pos_start_ai = 52;
uint8_t pos_end_ai = 76;
uint8_t flag = 0;
uint8_t bflag = 0;

void Initialize()
{
	lcd_init();
	
	cli();
	//initialize ADC
	PRR &=~(1<<PRADC);
	//Select Vref = AVcc
	ADMUX |= (1<<REFS0);
	ADMUX &=~(1<<REFS1);
	//Set the ADC clock time div by 128
	//125kHz
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	//select channel 0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	//set to auto trigger
	ADCSRA |= (1<<ADATE);
	//set to free running
	ADCSRB &=~(1<<ADTS0);
	ADCSRB &=~(1<<ADTS1);
	ADCSRB &=~(1<<ADTS2);
	//disable digital input buffer on ADC PIN
	DIDR0 |= (1<<ADC0D);
	//ENBALE ADC
	ADCSRA |= (1<<ADEN);
	//start conversion
	ADCSRA |= (1<<ADSC);
	
	//configuration for the LEDs
	DDRC |= (1<<DDC1);
	DDRC |= (1<<DDC2);
	
	//configuration for buzzer
	 DDRD |= (1<<DDD3);
	 
	 //configuration for 2 pins from WIFI module
	 DDRC &= ~(1<<DDC3);
	 DDRC &= ~(1<<DDC4); 

	sei();

}


int main(void)
{
	Initialize();
	
	UART_init(BAUD_PRESCALER); 
	LCD_setScreen(YELLOW);
	//LCD_drawLine(10, 20, 60, 100, BLACK);
	//LCD_drawCircle(50, 70, 40, BLACK);
	//LCD_drawBlock(10,10,40,40,BLACK);
	//object_bouncing(YELLOW);
	int x0 = 80;
	int y0 = 80;
	int x1 = 82;
	int y1 = 82;
	int w = abs(x1-x0);
	int sw = x0<x1 ? 1:-1;
	int h = -abs(y1-y0);
	int sh = y0<y1 ? 1:-1;
	int err = w+h;
	
	int score = 0;
	char score_char[1];
	int round = 0;
	char round_char[1];
	int score_ai = 0;
	char score_ai_char[1];
	int round_ai = 0;
	char round_ai_char[1];
    while (1) 
    {	
		sprintf(score_char, "%d", score);
		sprintf(round_char, "%d", round);
		sprintf(score_ai_char, "%d", score_ai);
		sprintf(round_ai_char, "%d", round_ai);
		LCD_drawChar(30, 5, score_char[0], BLACK, YELLOW);
		LCD_drawChar(60, 5, round_char[0], BLACK, YELLOW);
		LCD_drawChar(100, 5, round_ai_char[0], BLACK, YELLOW);
		LCD_drawChar(130, 5, score_ai_char[0], BLACK, YELLOW);
		if(flag == 0){
			LCD_drawBlock(156,pos_start_ai,159,pos_end_ai,BLACK);
			LCD_drawBlock(156,pos_start_ai,159,pos_end_ai,YELLOW);
			pos_start_ai += 1;
			pos_end_ai += 1;
			if(pos_end_ai >= 127){
				flag = 1;
			}
		}
		if(flag == 1){
			LCD_drawBlock(156,pos_start_ai,159,pos_end_ai,BLACK);
			LCD_drawBlock(156,pos_start_ai,159,pos_end_ai,YELLOW);
			pos_start_ai -= 1;
			pos_end_ai -= 1;
			if(pos_end_ai <= 24){
				flag = 0;
			}
		}
		
		//joystick
		if((ADC <= 600 && ADC >= 400)){
			LCD_drawBlock(0,pos_start,3,pos_end,BLACK);
		}

		if(ADC < 400 || PINC & (1<<PINC3)){
			if(pos_end > 24){
				LCD_drawBlock(0,pos_start,3,pos_end,BLACK);
				LCD_drawBlock(0,pos_start,3,pos_end,YELLOW);
				pos_start -= 1;
				pos_end -= 1;
			}
			if(ADC > 600 || PINC & (1<<PINC4)){
				continue;
			}
			if(pos_end <= 24){
				pos_start = 0;
				pos_end = 24;
				LCD_drawBlock(0,pos_start,3,pos_end,BLACK);
				LCD_drawBlock(0,pos_start,3,pos_end,YELLOW);
			}
		}
		if(ADC > 600 || PINC & (1<<PINC4)){
			if(pos_start < 103){
				LCD_drawBlock(0,pos_start,3,pos_end,BLACK);
				LCD_drawBlock(0,pos_start,3,pos_end,YELLOW);
				pos_start += 1;
				pos_end += 1;
			}
			if(ADC < 400 || PINC & (1<<PINC3)){
				continue;
			}
			if(pos_start >= 103){
				pos_start = 103;
				pos_end = 127;
				LCD_drawBlock(0,pos_start,3,pos_end,BLACK);
				LCD_drawBlock(0,pos_start,3,pos_end,YELLOW);
			}
		}
		
		//bouncing ball
		for (int i=x0-11; i<x0+11; i++){
			for (int j=y0-11; j<y0+11; j++){
				if ((i-x0)*(i-x0) + (j-y0)*(j-y0) <= 9){
					LCD_drawPixel(i,j,BLACK);
				}
			}
		}
		for (int i=x0-11; i<x0+11; i++){
			for (int j=y0-11; j<y0+11; j++){
				if ((i-x0)*(i-x0) + (j-y0)*(j-y0) <= 9){
					LCD_drawPixel(i,j,YELLOW);
				}
			}
		}
		
		if (x0 <= 0 || x0 >= 159){
			sw = -sw;
		}
		if(y0 <= 0 || y0 >= 127){
			sh = -sh;
		}
		int err2= 2 * err;
		if (err2 >= h){
			err += h;
			x0 +=sw;
		}
		if (err2 <= w){
			err += w;
			y0 += sh;
		}
		//computer wins
		if(x0 <= 0 && (y0>pos_end || y0<pos_start)){
			PORTC |= (1<<PORTC1);
			for(int i = 0; i < 10; i++){
				PORTD |= (1<<PORTD3);
				_delay_ms(65);
				PORTD &= ~ (1<<PORTD3);
				_delay_ms(35);
			}
			PORTC &= ~ (1<<PORTC1);
			if(score_ai <= 1){
				score_ai += 1;
			}else{
				score_ai = 0;
				score = 0;
				if(round_ai <= 1){
					round_ai += 1;
				}else{
					round_ai = 0;
					round = 0;
				}
			}
			sprintf(String,"computer score %d \r\n",score_ai);
			UART_putstring(String);
			x0 = 80;
			y0 = (rand() % (94 - 34 + 1)) + 34;
			pos_start = 52;
			pos_end = 76;
			pos_start_ai = 0;
			pos_end_ai = 24;
			flag = 0;
			if(bflag == 0){
				x1 = 78;
				y1 = y0 - 2;
				bflag = 1;
			}else{
				x1 = 82;
				y1 = y0 + 2;
				bflag = 0;
			}
			w = abs(x1-x0);
			sw = x0<x1 ? 1:-1;
			h = -abs(y1-y0);
			sh = y0<y1 ? 1:-1;
			err = w+h;
			LCD_setScreen(YELLOW);
		}
		//human wins
		if(x0 >= 159 && (y0>pos_end_ai || y0<pos_start_ai)){
			PORTC |= (1<<PORTC2);
			for(int i = 0; i < 10; i++){
				PORTD |= (1<<PORTD3);
				_delay_ms(65);
				PORTD &= ~ (1<<PORTD3);
				_delay_ms(35);
			}
			PORTC &= ~ (1<<PORTC2);
			if(score <= 1){
				score += 1;
			}else{
				score = 0;
				score_ai = 0;
				if(round <= 1){
					round += 1;
				}else{
					round = 0;
					round_ai = 0;
				}
			}
			sprintf(String,"human score %d \r\n",score);
			UART_putstring(String);
			x0 = 80;
			y0 = (rand() % (94 - 34 + 1)) + 34;
			pos_start = 52;
			pos_end = 76;
			pos_start_ai = 0;
			pos_end_ai = 24;
			flag = 0;
			if(bflag == 0){
				x1 = 78;
				y1 = y0 - 2;
				bflag = 1;
				}else{
				x1 = 82;
				y1 = y0 + 2;
				bflag = 0;
			}
			w = abs(x1-x0);
			sw = x0<x1 ? 1:-1;
			h = -abs(y1-y0);
			sh = y0<y1 ? 1:-1;
			err = w+h;
			LCD_setScreen(YELLOW);
		}
    }
}
