/*
 * Algoritme.h
 *
 * Created: 4-2-2020 11:55:34
 *  Author: Mirko
 */ 


#ifndef ALGORITME_H_
#define ALGORITME_H_

int MPPT2(int voltage, int current, int induty);

int MPPT(uint16_t Vin, uint16_t Iin);

int Battery_charge(int Vout, int Iout, int Vin, int Iin, int Voc, int duty);

void Voltage(uint16_t milivolt, uint16_t Vout);

void current(uint16_t miliamp, uint16_t Iout);

#endif /* ALGORITME_H_ */