/*
 * stack_array.h
 *
 * Created: 17.03.2022 21:21:37
 *  Author: artem
 */


#ifndef STACK_ARRAY_H_
#define STACK_ARRAY_H_


#include "../arduino/Arduino.h"
#include "ace_routine.h"
#include "stdint.h"

namespace multitasking {

typedef ace_routine::Coroutine *MultitaskingStackItem;

class VStackArray {
  public:
    // push an item to the stack.
    virtual void push(MultitaskingStackItem i) = 0;

    // pop an item from the stack.
    virtual MultitaskingStackItem pop() = 0;

    // unshift an item to the stack.
    virtual void unshift(MultitaskingStackItem i) = 0;

    // get an item from the stack.
    virtual MultitaskingStackItem top() const = 0;

    // check if the stack is empty.
    bool isEmpty() const { return _top_index == 0; }

    // get the number of items in the stack.
    int count() const { return _top_index; }

    // check if the stack is full.
    virtual bool isFull() const = 0;

  protected:
    uint8_t _top_index = 0; // the top index of the stack.
};


namespace internal_multitasking {
class PrintMessage {
  public:
    static void stack_is_full(uint8_t size, uint8_t stack_top);
    static void cant_pop_item_from_stack(uint8_t size, uint8_t stack_top);
    static void cant_peek_item_from_stack(uint8_t size, uint8_t stack_top);
};

} // namespace internal_multitasking


template <uint8_t size> class StackArray : public VStackArray {
    // TODO: print from PROGMEM
    enum class SizeValue : uint8_t { value = size };

  public:
    // init the stack (constructor).
    // push an item to the stack.
    void push(MultitaskingStackItem i)
    {
        // check if the stack is full.
        if (isFull()) {
            internal_multitasking::PrintMessage::stack_is_full(static_cast<uint8_t>(SizeValue::value), _top_index);
            return;
        }

        // store the item to the array.
        contents[_top_index++] = i;
    }

    // pop an item from the stack.
    MultitaskingStackItem pop()
    {
        // check if the stack is empty.
        if (isEmpty()) {
            internal_multitasking::PrintMessage::cant_pop_item_from_stack(static_cast<uint8_t>(SizeValue::value), _top_index);
            return nullptr;
        }

        // return the top item from the array.
        return contents[--_top_index];
    }

    // unshift an item to the stack.
    void unshift(MultitaskingStackItem i)
    {
        // check if the stack is full.
        if (isFull()) {
            // double size of array.
            internal_multitasking::PrintMessage::stack_is_full(static_cast<uint8_t>(SizeValue::value), _top_index);
            return;
        }

        // store the item to the array.
        for (int c = size - 1; c > 0; c--) {
            contents[c] = contents[c - 1];
        }
        contents[0] = i;
        _top_index++;
    }

    // get an item from the stack.
    MultitaskingStackItem top() const
    {
        // check if the stack is empty.
        if (isEmpty()) {
            internal_multitasking::PrintMessage::cant_peek_item_from_stack(static_cast<uint8_t>(SizeValue::value), _top_index);
            return nullptr;
        }

        // get the top item from the array.
        return contents[_top_index - 1];
    }

    // check if the stack is full.
    bool isFull() const { return _top_index == static_cast<uint16_t>(SizeValue::value); }

  private:
    MultitaskingStackItem contents[static_cast<uint16_t>(SizeValue::value)] = {nullptr}; // the array of the stack.
};


} // namespace multitasking

#endif /* STACK_ARRAY_H_ */
