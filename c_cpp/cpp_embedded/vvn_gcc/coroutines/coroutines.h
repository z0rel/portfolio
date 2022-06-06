/*
 * coroutines.h
 *
 * Created: 10.03.2022 6:46:12
 *  Author: artem
 */ 


#ifndef COROUTINES_H_
#define COROUTINES_H_

#include "../coroutine/Coroutine.h"
#include "../src/lcd/lcd.h"

// #include "adc/adc_coroutine.h"


EXTERN_COROUTINE(task_display, void);
EXTERN_COROUTINE(task_modbus, void);
EXTERN_COROUTINE(task_adc, void);


#endif /* COROUTINES_H_ */