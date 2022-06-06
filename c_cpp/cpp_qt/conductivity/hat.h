#ifndef HAT_H
#define HAT_H

#include <global.h>

#include <qwt3d_function.h>


using namespace Qwt3D;

class Hat : public Function
{
public:
    double min;
    double max;

    Hat(SurfacePlot& pw)
        : Function(pw)
	{
		min = 0;
		max = 1;
	}

	double operator()(double x, double y)
	{
        return (x + y) * (max - min) + min;
	}

    QString name() const { return "$\\frac{1}{x^2+y^2+\\frac{1}{2}}$"; }
};

#endif // HAT_H
