/*
 * ace_routine.h
 *
 * Created: 17.03.2022 15:24:35
 *  Author: artem
 */


#ifndef ACE_ROUTINE_H_
#define ACE_ROUTINE_H_


#include <stdint.h>

#include "../arduino/Arduino.h"


namespace ace_routine {

typedef void *__CoroutineLcPtr;
typedef void __CoroutineLc;
#define __COROUTINE_PTR_MAX_VALUE (void *)-1


namespace internal {
/**
 * Approximate division by 1000. More accurate algorithms exist (see for
 * example http://www.hackersdelight.org/divcMore.pdf) but I'm pretty sure
 * that this good enough since we don't guarantee accurate timing of the
 * COROUTINE_DELAY*() methods.
 */
typedef unsigned long TimeType;

inline TimeType udiv1000(TimeType n)
{
    // Use binomial expansion of 1/(1-x).
    // 1/1000 = 1/(1024 - 24)
    //        = (1/2^10) * (1 / (1 - 3/2^7))
    //        = (1/2^10) * (1 + 3/2^7 + 9/2^14 + 27/2^21 + ...)
    //        = (1/2^10 + 3/2^17 + 9/2^24 + 27/2^31 + ...)
    unsigned long x = (n >> 8);
    unsigned long y = (x >> 8);
    unsigned long z = (y >> 8);
    return (x >> 2) + 3 * (y >> 1) + 9 * z;
}

}  // namespace internal


#define __COROUTINE_IS_DELAY_EXPIRED(t, t0, t_delta) (((t) - (t0)) > static_cast<internal::TimeType>(t_delta) || (t0) > (t))

/**
 * Base class of all coroutines. The actual coroutine code is an implementation
 * of the virtual runCoroutine() method.
 */
class Coroutine
{
  public:
    enum class Status : uint8_t {
        SUSPENDED,
        /** Coroutine returned using the COROUTINE_YIELD() statement. */
        YIELDING,
        /** Coroutine returned using the COROUTINE_DELAY() statement. */
        DELAYING,
        /** Coroutine is currently running. True only within the coroutine itself. */
        RUNNING,
        /** Coroutine executed the COROUTINE_END() statement. */
        ENDING,
        /** Coroutine has ended and no longer in the scheduler queue. */
        TERMINATED,

        AWAITING_CONDITION
    };


    /** Human-readable name of the coroutine. */
    // const FCString &getName() const { return mName; }

    /**
     * The body of the coroutine. The COROUTINE macro creates a subclass of
     * this class and puts the body of the coroutine into this method.
     *
     * @return The return value is always ignored. This method is declared to
     * return an int to prevent the user from accidentally returning from this
     * method using an explicit 'return' statement instead of through one of
     * the macros (e.g. COROUTINE_YIELD(), COROUTINE_DELAY(), COROUTINE_AWAIT()
     * or COROUTINE_END()).
     */
    // virtual int runCoroutine() = 0;

    /**
     * Returns the current millisecond clock. By default it returns the global
     * millis() function from Arduino but can be overridden for testing.
     */
    inline internal::TimeType coroutineMillis() const { return ::millis(); }

    /**
     * Returns the current millisecond clock. By default it returns the global
     * micros() function from Arduino but can be overridden for testing.
     */
    inline internal::TimeType coroutineMicros() const { return ::micros(); }

    /**
     * Returns the current clock in unit of seconds, truncated to the lower
     * 16-bits. This is an approximation of (millis / 1000). It does not need
     * to be perfectly accurate because COROUTINE_DELAY_SECONS() is not
     * guaranteed to be precise.
     */
    inline internal::TimeType coroutineSeconds() const
    {
        internal::TimeType m = ::millis();
        return internal::udiv1000(m);
    }

    /**
     * Suspend the coroutine at the next scheduler iteration. If the coroutine
     * is already in the process of ending or is already terminated, then this
     * method does nothing. A coroutine cannot use this method to suspend
     * itself, it can only suspend some other coroutine. Currently, there is no
     * ability for a coroutine to suspend itself, that would require the
     * addition of a COROUTINE_SUSPEND() macro. Also, this method works only if
     * the CoroutineScheduler::loop() is used because the suspend functionality
     * is implemented by the CoroutineScheduler.
     */
    void suspend()
    {
        if (isDone()) { return; }
        mStatus = Status::SUSPENDED;
    }

    /**
     * Add a Suspended coroutine into the head of the scheduler linked list,
     * and change the state to Yielding. If the coroutine is in any other
     * state, this method does nothing. This method works only if the
     * CoroutineScheduler::loop() is used.
     */
    void resume()
    {
        if (mStatus == Status::SUSPENDED) {
            // We lost the original state of the coroutine when suspend() was called but
            // the coroutine will automatically go back into the original state when
            // Coroutine::runCoroutine() is called because COROUTINE_YIELD(),
            // COROUTINE_DELAY() and COROUTINE_AWAIT() are written to restore their
            // status.
            mStatus = Status::YIELDING;
        }
    }

    /** Check if delay time is over. */
    bool isDelayExpiredCommon()
    {
        switch (mDelayType) {
            case CoroutineDelayType::MILLIS: {
                internal::TimeType nowMillis = coroutineMillis();  // t
                return __COROUTINE_IS_DELAY_EXPIRED(nowMillis, this->mDelayStart, this->mDelayDuration);
            }
            case CoroutineDelayType::MICROS: {
                internal::TimeType nowMicros = coroutineMicros();
                return __COROUTINE_IS_DELAY_EXPIRED(nowMicros, this->mDelayStart, this->mDelayDuration);
            }
            case CoroutineDelayType::SECONDS: {
                internal::TimeType nowMicros = coroutineMicros();
                return __COROUTINE_IS_DELAY_EXPIRED(nowMicros, this->mDelayStart, this->mDelayDuration);
            }
            default:
                // This should never happen.
                return true;
        }
    }

    /** Check if delay millis time is over. */
    inline bool isDelayExpiredMillis() const
    {
        internal::TimeType nowMillis = coroutineMillis();  // t
        return __COROUTINE_IS_DELAY_EXPIRED(nowMillis, this->mDelayStart, this->mDelayDuration);
    }


    /** Check if delay micros time is over. */
    inline bool isDelayMicrosExpired() const
    {
        internal::TimeType nowMicros = coroutineMicros();
        return __COROUTINE_IS_DELAY_EXPIRED(nowMicros, this->mDelayStart, this->mDelayDuration);
    }

    /** Check if delay seconds time is over. */
    inline bool isDelaySecondsExpired() const
    {
        internal::TimeType nowSeconds = coroutineSeconds();
        return __COROUTINE_IS_DELAY_EXPIRED(nowSeconds, this->mDelayStart, this->mDelayDuration);
    }


    /** The coroutine was suspended with a call to suspend(). */
    bool isSuspended() const { return mStatus == Status::SUSPENDED; }

    /** The coroutine returned using COROUTINE_YIELD(). */
    bool isYielding() const { return mStatus == Status::YIELDING; }

    /** The coroutine returned using COROUTINE_DELAY(). */
    bool isDelaying() const { return mStatus == Status::DELAYING; }

    /** The coroutine is currently running. True only within the coroutine. */
    bool isRunning() const { return mStatus == Status::RUNNING; }

    /**
     * The coroutine returned using COROUTINE_END(). In most cases, isDone() is
     * recommended instead because it works when coroutines are executed
     * manually or through the CoroutineScheduler.
     */
    bool isEnding() const { return mStatus == Status::ENDING; }

    /**
     * The coroutine was terminated by the scheduler with a call to
     * setTerminated(). In most cases, isDone() should be used instead
     * because it works when coroutines are executed manually or through the
     * CoroutineScheudler.
     */
    bool isTerminated() const { return mStatus == Status::TERMINATED; }

    /**
     * The coroutine is either Ending or Terminated. This method is recommended
     * over isEnding() or isTerminated() because it works when the coroutine is
     * executed either manually or through the CoroutineScheduler.
     */
    bool isDone() const { return mStatus == Status::ENDING || mStatus == Status::TERMINATED; }

    /**
     * The execution status of the coroutine, corresponding to the
     * COROUTINE_YIELD(), COROUTINE_DELAY(), COROUTINE_AWAIT() and
     * COROUTINE_END() macros.
     *
     * The finite state diagram looks like this:
     *
     * @verbatim
     *          Suspended
     *          ^       ^
     *         /         \
     *        /           \
     *       v             \
     * Yielding          Delaying
     *      ^               ^
     *       \             /
     *        \           /
     *         \         /
     *          v       v
     *           Running
     *              |
     *              |
     *              v
     *           Ending
     *              |
     *              |
     *              v
     *         Terminated
     * @endverbatim
     */

    /**
     * Coroutine has been suspended using suspend() and the scheduler should
     * remove it from the queue upon the next iteration. We don't distinguish
     * whether the coroutine is still in the queue or not with this status. We
     * can add that later if we need to.
     */


    enum class CoroutineDelayType : uint8_t {
        /** Delay using units of millis. */
        MILLIS,
        /** Delay using units of micros. */
        MICROS,
        /** Delay using units of seconds. */
        SECONDS
    };


    /**
     * Constructor. All subclasses are expected to call either
     * setupCoroutine(const char*) or setupCoroutine(const
     * __FlashStringHelper*) before the CoroutineScheduler is used. The
     * COROUTINE() macro will automatically call setupCoroutine().
     *
     * See comment in setupCoroutine(const __FlashStringHelper*) for reason why
     * the setupCoroutine() function is used instead of chaining the name
     * through the constructor.
     */
    // Coroutine() = default;

    /** Destructor. */
    // ~Coroutine() = default;

    /** Return the status of the coroutine. Used by the CoroutineScheduler. */
    inline Status getStatus() const { return this->mStatus; }

    /** Set the kStatusRunning state. */
    inline void setRunning() { this->mStatus = Status::RUNNING; }

    /** Set the kStatusDelaying state. */
    inline void setYielding() { this->mStatus = Status::YIELDING; }

    inline void setAwaiting() { this->mStatus = Status::AWAITING_CONDITION; }

    /** Set the kStatusDelaying state. */
    inline void setDelaying() { this->mStatus = Status::DELAYING; }

    /** Set the kStatusEnding state. */
    inline void setEnding() { this->mStatus = Status::ENDING; }

    /**
     * Set status to indicate that the Coroutine has been removed from the
     * Scheduler queue. Should be used only by the CoroutineScheduler.
     */
    inline void setTerminated() { this->mStatus = Status::TERMINATED; }

    /**
     * Configure the delay timer for delayMillis.
     *
     * The maximum duration is set to (UINT16_MAX / 2) (i.e. 32767
     * milliseconds) if given a larger value. This makes the longest allowable
     * time between two successive calls to isDelayExpired() for a given
     * coroutine to be 32767 (UINT16_MAX - UINT16_MAX / 2 - 1) milliseconds,
     * which should be long enough for all practical use-cases. (The '- 1'
     * comes from an edge case where isDelayExpired() evaluates to be true in
     * the CoroutineScheduler::runCoroutine() but becomes to be false in the
     * COROUTINE_DELAY() macro inside Coroutine::runCoroutine()) because the
     * clock increments by 1 millisecond.)
     */
    void setDelayMillis(uint16_t delayMillis)
    {
        mDelayType     = CoroutineDelayType::MILLIS;
        mDelayStart    = coroutineMillis();
        mDelayDuration = (delayMillis >= UINT16_MAX / 2) ? UINT16_MAX / 2 : delayMillis;
    }

    /**
     * Configure the delay timer for delayMicros. Similar to seDelayMillis(),
     * the maximum delay is 32767 micros.
     */
    void setDelayMicros(uint16_t delayMicros)
    {
        mDelayType     = CoroutineDelayType::MICROS;
        mDelayStart    = coroutineMicros();
        mDelayDuration = (delayMicros >= UINT16_MAX / 2) ? UINT16_MAX / 2 : delayMicros;
    }

    /**
     * Configure the delay timer for delaySeconds. Similar to seDelayMillis(),
     * the maximum delay is 32767 seconds.
     */
    void setDelaySeconds(uint16_t delaySeconds)
    {
        mDelayType     = CoroutineDelayType::SECONDS;
        mDelayStart    = coroutineSeconds();
        mDelayDuration = (delaySeconds >= UINT16_MAX / 2) ? UINT16_MAX / 2 : delaySeconds;
    }


    /* routine local context to restore */
  public:
    __CoroutineLcPtr lc;
    Status mStatus = Status::YIELDING;

    typedef bool (*AvaitPFun)();

    void *callContext;
    AvaitPFun awaitPfun;

    virtual uint8_t runCoroutine() = 0;

  private:
    CoroutineDelayType mDelayType;
    internal::TimeType mDelayStart;  // millis or micros
    uint16_t mDelayDuration;  // millis or micros
};

}  // namespace ace_routine


#endif /* ACE_ROUTINE_H_ */
