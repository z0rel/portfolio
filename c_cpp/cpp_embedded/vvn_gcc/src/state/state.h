/*
 * state.h
 *
 * Created: 16.03.2022 11:09:23
 *  Author: artem
 */ 


#ifndef STATE_H_
#define STATE_H_

#include "../../coroutine/Coroutine.h"


namespace state {

void set_initial_state(); 

EXTERN_COROUTINE(state_next, void);

}


#endif /* STATE_H_ */