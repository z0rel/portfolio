#ifndef HAT_H
#define HAT_H

#include "qwt3d_function.h"

using namespace Qwt3D;

/// Для нашей задачи - это класс-заглушка
class Hat : public Function
{
public:
    Hat(Qwt3D::GridPlot & pw) : Function(pw) {}

    QString name() const { return "___"; }
    double operator() (double x, double y) { return x + y; }
};

#endif // HAT_H
