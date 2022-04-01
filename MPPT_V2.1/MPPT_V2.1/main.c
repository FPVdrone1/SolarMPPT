/*
 * MPPT_V2.1.c
 *
 * Created: 25-3-2020 20:02:11
 * Author : Mirko
 */ 
#define BAUD_RATE_230400_BPS  2 // 230.4kps


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "Initialise.h"
#include "LED.h"
#include "Algoritme.h"

#define cell_max_voltage 4100
#define max_cell_difference 200

//Function deceleration
void uartprintln(int input);
void uartprint(int input);
void call_input();

//All measured value
int Vout = 0;
int Iout = 0;
int Vin = 0;
int Iin = 0;
int cell1 = 0;
int cell2 = 0;
int cell3 = 0;
int max_voltage = 1;
int cell_compensate = 0;

//Calibration value's
uint8_t call_Vout;
uint8_t call_Vin;
uint8_t call_cell1;
uint8_t call_cell2;

volatile uint8_t stop = 0;			//For exit main loop


volatile uint8_t battery_type = 0;		//Include 2 or 3 with lipo
volatile uint8_t balance_connected = 0;	


uint8_t cell_counter = 18;		//Counter for starting cell measurement
int duty = 60;






int main(void){
	//call_Vout = eeprom_read_byte((uint8_t*)1);
	call_Vin = eeprom_read_byte((uint8_t*)3);
	call_cell1 = eeprom_read_byte((uint8_t*)5);
	call_cell2 = eeprom_read_byte((uint8_t*)7);
	call_Vout = 147;
	
	for(uint32_t j=0; j < 100; j++){for(uint16_t k=0; k < 1770; k++){}}
	initialise_uart();
	initialise_input();
	initialise_led();
	initialise_ADC();
	
	
	call_input();
		
		
	
	initialise_PWM();
	
	
	
	
	sei();
// battery
	switch(check_bat(&Vout, &cell1, &cell2)){ // kijkt welke baterij er op aangelsoten zit
		case 2:
			battery_type = 2;	
			balance_connected = 0;
			break;
		case 3:
			battery_type = 3;	
			balance_connected = 0;
			break;
		case 12:								// kijkt of balance kabel is aangelsotenbalance kabel
			battery_type = 2;	
			balance_connected = 1;
			break;
		case 13:
			battery_type = 3;	
			balance_connected = 1;
			break;
		default:
			max_voltage = 2;
			break;
	}	//battery end
	//main
	max_voltage = cell_max_voltage * battery_type;  // max voltage wat je wilt hebben ligt aan cel type (alles in mv en ma 32768mv=65536)
  
	
	int voltage = 0;
	int current = 0;


    while (!stop) { // bij een fout komt ie in deze loop moet hem uit zetten om er uit te komen(de error loop)

		blink_times(((Iout/100)*(Vout/100))/100);
	//main
	//weg		
		//Hold solar values
		voltage = Vin;
		current = Iin;
	//weg	
	//main				
		//Calculate new duty cycle
		duty = Battery_charge(Vout + cell_compensate, Iout, voltage, current, max_voltage, duty);
	
		//Stop if solar voltage below 3000mV
		if(voltage <= 3000){
			duty = 0;
			errore(1);      
		}else{
			errore(0);
		}
//main
// library bug boost 	
		//Maximize duty cycle
		if(duty >= 210){duty = 210;}
		if(duty <= 0  ){duty = 0;}
		
			
		if(Vout >= 13000){
			duty = 0;
		}
		
		//Write duty to Buck FET
		if(duty > 123){
			OCR0B = 123;
			}else{
			OCR0B = duty;
		}
		
		//Write duty to boost FET
		if(duty > 120){
			OCR2B = duty-120;
			}else{
			OCR2B = 0;
		}
// library bug boost
// battery		
		if(cell_counter == 10){
			uint8_t Highest_cell = 0;
			uint8_t Lowest_cell = 0;
			if(balance_connected){
				if(battery_type == 3){
					if(cell1 > cell2){
						if(cell1 > cell3){
							Highest_cell = 1;
							if(cell2 > cell3){
								Lowest_cell = 3;
							}else{
								Lowest_cell = 2;
							}
						}else{
							Highest_cell = 3;
							Lowest_cell = 2;
						}
					}else{
						if(cell2 > cell3){
							Highest_cell = 2;
							if(cell1 > cell3){
								Lowest_cell = 3;
							}else{
								Lowest_cell = 1;
							}
						}else{
							Highest_cell = 3;
							Lowest_cell = 2;
						}
					}
				}else{
					if(cell1 > cell2){
						Highest_cell = 1;
						Lowest_cell = 2;
					}else{
						Highest_cell = 2;
						Lowest_cell = 1;
					}
				}
			}
	
			switch((Highest_cell*10) + Lowest_cell){
				case 0:
					//No ballance connected
					cell_compensate = 0;
					break;
				case 12:
					// Cell 1 highest, Cell 2 lowest
					cell_compensate = (cell1 * battery_type) - Vout;
					if((cell1 - cell2) > max_cell_difference)errore(3);
					break;
				case 13:
					// Cell 1 highest, cell 3 lowest
					cell_compensate = (cell1 * battery_type) - Vout;
					if((cell1 - cell3) > max_cell_difference)errore(3);
					break;
				case 21:
					// Cell 2 highest, cell 1 lowest
					cell_compensate = (cell2 * battery_type) - Vout;
					if((cell2 - cell1) > max_cell_difference)errore(3);
					break;
				case 23:
					// Cell 2 highest, cell 3 lowest
					cell_compensate = (cell2 * battery_type) - Vout;
					if((cell2 - cell3) > max_cell_difference)errore(3);
					break;
				case 31:
					// Cell 3 highest, cell 1 lowest
					cell_compensate = (cell3 * battery_type) - Vout;
					if((cell3 - cell1) > max_cell_difference)errore(3);
					break;
				case 32:
					// Cell 3 highest, cell 2 lowest
					cell_compensate = (cell3 * battery_type) - Vout;
					if((cell3 - cell2) > max_cell_difference)errore(3);
					break;
			}
		}
// battery			
// debug mag weg
		//uartprint(max_voltage);
		//uartprint(Vout);
		//uartprintln(Vin);
		//uartprint(call_cell1);
		//uartprint(cell1);
		//uartprint(call_cell2);
		//uartprint(cell2);
		//uartprint(cell3);
		uartprint(Vout);
		//uartprint(Vin);
		uartprintln(Iout);
		//uartprintln(cell_compensate);
// debug mag weg		
		
// main 
		for(uint16_t k=0; k < 64000; k++){for(uint16_t l=0; l < 10; l++){}}
    }

	//If errore detected loop forever with timers 0
	while(1){
		OCR0B = 0;
		OCR2B = 0;
	}
}// main


// timer voor led (naar led librari)
ISR(TIMER1_COMPA_vect){
	blue_state();
	red_state();
	OCR1A = 3891;
	
	cell_counter++;		//Counter for start measuring cells
}
// adc optimaliseren maar werkt (main)
ISR(ADC_vect){ 
	if(ADC >= 1020 && !(ADMUX == 0b01000001)){	//Safety emergancy stop if value becomes to high
		//stop = 1;		//Exit main loop
		errore(5);		//Log errore	
		OCR0B = 0;		//Counter off
		OCR2B = 0;		//Counter off
		uartprintln(ADMUX);
		}else{
		
		//Interrupt
		switch(ADMUX){				//Switch to next ADC
			case 0b01000010:								//Select PC2 - Vout//float temp = (float)call_Vout;
				;
				uint32_t temp_vout = ADC;
				temp_vout = ((temp_vout * call_Vout)/10);
				Vout = temp_vout;							//Calc Vout to mili volts
				if(Vout >= 13000){
					OCR2B=0;
					duty = 100;
				}
				ADMUX = (1<<REFS0) | (1<<MUX1) | (1<<MUX0); //goto PC3
				
				break;
			case 0b01000011:								//Select PC3 - Vin
			;
				uint32_t temp_Vin = ADC;
				temp_Vin = ((temp_Vin * call_Vin)/10);
				Vin = temp_Vin;							//Calc Vin to mili volts
				ADMUX = (1<<REFS0) | (1<<MUX2);				//goto PC4
				
				break;
			case 0b01000100:								//Select PC4 - Iout
				Iout = ADC * 7.8;
				ADMUX = (1<<REFS0) | (1<<MUX2) | (1<<MUX0);	//goto PC5
				break;
			case 0b01000101:								//Select PC5 - Iin
				Iin = ADC * 8.517/100;
				ADMUX = (1<<REFS0) | (1<<MUX1);				//goto PC2
				
				break;
			case 0b01000000:								//Select cell1
				ADMUX = (1<<REFS0) | (1<<MUX1);				//goto PC1
				uint32_t temp_cell2 = ADC ;
				temp_cell2 = ((temp_cell2 * call_cell2)/10) - cell1;
				cell2 = temp_cell2;
				if(cell2 < 0) cell2 = 0;
				cell3 = Vout - cell1 - cell2;
				break;
			case 0b01000001:								//Select cell1
				cell1 =  (ADC * call_cell1) / 10;
				
				if((max_voltage/cell_max_voltage) == 3){ // 3s
					ADMUX = (1<<REFS0);							//goto PC2
				}else{ // 2s
					ADMUX = (1<<REFS0) | (1<<MUX1);				//goto PC1
					cell2 = Vout - cell1;
				}
				
				//uartprintln(ADC);
				break;
			default:
				ADMUX |= (1<<REFS0) | (1<<MUX2) | (1<<MUX0);//If no case found goto PC2
				errore(4);									//Make error register 1
				break;
		}
		
		if(cell_counter >= 20){	//if statement for starting cell measurement
			ADMUX = 0b01000001;
			cell_counter = 0;
		}
		
		
		ADCSRA |= 1<<ADSC; //Start ADC
	}
}
// calibrate (onbepaalt)
void call_input(){
	//Calibration for input and output
	if(!(PINB&0x08)){
		sei();
		
		PORTB |= 0b00000010;
		call_Vout = 50;
		call_Vin = 0;
		
		for(uint16_t k=0; k < 64000; k++){for(uint16_t l=0; l < 20; l++){}} //wait
		
		int last_Vout;
		
		while(Vout < 10000 || Vin < 10000){
			
			if(Vout < 10000){
				call_Vout ++;
				last_Vout = Vout;
			}else{
			if((last_Vout-10000) > (Vout-10000)){
				call_Vout --;
			}
		}
			if(Vin < 10000)call_Vin ++;
			
			
			for(uint16_t k=0; k < 64000; k++){for(uint16_t l=0; l < 10; l++){}}
			
			
			uartprint(Vout);
			uartprint(call_Vout);
			
			uartprint(Vin);
			uartprintln(call_Vin);
			
			while (!( UCSR0A & (1<<UDRE0)));
			UDR0 = 10;
		}
		
		eeprom_write_byte(1, call_Vout);
		eeprom_write_byte(3, call_Vin);
		while(1){PORTB |= 0b00000001;}
	}
	
	//Calibration for cells
	if(!(PINB&0x20)){
		OCR1A = 3;
		sei();
		
		PORTB |= 0b00000100;
		call_cell1 = 30;
		call_cell2 = 30;
		
		for(uint16_t k=0; k < 64000; k++){for(uint16_t l=0; l < 20; l++){PORTB |= 0b00000100; cell_counter = 19;}} //wait
		
		int last_cell1;
		int last_cell2;
		
		while(cell1 < 4000 || (cell2+cell1) < 4000){
			
			if(cell1 < 4000){
				last_cell1 = cell1;
				call_cell1 ++;
			}else{
				if((last_cell1-4000) > (cell1-4000)){
					call_cell1 --;
				}
			}
			if((cell2+cell1) < 4000){
				call_cell2 ++;
				last_cell1 = cell1;
			}
			
			
			for(uint16_t k=0; k < 64000; k++){for(uint16_t l=0; l < 10; l++){PORTB |= 0b00000100; cell_counter = 255;}}
			
			
			uartprint(cell1);
			uartprint(call_cell1);
			
			uartprint((cell2+cell1));
			uartprintln(call_cell2);
			
			while (!( UCSR0A & (1<<UDRE0)));
			UDR0 = 10;
		}
		if((last_cell2-4000) > (cell2-4000)){
			call_cell2 --;
		}
		
		eeprom_write_byte(5, call_cell1);
		eeprom_write_byte(7, call_cell2);
		while(1){PORTB |= 0b00000001;}
	}
}
// werkt maar is niet optimaal (debugger niet nodig voor final product)
void uartprint(int input){
	unsigned char data[] = "Hello from ATmega328p  "; //Data buffer with random text
	itoa(input, data, 10); //Convert value to ascii
	uint8_t i = 0;
	while(data[i] != 0) /* print the String   */
	{
		while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/
		UDR0 = data[i];            /* Put data into buffer, sends the data */
		i++;                             /* increment counter           */
	}
	
	while (!( UCSR0A & (1<<UDRE0)));
	UDR0 = 9;
}
// werkt maar is niet optimaal (debugger niet nodig voor final product)
void uartprintln(int input){
	uartprint(input);					//Print actual data
	
	while (!( UCSR0A & (1<<UDRE0))); 
	UDR0 = 10;							//Print enter
}


/*
itoa(Vin, data, 10);
uint8_t i = 0;{
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = data[i];            
i++;                            
}
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = 9;

itoa(Iout, data, 10);
i = 0;
while(data[i] != 0){
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = data[i];            
i++;                            
}
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = 9;

itoa(power, data, 10);
i = 0;
while(data[i] != 0) {
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = data[i];           
i++;                             
}
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = 9;


int temp_duty;
temp_duty = duty*1;
itoa(temp_duty, data, 10);
i = 0;
while(data[i] != 0) {
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = data[i];            
i++;                             
}
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = 9;

itoa(Vout, data, 10);
i = 0;
while(data[i] != 0) {
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = data[i];            
i++;                          
while (!( UCSR0A & (1<<UDRE0))); 
UDR0 = 9;


*/