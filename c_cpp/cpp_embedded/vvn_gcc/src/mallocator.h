/*
 * mallocator.h
 *
 * Created: 12.03.2022 14:35:47
 *  Author: artem
 */


#ifndef MALLOCATOR_H_
#define MALLOCATOR_H_


#include "../freertos/portable.h" 

#define MEMORY_SIZE_MAX  (0x90FF - 0x1100 - (0x7FFF - 0x7FF8 + 1))

class std_bad_alloc {};

template <class T> 
struct Mallocator 
{
    typedef T value_type;

    Mallocator() = default;
    template <class U> constexpr Mallocator(const Mallocator<U> &) noexcept {}

    T *allocate(uint16_t n)
    {
        if (n > MEMORY_SIZE_MAX / sizeof(T)) {
            throw std_bad_alloc();
		}

        if (auto p = static_cast<T *>(pvPortMalloc(n * sizeof(T)))) {
            return p;
        }

        throw std_bad_alloc();
    }

    void deallocate(T *p, uint16_t n) noexcept
    {
        vPortFree(p);
    }
};


template <class T, class U> bool operator==(const Mallocator<T> &, const Mallocator<U> &) { return true; }
template <class T, class U> bool operator!=(const Mallocator<T> &, const Mallocator<U> &) { return false; }


#endif /* MALLOCATOR_H_ */
