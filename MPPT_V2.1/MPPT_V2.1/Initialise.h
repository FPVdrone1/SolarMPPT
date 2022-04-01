/*
 * Initialise.h
 *
 * Created: 30-12-2019 19:20:58
 *  Author: Mirko
 */ 

#ifndef INITIALISE_H_
#define INITIALISE_H_

void initialise_ADC();

void initialise_PWM();

void initialise_led();

void initialise_input();

int check_bat(int* Vout, int* cell1, int* cell2);

void initialise_uart();

void tx_uart (uint8_t data);

void initialise_ADC_multiplier(uint16_t* Vout, uint16_t* Iout, uint16_t* Vin, uint16_t* Iin, uint16_t* cell2);

#endif 