#include "stdafx.h"
#include "svga/svga.h"
#include "primitives.h"
#include <math.h>


extern unsigned *shadow_buf;
extern unsigned int sv_width, sv_height;


void setColor(int color, int x, int y)
{
    *(int*)((char*)(shadow_buf) + y * sv_width * 4 + x * 4) = color;
}


void drawHline(int color, int x, int y, int l)
{
    for (int i = x; i < x + l; i++)
    {
        setColor(color, i, y);
    }
}


void drawVline(int color, int x, int y, int l)
{
    for (int i = y; i < y + l; i++)
    {
        setColor(color, x, i);
    }
}


void drawRect(int color, int x, int y, int w, int h)
{
    drawHline(color, x, y, w);
    drawVline(color, x, y, h);
    drawHline(color, x, y + h, w);
    drawVline(color, x + w, y, h);
}


void drawFillRect(int color, int x, int y, int w, int h)
{
    for (int i = y; i < y + h; i++)
    {
        drawHline(color, x, i, w);
    }
}


static void Draw8Point(int color, int x0,int y0, int xoff, int yoff)
{
    setColor(color, x0 + xoff, y0 + yoff);
    setColor(color, x0 - xoff, y0 + yoff);
    setColor(color, x0 + yoff, y0 + xoff);
    setColor(color, x0 + yoff, y0 - xoff);
    if (yoff)
    {
        setColor(color, x0 + xoff, y0 - yoff);
        setColor(color, x0 - xoff, y0 - yoff);
        setColor(color, x0 - yoff, y0 + xoff);
        setColor(color, x0 - yoff, y0 - xoff);
    }
}


void DrawCircle(int color, int x0, int y0, int r)
{
    int i;
    int imax = ((int)((int)r * 707)) / 1000 + 1;
    int sqmax = (int)r * (int)r + (int)r / 2;
    int y = r;
    Draw8Point(color, x0, y0, r, 0);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + y * y) > sqmax)
        {
            Draw8Point(color, x0, y0, i, y);
            y--;
        }
        Draw8Point(color, x0, y0, i, y);
    }
}


void FillCircle(int color, int x0, int y0, int r)
{
    for (int i = 0; i <= r; i++)
    {
        DrawCircle(color, x0, y0, i);
    }
}


// пересекает ли окружность радиуса R c центром в x,y
// отрезок (A,B) длина которого есть L
bool CircleIntersects(double x, double y, double R, double L, const Dot_2d& A, const Dot_2d& B, Dot_2d& Z)
{
    // Единичный вектор отрезка AB
    double Xv = (B.x - A.x) / L;
    double Yv = (B.y - A.y) / L;
    double Xd = (A.x - x);
    double Yd = (A.y - y);
    double b = 2 * (Xd * Xv + Yd * Yv);
    double c = Xd * Xd + Yd * Yd - R * R;
    double c4 = c + c;
    c4 += c4;

    double D = b * b - c4;
    if (D < 0)
    {
        return false; // нет корней, нет пересечений
    }

    D = sqrt(D);
    double l1 = (-b + D) * 0.5;
    double l2 = (-b - D) * 0.5;
    bool intersects1 = ((l1 >= 0.0) && (l1 <= L));
    bool intersects2 = ((l2 >= 0.0) && (l2 <= L));
    bool intersects = intersects1 || intersects2;
    if (intersects)
    {
        if (intersects1 && intersects2)
        {
            l1 = (l1 + l2) * 0.5;
            Z.x = A.x + Xv * l1;
            Z.y = A.y + Yv * l1;
        }
        else if (intersects1)
        {
            Z.x = A.x + Xv * l1;
            Z.y = A.y + Yv * l1;
        }
        else
        {
            // intersects2
            Z.x = A.x + Xv * l2;
            Z.y = A.y + Yv * l2;
        }
    }
    return intersects;
}
