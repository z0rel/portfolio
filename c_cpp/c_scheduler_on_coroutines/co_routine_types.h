#ifndef CO_ROUTINE_TYPES_H
#define CO_ROUTINE_TYPES_H

#include "scheduler/coro.h"


/**
 * @struct co_routine
 *
 * @brief represents co-routine stack frame
 *
 */
struct co_routine
{
    /** Контекст указателя стека исполняемой функции корутины */
    struct coro_context sp;
};

#define CO_ROUTINE_INITIALIZER { CORO_CONTEXT_INITIALIZER, NULL }


#endif // CO_ROUTINE_TYPES_H
