#ifndef BAR_H
#define BAR_H

#include <QVector>
#include <global.h>

#include <qwt3d_plot3d.h>

using namespace Qwt3D;


class Bar : public Qwt3D::VertexEnrichment
{
public:
	Bar();

    Qwt3D::Enrichment* clone() const { return new Bar(*this); }

    struct Point3
	{
        double x;
        double y;
        double z;
		QColor c;

        int x_lay;
        int y_lay;
        int z_lay;

		Point3() {}
		Point3(double _x, double _y, double _z, QColor _color, int _x_lay, int _y_lay, int _z_lay)
            : x(_x), y(_y), z(_z), c(_color), x_lay(_x_lay), y_lay(_y_lay), z_lay(_z_lay) {}
	};

	struct Rectangle
	{
		Rectangle() {}
		Rectangle(const Point3 &p1, const Point3 &p2, const Point3 &p3, const Point3 &p4)
		{
			set(p1, p2, p3, p4);
		}

		void set(const Point3 &p1, const Point3 &p2, const Point3 &p3, const Point3 &p4)
		{
            points[0] = p1;
            points[1] = p2;
            points[2] = p3;
            points[3] = p4;
		}

		Point3 points[4];
	};

	QVector<Rectangle> rectangles;

	bool is_line;

	void test_plot(double xs = 0, double ys = 0,  double zs = 0,
				   double dx = 3, double dy = 15, double dz = 3);

	void plotRect(const QVector<Rectangle> &vector);

	void drawBegin();
	void drawEnd();
    void draw(const Qwt3D::Triple &);
};


#endif // BAR_H
