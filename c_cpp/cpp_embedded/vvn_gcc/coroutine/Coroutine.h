#ifndef ACE_ROUTINE_COROUTINE_H
#define ACE_ROUTINE_COROUTINE_H

#include "ace_routine.h"
#include "task.h"


// l_yield - метка в конце корутины
// l_yield:
//     CO_RETURN();
// l_error:
//     abort();
//     CO_END;

#define __LC_RESUME()                                                                                     \
    do {                                                                                                  \
        ace_routine::__CoroutineLcPtr llc = this->lc;                                                     \
        this->lc                          = __COROUTINE_PTR_MAX_VALUE; /* state routine as not yielded */ \
        if (llc != nullptr) {                                                                             \
            goto *(llc);                                                                                  \
        }                                                                                                 \
    } while (0)


#define ___CONCAT(x, y) x##y
#define __LC_ENTRY(line) ___CONCAT(__LC_, line)
#define __LC_FORCE_ENTRY(line) ___CONCAT(__FORCE_, line)
#define __LC_ERR_ENTRY(line) ___CONCAT(__ERR_, line)
#define __LC_SET(lc, line) (lc) = &&__LC_ENTRY(line);
#define __LC_FORCE_SET(lc, line) (lc) = &&__LC_FORCE_ENTRY(line);
#define __LC_ERR_SET(lc, line) (lc) = &&__LC_ERR_ENTRY(line);
#define __CO_IS_DONE(routine) ((routine)->lc == (void *)-1)


/**
 * @file Coroutine.h
 *
 * All coroutines are instances of the Coroutine base class. The COROUTINE()
 * macro creates these instances, and registers them to automatically run when
 * CoroutineScheduler::loop() is called.
 *
 * Various macros use macro overloading to implement a 1-argument and
 * a 2-argument version. See https://stackoverflow.com/questions/11761703 to
 * description of how that works.
 *
 * The computed goto is a GCC extension:
 * https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
 * The __noinline__ and __noclone__ attributes make sure that label pointers are
 * always the same. I'm not 100% sure they are needed here, but they don't seem
 * to hurt.
 */

/**
 * Create a Coroutine instance named 'name'. Two forms are supported
 *
 *   - COROUTINE(name) {...}
 *   - COROUTINE(className, name) {...}
 *
 * The 1-argument form uses the Coroutine class as the base class of the
 * coroutine. The 2-argument form uses the user-provided className which must be
 * a subclass of Coroutine.
 *
 * The code in {} following this macro becomes the body of the
 * Coroutine::runCoroutine() method.
 */
#define COROUTINE(...) COROUTINE1(__VA_ARGS__)


#define GET_COROUTINE(_1, _2, NAME, ...) NAME


#define COROUTINE1(name, CTX_TYPE)                           \
    class Coroutine_##name : public ace_routine::Coroutine { \
      public:                                                \
        typedef CTX_TYPE T_CONTEXT;                          \
        virtual uint8_t runCoroutine();                      \
    } name;                                                  \
    uint8_t Coroutine_##name ::runCoroutine()


#define COROUTINE1_ARG(name, ...)                            \
    class Coroutine_##name : public ace_routine::Coroutine { \
      public:                                                \
        virtual uint8_t runCoroutine(__VA_ARGS__);           \
    } name;                                                  \
    uint8_t Coroutine_##name ::runCoroutine(__VA_ARGS__)


/**
 * Create an extern reference to a coroutine that is defined in another .cpp
 * file. The extern reference is needed before it can be used. Two forms are
 * supported:
 *
 *    - EXTERN_COROUTINE(name);
 *    - EXTERN_COROUTINE(className, name);
 */
#define EXTERN_COROUTINE(...) EXTERN_COROUTINE1(__VA_ARGS__)

#define GET_EXTERN_COROUTINE(_1, _2, NAME, ...) NAME

#define EXTERN_COROUTINE1(name, CTX_TYPE)                    \
    class Coroutine_##name : public ace_routine::Coroutine { \
      public:                                                \
        typedef CTX_TYPE T_CONTEXT;                          \
        Coroutine_##name();                                  \
        virtual uint8_t runCoroutine();                      \
    };                                                       \
    extern Coroutine_##name name


#define EXTERN_COROUTINE1_ARG(name, ...)                     \
    class Coroutine_##name : public ace_routine::Coroutine { \
      public:                                                \
        Coroutine_##name();                                  \
        virtual uint8_t runCoroutine(__VA_ARGS__);           \
    };                                                       \
    extern Coroutine_##name name

#define EXTERN_COROUTINE2_ARG(className, name, ...) \
    struct className##_##name : className {         \
        className##_##name();                       \
        virtual uint8_t runCoroutine(__VA_ARGS__);  \
    };                                              \
    extern className##_##name name

/** Mark the beginning of a coroutine. */
/*
#define COROUTINE_BEGIN() \
    void *p = getJump();  \
    if (p != nullptr) {   \
        goto *p;          \
    }


#define COROUTINE__SET_JUMP(val) \
    __label__ jumpLabel;         \
    setJump(&&jumpLabel);        \
    return (val);                \
    jumpLabel:;
*/


#define COROUTINE_BEGIN() \
    do {                  \
        __LC_RESUME();    \
    } while (0)


#define COROUTINE__SET_JUMP(val)  \
    __LC_SET(this->lc, __LINE__); \
    return (val);                 \
    __LC_ENTRY(__LINE__) :;


/**
 * Mark the beginning of a coroutine loop. Can be used instead of
 * COROUTINE_BEGIN() at the beginning of a Coroutine.
 */
#define COROUTINE_LOOP() \
    COROUTINE_BEGIN();   \
    while (true)


#define COROUTINE_YIELD_INTERNAL() \
    do {                           \
        COROUTINE__SET_JUMP(0)     \
    } while (false)

/** Yield execution to another coroutine. */
#define COROUTINE_YIELD()           \
    do {                            \
        this->setYielding();        \
        COROUTINE_YIELD_INTERNAL(); \
        this->setRunning();         \
    } while (false)


#define COROUTINE_YIELD_INTERNAL_ARG(val) \
    do {                                  \
        COROUTINE__SET_JUMP(val)          \
    } while (false)

/** Yield execution to another coroutine. */
#define COROUTINE_YIELD_ARG(val)           \
    do {                                   \
        this->setYielding();               \
        COROUTINE_YIELD_INTERNAL_ARG(val); \
        this->setRunning();                \
    } while (false)


/** Yield execution to another coroutine. */
#define COROUTINE_END_CALL()               \
    do {                                   \
        this->setYielding();               \
        COROUTINE_YIELD_INTERNAL_ARG(1);   \
        this->setRunning();                \
    } while (false)


/**
 * Yield until condition is true, then execution continues. This is
 * functionally equivalent to:
 *
 * @code
 *    while (!condition) COROUTINE_YIELD();
 * @endcode
 *
 * but potentially slightly more efficient.
 */
#define COROUTINE_AWAIT(condition)      \
    do {                                \
        this->setYielding();            \
        do {                            \
            COROUTINE_YIELD_INTERNAL(); \
        } while (!(condition));         \
        this->setRunning();             \
    } while (false)


#define COROUTINE_AWAIT_CB(cb)      \
    do {                            \
        this->awaitPfun = (cb);     \
        this->setAwaiting();        \
        COROUTINE_YIELD_INTERNAL(); \
        this->setRunning();         \
    } while (false)


#define COROUTINE_CALL(coro_instance, ctx)                      \
    do {                                                        \
        (coro_instance).callContext = static_cast<void *>(ctx); \
        multitasking::current_task->push(&coro_instance);       \
        (coro_instance).setYielding();                          \
        COROUTINE_YIELD_INTERNAL();                             \
        this->setRunning();                                     \
    } while (false)


/**
 * Yield for delayMillis. A delayMillis of 0 is functionally equivalent to
 * COROUTINE_YIELD(). To save memory, the delayMillis is stored as a uint16_t
 * but the actual maximum is limited to 32767 millliseconds. See
 * setDelayMillis() for the reason for this limitation.
 *
 * If you need to wait for longer than that, use a for-loop to call
 * COROUTINE_DELAY() as many times as necessary or use
 * COROUTINE_DELAY_SECONDS().
 *
 * This could have been implemented using COROUTINE_AWAIT() but this macro
 * matches the global delay(millis) function already provided by the Arduino
 * API. Also having a separate kStatusDelaying state allows the
 * CoroutineScheduler to be slightly more efficient by avoiding the call to
 * Coroutine::runCoroutine() if the delay has not expired.
 */
#define COROUTINE_DELAY(delayMillis)             \
    do {                                         \
        this->setDelayMillis(delayMillis);       \
        this->setDelaying();                     \
        do {                                     \
            COROUTINE_YIELD_INTERNAL();          \
        } while (!this->isDelayExpiredMillis()); \
        this->setRunning();                      \
    } while (false)


/** Yield for delayMicros. Similiar to COROUTINE_DELAY(delayMillis). */
#define COROUTINE_DELAY_MICROS(delayMicros)      \
    do {                                         \
        this->setDelayMicros(delayMicros);       \
        this->setDelaying();                     \
        do {                                     \
            COROUTINE_YIELD_INTERNAL();          \
        } while (!this->isDelayMicrosExpired()); \
        this->setRunning();                      \
    } while (false)


/** Yield for delaySeconds. Similar to COROUTINE_DELAY(delayMillis). */
#define COROUTINE_DELAY_SECONDS(delaySeconds)     \
    do {                                          \
        this->setDelaySeconds(delaySeconds);      \
        this->setDelaying();                      \
        do {                                      \
            COROUTINE_YIELD_INTERNAL();           \
        } while (!this->isDelaySecondsExpired()); \
        this->setRunning();                       \
    } while (false)


/// Задержка в миллисекундах в корневой корутине
#define COROUTINE_DELAY_MAIN(delayMillis)  \
    do {                                   \
        this->setDelayMillis(delayMillis); \
        this->setDelaying();               \
        COROUTINE_YIELD_INTERNAL();        \
        this->setRunning();                \
    } while (false)


/// Задержка в микросекундах в корневой корутине
/** Yield for delayMicros. Similiar to COROUTINE_DELAY(delayMillis). */
#define COROUTINE_DELAY_MICROS_MAIN(delayMicros) \
    do {                                         \
        this->setDelayMicros(delayMicros);       \
        this->setDelaying();                     \
        COROUTINE_YIELD_INTERNAL();              \
        this->setRunning();                      \
    } while (false)


/** Yield for delaySeconds. Similar to COROUTINE_DELAY(delayMillis). */
#define COROUTINE_DELAY_SECONDS_MAIN(delaySeconds) \
    do {                                           \
        this->setDelaySeconds(delaySeconds);       \
        this->setDelaying();                       \
        COROUTINE_YIELD_INTERNAL();                \
        this->setRunning();                        \
    } while (false)


/**
 * Mark the end of a coroutine. Subsequent calls to Coroutine::runCoroutine()
 * will do nothing.
 */
#define COROUTINE_END()        \
    do {                       \
        this->setEnding();     \
        COROUTINE__SET_JUMP(0) \
    } while (false)


#define COROUTINE_END_ARG(arg)   \
    do {                         \
        this->setEnding();       \
        COROUTINE__SET_JUMP(arg) \
    } while (false)


#endif
