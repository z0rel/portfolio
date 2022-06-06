/*
 * dac.h
 *
 * Created: 11.03.2022 5:48:48
 *  Author: artem
 */ 


#ifndef DAC_H_
#define DAC_H_

#include <stdint.h>


void init_dac();

/// Запись 12 битов в ЦАП
void dac_write(uint16_t word12);


#endif /* DAC_H_ */