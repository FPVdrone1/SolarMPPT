/*
 * Algoritme.c
 *
 * Created: 4-2-2020 11:54:38
 *  Author: Mirko
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define max_ocr 5
#define offset_pwm 15

#define Ibatter_max 5000
#define Isteady 150

#define step 1

int MPPT2(int voltage, int current, int induty){
		int power = 0;
		int static power_k = 20;
		int static voltage_k = 0;
		//int static current_k = 0;
		int static deltaP = 0;
		int static deltaV = 0;
		int duty = induty;
		
		
		power = (voltage * current);
		deltaP = power - power_k;
		deltaV = voltage - voltage_k;
		
		
		
		if(power == 0 && 1){
			//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
			//UDR0 = 0x30;
			duty += step;
			}else{
			if((deltaP >= 0 && current >= 10) || ((deltaP > 0 && current < 10))){
				if(deltaV > 0){
					duty -= step;
					//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
					//UDR0 = 0x31;
					}else{
					duty += step;
					//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
					//UDR0 = 0x32;
				}
				}else{
				if(deltaV >= 0){
					duty += step;
					//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
					//UDR0 = 0x33;
					}else{
					duty -= step;
					//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
					//UDR0 = 0x34;
				}
			}
		}
		
		power_k = power;
		voltage_k = voltage;
		return duty;
		
}

//Deze werkt niet
int MPPT(int Vin, int Iin){ 
	static int power_k;
	static int Vin_k;
	int power = Vin * Iin;
	int DVin = Vin - Vin_k;
	int temp;
	
	if(((power - power_k) == 0)){
		temp = -1;
	}else{
		if((power - power_k) > 0){
			if(DVin > 0){
				temp = -1;
			}else{
				temp = 1;
			}
		}else{
			if(DVin > 0){
				temp = 1;
			}else{
				temp = -1;
			}
		}
	}
	power_k = power;
	Vin_k = Vin;
	return temp;
}

int Battery_charge(int Vout, int Iout, int Vin, int Iin, int Voc, int duty){
	PORTB &= 0b11111101;
	if(Vout >= Voc){
		if(Iout <= Isteady){
			//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
			//UDR0 = 0x35;
			PORTB |= 0b00000010;
			return 0;
			
		}else{
			//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
			//UDR0 = 0x36;
			return duty - step;
		}
	}else{
		if(Iout >= Ibatter_max){
			//while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
			//UDR0 = 0x37;
			return duty - step;
		}else{
			//PORTB |= 0b00000100;//Turn on blue light for mppt indication
			return MPPT2(Vin, Iin, duty);
		}
	}
	
}

void Voltage(int Threshold, int Value){
	uint8_t pwm0 = OCR0B;
	uint8_t pwm2 = OCR2B;
	uint8_t increase_decrease = 0;
	
	
	if(Value > Threshold && pwm2 < (OCR2A-10)){
		//Decrease value
		increase_decrease = 1;
	}else if(Value < Threshold && pwm0 > 0){
		//Increase value
		increase_decrease = 0;
	}
	
	//if(increase_decrease == 1 && pwm0 > 70){
		//// nothing
		//pwm0 --;
	//}else{
	
	if(pwm0 < 90){
		//
		if(increase_decrease == 0){pwm0 --;}
		if(increase_decrease == 1){pwm0 ++;}
		pwm2 = 0;
	}else if(pwm0 >= (OCR0A-(offset_pwm+max_ocr)) && pwm0 < (OCR0A-max_ocr)){
		if(increase_decrease == 0){pwm0 --;}
		if(increase_decrease == 1){pwm0 ++;}
		if(increase_decrease == 0){pwm2 --;}
		if(increase_decrease == 1){pwm2 ++;}
	}else if(pwm2 <= offset_pwm){
		if(increase_decrease == 0){pwm0 --;}
		if(increase_decrease == 1){pwm0 ++;}
		if(increase_decrease == 0){pwm2 --;}
		if(increase_decrease == 1){pwm2 ++;}
	}else if(pwm2 > offset_pwm){
		if(increase_decrease == 0){pwm2 --;}
		if(increase_decrease == 1){pwm2 ++;}
		pwm0 = OCR0A-max_ocr;
	}
	//}
	
	
	
	
	if(pwm0 > 250){ //Through zero point 
		pwm0 = 0;
	}
	if(pwm0 > (OCR0A-7)){
		pwm0 = OCR0A-7;
	}
	
	if(pwm2 > 250){ //Through zero point 
		pwm2 = 0;
	}
	
	if(pwm2 > (OCR2A-10)){
		pwm2 = OCR2A-10;
	}
	
	OCR0B = pwm0;
	OCR2B = pwm2;
}

