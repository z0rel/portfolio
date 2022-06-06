/*
 * init_adc.h
 *
 * Created: 02.03.2022 0:01:24
 *  Author: artem
 */ 


#ifndef INIT_ADC_H_
#define INIT_ADC_H_

void init_adc();


#define ADC_INTERNAL_REFERENCE (_BV(REFS0) | _BV(REFS1))

// Выравнивание результата по правому краю (ADLAR = 0) ADCH: [0]=8,[1]=9  ADCL: [0]=0,...,[7]=7
#define ADC_LITTLE_ENDIAN 0
// Выравнивание результата по левому краю (ADLAR = 1) ADCH: [0]=2,...,[7]=9  ADCL: [6]=0, [7]=1
#define ADC_BIG_ENDIAN _BV(ADLAR)

// ADC7
#define ADC_MUX_SELECT_VISCOSITY (ADC_INTERNAL_REFERENCE | ADC_LITTLE_ENDIAN | _BV(MUX2) | _BV(MUX1) | _BV(MUX0))
// ADC5
#define ADC_MUX_SELECT_DENCITY (ADC_INTERNAL_REFERENCE | ADC_LITTLE_ENDIAN | _BV(MUX2) | _BV(MUX0))
// ADC6
#define ADC_MUX_SELECT_TEMPERATURE (ADC_INTERNAL_REFERENCE | ADC_LITTLE_ENDIAN | _BV(MUX2) | _BV(MUX1))



#endif /* INIT_ADC_H_ */