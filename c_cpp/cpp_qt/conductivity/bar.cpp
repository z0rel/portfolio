
#include <qbitmap.h>
#include <qwt3d_color.h>
#include <qwt3d_plot3d.h>


#include "bar.h"


Bar::Bar()
{	
	is_line = false;
}


void Bar::plotRect(const QVector<Rectangle> &vector)
{
	this->rectangles = vector;
}


void Bar::test_plot(double xs, double ys, double zs,
					double dx, double dy, double dz)
{
	Point3 p[8];
	p[0] = Point3(xs,      ys,      zs,      Qt::magenta, 0, 0, 0);
	p[1] = Point3(xs,      ys + dy, zs,      Qt::blue   , 0, 0, 0);
	p[2] = Point3(xs,      ys + dy, zs + dz, Qt::red    , 0, 0, 0);
	p[3] = Point3(xs,      ys,      zs + dz, Qt::blue   , 0, 0, 0);
	p[4] = Point3(xs + dx, ys + dy, zs,      Qt::green  , 0, 0, 0);
	p[5] = Point3(xs + dx, ys,      zs,      Qt::red    , 0, 0, 0);
	p[6] = Point3(xs + dx, ys,      zs + dz, Qt::green  , 0, 0, 0);
	p[7] = Point3(xs + dx, ys + dy, zs + dz, Qt::magenta, 0, 0, 0);

	rectangles << Rectangle(p[0], p[1], p[2], p[3]);
	rectangles << Rectangle(p[0], p[3], p[6], p[5]);
	rectangles << Rectangle(p[0], p[1], p[4], p[5]);
	rectangles << Rectangle(p[4], p[5], p[6], p[7]);
	rectangles << Rectangle(p[1], p[2], p[7], p[4]);
	rectangles << Rectangle(p[2], p[3], p[6], p[7]);

}


void Bar::drawBegin()
{
    // diag_ = (plot->hull().maxVertex - plot->hull().minVertex).length() * radius_;
    glLineWidth( 0 );
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1,1);

    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);
}


void Bar::drawEnd() {}


void Bar::draw(const Qwt3D::Triple &pos)
{
	for (int i = 0; i < rectangles.count(); i++)
	{
		Rectangle rect = rectangles.at(i);
		glBegin (GL_QUADS);
		for (int j = 0; j < 4; j++)
		{
			glColor3f (rect.points[j].c.redF(), rect.points[j].c.greenF(), rect.points[j].c.blueF());
			glVertex3d (rect.points[j].x, rect.points[j].y, rect.points[j].z);
		}
		glEnd();
		if (is_line)
		{
			glBegin (GL_LINES);
			glColor3f (0.0, 0.0, 0.0);
			glVertex3d (rect.points[0].x, rect.points[0].y, rect.points[0].z);
			for (int j = 1; j < 4; j++)
			{
				glVertex3d (rect.points[j].x, rect.points[j].y, rect.points[j].z);
				glVertex3d (rect.points[j].x, rect.points[j].y, rect.points[j].z);
			}
			glVertex3d (rect.points[0].x, rect.points[0].y, rect.points[0].z);
			//glVertex3d (rect.points[0].x, rect.points[0].y, rect.points[0].z);
			//glVertex3d (rect.points[2].x, rect.points[2].y, rect.points[2].z);
			glEnd();
		}
	}
}

