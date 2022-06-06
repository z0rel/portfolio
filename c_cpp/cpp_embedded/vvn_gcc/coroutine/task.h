/*
 * task.h
 *
 * Created: 17.03.2022 21:36:31
 *  Author: artem
 */


#ifndef TASK_H_
#define TASK_H_

#include "stack_array.h"

namespace multitasking {

extern VStackArray *current_task;

template <uint16_t depth> class Task {
  public:
    StackArray<depth> stack;
    ace_routine::Coroutine *ctx = nullptr;

    Task(ace_routine::Coroutine *coro, void *ctx = nullptr)
    {
        coro->callContext = ctx;
        stack.push(coro);
    }

    inline void check_top_and_pop_if_need()
    {
        current_task = &this->stack;
        if (this->ctx != nullptr) {
            uint8_t val = this->ctx->runCoroutine();
            if (val) {
                stack.pop();
            }
            this->ctx = stack.top();
        }
    }

    inline void run()
    {
        this->ctx    = stack.top();
		// TODO: переместить проверку исчерпания времени в эту точку

        uint8_t level;
        uint8_t next_step_level = this->stack.count();
        do {
            level = next_step_level;

            switch (this->ctx->mStatus) {
            case ace_routine::Coroutine::Status::YIELDING:
                check_top_and_pop_if_need();
                break;
            case ace_routine::Coroutine::Status::DELAYING:
                if (this->ctx->isDelayExpiredCommon()) {
                    check_top_and_pop_if_need();
                }
                break;
            case ace_routine::Coroutine::Status::AWAITING_CONDITION:
                if ((*this->ctx->awaitPfun)()) {
                    check_top_and_pop_if_need();
                }
                break;
            default:
                check_top_and_pop_if_need();
                break;
            }

            next_step_level = this->stack.count();
        } while (level != next_step_level);
    }
};

} // namespace multitasking

#endif /* TASK_H_ */
