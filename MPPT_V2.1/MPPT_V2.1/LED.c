/*
 * LED.c
 *
 * Created: 12-1-2020 13:35:13
 *  Author: Mirko
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define offset 2
#define inSync 0

volatile uint16_t amounts_blue;
volatile uint16_t amounts_red;
volatile uint16_t ready = 0;


void blink_times(uint16_t amount_blue){
		amounts_blue = amount_blue/5;
}

void blue_state(){
	static uint16_t times = 0;
	if(times < (amounts_blue*2)){
		PORTB ^= 0b00000100;
		times ++;
		ready |= 0b00000001;
	}else{
		PORTB &= 0b11111011;
		if((times >= amounts_blue*2+offset*2)){
			ready &= 0b11111110;
			if(ready == 0 || !inSync){
				times = 0;
			}
		}else{
			times++;
		}
	}
}

void red_state(){
	static uint16_t times = 0;
	if(times < (amounts_red*2)){
		PORTB ^= 0b00000001;
		times ++;
		ready |= 0b00000010;
	}else{
		times ++;
		PORTB &= 0b11111110;
		if(times >= amounts_red*2+offset*2){
			ready &= 0b11111101;
			if(ready == 0 || !inSync){
				times = 0;
			}
		}else{
			times++;
		}
	}
}

void errore(uint8_t error_nr){
	static uint16_t error_register = 0;
	
	switch(error_nr){
		case 0: 
			//Reset low PV voltage
			error_register &= 0b1111111111111110;
			break;
		case 1:
			//Low PV voltage
			error_register |= 0b0000000000000001;
			break;
		case 2:
			//Wrong battery
			error_register |= 0b0000000000000010;
			break;
		case 3: 
			//Cell voltage differance to high
			error_register |= 0b0000000000000100;
			break;
		case 4:
			//ADC fault (ADC reached default)
			error_register |= 0b0000000000001000;
			break;
		case 5:
			//Fatal Error (Te hoge waarde geconstateerd)
			error_register |= 0b0000000000010000;
			break;
		case 6:
			//Wrong measurment
			error_register |= 0b0000000000100000;
			break;
		default:
			//Not here
			break;
	}
	amounts_red = error_register;
}