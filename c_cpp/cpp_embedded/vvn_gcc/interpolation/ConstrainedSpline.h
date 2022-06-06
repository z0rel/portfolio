/*
 * ConstrainedSpline.h
 *
 * Created: 28.03.2022 7:48:44
 *  Author: artem
 */


#ifndef CONSTRAINEDSPLINE_H_
#define CONSTRAINEDSPLINE_H_

#include <stdint.h>
#include <stdlib.h>


namespace interpolation {


struct InterpolationPoint {
    double x;
    double y;

    static int compare(const void *l, const void *r);
};


namespace interp_internal {


class CatmullCtx {
  public:
    InterpolationPoint *points;
    uint8_t n;
};


inline double
getFirstDerivate_from_internal_i_range(CatmullCtx &ctx, uint8_t i)
{
    double dx_dy_dx_right;
    double dx_dy_dx_left;

    {
        double dx_right = ctx.points[i + 1].x - ctx.points[i].x;
        double dy_right = ctx.points[i + 1].y - ctx.points[i].y;

        dx_dy_dx_right = dx_right / dy_right;
    }

    {
        double dx_left = ctx.points[i].x - ctx.points[i - 1].x;
        double dy_left = ctx.points[i].y - ctx.points[i - 1].y;

        dx_dy_dx_left = dx_left / dy_left;
    }

    if (dx_dy_dx_right * dx_dy_dx_left < 0) {
        return 0.0;
    }
    else {
        return 2.0 / (dx_dy_dx_right + dx_dy_dx_left);
    }
}


inline double getFirstDerivate(CatmullCtx &ctx, uint8_t i)
{
    double fd1_x;

    if (i == 0) {
        {
            double dy = ctx.points[1].y - ctx.points[0].y;
            double dx = ctx.points[1].x - ctx.points[0].x;
            fd1_x     = (3.0 / 2.0) * dy / dx;
        }

        fd1_x -= getFirstDerivate_from_internal_i_range(ctx, 1) / 2.0;
    }
    else if (i == ctx.n) {
        {
            double dy = ctx.points[ctx.n].y - ctx.points[ctx.n - 1].y;
            double dx = ctx.points[ctx.n].x - ctx.points[ctx.n - 1].x;
            fd1_x     = (3.0 / 2.0) * dy / dx;
        }
        fd1_x -= getFirstDerivate_from_internal_i_range(ctx, ctx.n - 1) / 2.0;
    }
    else {
        fd1_x = getFirstDerivate_from_internal_i_range(ctx, i);
    }

    return fd1_x;
}


inline double getLeftSecondDerivate(CatmullCtx &ctx, uint8_t i)
{
    double fd2l_x;

    double dx_left = ctx.points[i].x - ctx.points[i - 1].x;

    {
        double fdi_x   = getFirstDerivate(ctx, i);
        double fdi_xl1 = getFirstDerivate(ctx, i - 1);
        fd2l_x         = -2.0 * (fdi_x + 2.0 * fdi_xl1) / dx_left;
    }

    {
        double dy_left = ctx.points[i].y - ctx.points[i - 1].y;
        fd2l_x += 6.0 * dy_left / dx_left / dx_left;
    }

    return fd2l_x;
}


inline double getRightSecondDerivate(CatmullCtx &ctx, uint8_t i)
{
    double fd2r_x;
    double dx_left = ctx.points[i].x - ctx.points[i - 1].x;

    {
        double fdi_x   = getFirstDerivate(ctx, i);
        double fdi_xl1 = getFirstDerivate(ctx, i - 1);
        fd2r_x         = 2.0 * (2.0 * fdi_x + fdi_xl1) / dx_left;
    }

    double dy_left = ctx.points[i].y - ctx.points[i - 1].y;
    fd2r_x -= 6.0 * dy_left / dx_left / dx_left;

    return fd2r_x;
}


}; // namespace interp_internal


template <uint8_t SPLINE_ARRAY_SIZE> class CalculatedSpline {

    enum class SizeValue : uint8_t { value = SPLINE_ARRAY_SIZE };

    double a[static_cast<uint8_t>(SizeValue::value) - 1];
    double b[static_cast<uint8_t>(SizeValue::value) - 1];
    double c[static_cast<uint8_t>(SizeValue::value) - 1];
    double d[static_cast<uint8_t>(SizeValue::value) - 1];
    double x[static_cast<uint8_t>(SizeValue::value)];
    double y[static_cast<uint8_t>(SizeValue::value)];
    uint8_t dup_offset = 0;

  public:
    CalculatedSpline() {}
	CalculatedSpline& operator=(const CalculatedSpline &oth) {
	   for (uint8_t i = 0; i < static_cast<uint8_t>(SizeValue::value) - 1; ++i) {
	       this->a[i] = oth.a[i];
	       this->b[i] = oth.b[i];
	       this->c[i] = oth.c[i];
	       this->d[i] = oth.d[i];
	       this->x[i] = oth.x[i];
	       this->y[i] = oth.y[i];
	       this->dup_offset = oth.dup_offset;
	   }
	   return *this;
	}
	double get_x(uint8_t i) const { return this->x[i]; }
	double get_y(uint8_t i) const { return this->y[i]; }

    CalculatedSpline(InterpolationPoint points[static_cast<uint8_t>(SizeValue::value)])
    {
        // сортировка точек
        qsort(points, static_cast<uint8_t>(SizeValue::value), sizeof(InterpolationPoint), InterpolationPoint::compare);

        // Удаление дубликатов по X

        double x_start   = points[0].x;
        uint8_t i_assign = 0;

        for (uint8_t j = 1; j < SPLINE_ARRAY_SIZE; ++j) {
            if (points[j].x == x_start) {
                ++this->dup_offset;
            }
            else {
                ++i_assign;
                points[i_assign].x = points[j].x;
                points[i_assign].y = points[j].y;
                x_start            = points[j].x;
            }
        }


        for (uint8_t j = 0; j < static_cast<uint8_t>(SizeValue::value); ++j) {
            this->x[j] = points[j].x;
            this->y[j] = points[j].y;
        }

        uint8_t max_i = static_cast<uint8_t>(SizeValue::value) - 1 - this->dup_offset;

        if (max_i == 1) {
            // сплайн вырождается в линейную интерполяцию
            this->a[0] = points[0].y;
            this->b[0] = points[1].y;
            this->c[0] = points[0].x;
            this->d[0] = points[1].x - points[0].x;
        }
        else {
            interp_internal::CatmullCtx ctx{points, max_i};
            for (uint8_t i = 0; i < max_i; ++i) {
                double x0 = points[i + 1].x;
                double x1 = points[i].x;
                double y0 = points[i + 1].y;
                double y1 = points[i].y;

                double fd2i_xl1 = getLeftSecondDerivate(ctx, i + 1);
                double fd2i_x   = getRightSecondDerivate(ctx, i + 1);

                double d = (fd2i_x - fd2i_xl1) / (6.0 * (x0 - x1));
                double c = (x0 * fd2i_xl1 - x1 * fd2i_x) / 2.0 / (x0 - x1);
                double b = (y0 - y1 - c * (x0 * x0 - x1 * x1) - d * (x0 * x0 * x0 - x1 * x1 * x1)) / (x0 - x1);

                this->a[i] = y1 - b * x1 - c * x1 * x1 - d * x1 * x1 * x1;
                this->b[i] = b;
                this->c[i] = c;
                this->d[i] = d;
            }
        }
    }

    double calculate(double pointX) const
    {
        uint8_t max_i = static_cast<uint8_t>(SizeValue::value) - 1 - this->dup_offset;
        uint8_t i;

        if (max_i == 1) {
            // сплайн вырождается в линейную интерполяцию
            double t = (pointX - this->c[0]) / this->d[0];
            return this->a[0] * (1 - t) + this->b[0] * t;
        }

        i = 0;
        if (pointX <= this->x[0]) {
            i = 0;
        }
        else if (pointX >= this->x[max_i]) {
            i = max_i - 1;
        }
        else {
            while (pointX >= this->x[i + 1]) {
                ++i;
            }
        }

        // Если текущая точка равна максимальной, либо если точка интерполяции всего одна
        if (pointX == this->x[max_i] || !max_i) {
            return this->y[max_i];
        }

        return this->a[i] + pointX * (this->b[i] + pointX * (this->c[i] + pointX * this->d[i]));
    }
};


} // namespace interpolation

#endif /* CONSTRAINEDSPLINE_H_ */
