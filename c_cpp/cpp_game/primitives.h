#ifndef PRIMITIVES_H
#define PRIMITIVES_H

class Dot_2d {
public:
	int x, y;
	Dot_2d(int _x, int _y) : x(_x), y(_y) {}
};


inline int rgb(int r, int g, int b)  { return b | (g << 8) | (r << 16); }

void setColor(int color, int x, int y);
void drawHline(int color, int x, int y, int l);
void drawVline(int color, int x, int y, int l);
void drawRect(int color, int x, int y, int w, int h);
void drawFillRect(int color, int x, int y, int w, int h);
void DrawCircle(int color, int x0, int y0, int r) ;
void FillCircle(int color, int x0, int y0, int r);


bool CircleIntersects(double x, double y, double R, double L, const Dot_2d& A, const Dot_2d& B, Dot_2d& Z);

#endif
