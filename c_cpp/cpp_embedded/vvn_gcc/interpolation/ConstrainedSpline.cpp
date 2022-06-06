/*
 * ConstrainedSpline.cpp
 *
 * Created: 29.03.2022 6:24:08
 *  Author: artem
 */


#include "ConstrainedSpline.h"


namespace interpolation {


int InterpolationPoint::compare(const void *l, const void *r)
{
    if (reinterpret_cast<const InterpolationPoint *>(l)->x == reinterpret_cast<const InterpolationPoint *>(r)->x) {
        return 0;
    }
    else if (reinterpret_cast<const InterpolationPoint *>(l)->x < reinterpret_cast<const InterpolationPoint *>(r)->x) {
        return -1;
    }
    else {
        return 1;
    }
}


} // namespace interpolation
