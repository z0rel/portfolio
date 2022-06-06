/*
 * Linear.h
 *
 * Created: 28.03.2022 9:49:35
 *  Author: artem
 */


#ifndef LINEAR_H_
#define LINEAR_H_

#include "ConstrainedSpline.h"

namespace interpolation {

class Linear {
    double x0_y            = 0.0;
    double x1_y            = 0.0;
    double x0_x            = 0.0;
    double x1_x_minus_x0_x = 0.0;

  public:
    Linear(const InterpolationPoint &x0, const InterpolationPoint &x1)
    {
        this->x0_y            = x0.y;
        this->x1_y            = x1.y;
        this->x0_x            = x0.x;
        this->x1_x_minus_x0_x = x1.x - x0.x;
    }
    double calculate(double pointX) const
    {
        double t = (pointX - this->x0_x) / this->x1_x_minus_x0_x;
        return this->x0_y * (1.0 - t) + this->x1_y * t;
    }
};


#define STATIC_LINEAR_INTERPOLATE(x0, y0, x1, y1, tmp_var, result_var, point_x) \
    (tmp_var)    = ((point_x) - (x0)) / ((x1) - (x0));                          \
    (result_var) = (y0) * (1.0 - (tmp_var)) + (y1) * (tmp_var);


class Linear4_20ma {
    // double x0_y = 0.0;
    // double x1_y = 0.0;
    double y0 = 0.0;
    double y1 = 0.0;

  public:
    inline Linear4_20ma() {}
    inline Linear4_20ma(const Linear4_20ma &oth) : y0(oth.y0), y1(oth.y1) {}
    inline Linear4_20ma(double y0, double y1)
    {
        // this->x0_y = x0.y; // 4 ma
        // this->x1_y = x1.y; // 20 ma
        this->y0 = y0;
        this->y1 = y1;
    }
    inline double calculate(double pointX) const
    {
        double t = (pointX - 4.0) / (20.0 - 4.0);
        return this->y0 * (1.0 - t) + this->y1 * t;
    }

    inline double get_y0() const { return this->y0; }
    inline double get_y1() const { return this->y1; }
};


} // namespace interpolation


#endif /* LINEAR_H_ */
