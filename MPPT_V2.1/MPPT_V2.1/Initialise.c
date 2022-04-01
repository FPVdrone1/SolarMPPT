/*
 * Initialise.c
 *
 * Created: 30-12-2019 19:17:39
 *  Author: Mirko
 */ 

#define auto_lipo 0

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

#define baud_user 57600
#define baudrate 16//((F_CPU)/16/(baud_user)-1)

#include "LED.h"

void delay(uint32_t delaytime){
	for(uint32_t j=0; j < delaytime; j++){for(uint16_t k=0; k < 1770; k++){}}
}

void initialise_ADC(){
	ADMUX &= 0b11011111;
	ADMUX |= (1<<REFS0); //Set REF on AREF pin
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADIE);
	DIDR0 = (1<<ADC0D) | (1<<ADC1D);
	
	//Start first sample
	ADMUX |=  (1<<MUX0);	
	ADCSRA |= (1<<ADSC) | (1<<MUX1); //Start ADC
}

void initialise_PWM(){
	/*
	* Set Mode 7: Fast PWM (TOP:OCRA, BOTTOM:Reset)
	* Prescaler: 0
	* Clear OCOB on compare, Set OCOB at BOTTOM
	*/
	
	DDRD |= 0b00101000; //Initialise output
	
	//Set timer 1
	TCCR0A = (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<WGM02) | (1<<CS00) ;//| (1<<CS01);
	OCR0A = 127;
	OCR0B = 0;
	
	//Set timer 2
	TCCR2A =  (1<<COM2B1) | (1<<WGM21) | (1<<WGM20);
	TCCR2B = (1<<WGM22) | (1<<CS20);
	OCR2A = 127;
	OCR2B = 0;
	
	//sync timers
	TCNT0 = 0;
	TCNT2 = 6;
}


void initialise_led(){
	DDRB |= 0x07;
	PORTB &= 0b11111000;	//Turn off all lights
	//PORTB |= 0b00000010;  //Turn on Green led (Start proces)
	
	
	//Initialize counter 1
	TCCR1B |= (1 << CS12) | (1<<CS10) | (1<<WGM12);
	OCR1A = 3891;
	TIMSK1 |= (1<<OCIE1A);
}

void initialise_input(){
	//Input 2s/3s sellect pin (PD4)
	DDRD &=  0b11101111; //PD4 input
	PORTD |= 0b00010000; //Pull-up on
	
	//Input MOSI pin en SCK pin
	PORTB &= 0b11010111;
	PORTB |= 0b00101000;
}

void initialise_uart(){
	unsigned int ubrr = 16;//BAUD_RATE_230400_BPS;
	UBRR0H = (ubrr>>8);
	UBRR0L = (ubrr);
	
	UCSR0C = 0x06;       /* Set frame format: 8data, 1stop bit  */
	UCSR0B = (1<<TXEN0); /* Enable  transmitter                 */
}

int check_bat(int* Vout, int* cell1, int* cell2){
	uint8_t battery_type = 0;
	
	for(uint16_t i=0; i < 100; i++){
		for(uint32_t j=0; j < 100; j++){for(uint16_t k=0; k < 1770; k++){}}
		PORTB ^= 0b00000010;
	}
	
	while(*Vout <= 5000){		//Check if battery is higher then 5V
		PORTB ^= 0b00000010;
		for(uint32_t j=0; j < 25; j++){for(uint16_t k=0; k < 1770; k++){}}
	}
	
	PORTB &= 0b11111101;
	
	for(uint32_t j=0; j < 1000; j++){for(uint16_t k=0; k < 1770; k++){}}
	
	if(!auto_lipo){
		if(!(PIND&0x10)){			//If jumper solderd
			//Selected 3S battery			
			if(*Vout < 9000){
				errore(2);
				while(1);			//Safety Wrong battery type
			}
			battery_type = 3;
		}else{
			//Selected 2s battery
			if(*Vout > 9000){
				errore(2);
				while(1);			//Safety Wrong battery type
			}
			battery_type = 2;
		}
	}else{
		if(*Vout > 9000){
			battery_type = 3;
		}else{
			battery_type = 2;
		}
	}
	
	uint8_t ballance = 0;
	
	cli();
	for(uint16_t i=0; i < (battery_type*2)+1 ; i++){
		for(uint32_t j=0; j < 500; j++){for(uint16_t k=0; k < 1770; k++){}}	
		//PORTB ^= 0b00000010;
		if(battery_type == 3 || battery_type == 13){
			if(*cell1 > 3000 && *cell2 >3000){
				PORTB ^= 0b00000100;
				ballance = 10;
			}else{
				PORTB ^= 0b00000010;
			}
		}else{
			if(*cell1 > 3000 ){
				PORTB ^= 0b00000100;
				ballance = 10;
			}else{
				PORTB ^= 0b00000010;
			}
		}
	}
	sei();
	PORTB &= 0b11111001;
	
	
	
	
	return battery_type + ballance;
}


void initialise_ADC_multiplier(uint16_t* Vout, uint16_t* Iout, uint16_t* Vin, uint16_t* Iin, uint16_t* cell2){
	
	if(*cell2 >= 9000){			//enter calibrating mode
		
	}
}


