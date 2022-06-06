#ifndef MAINWINDOW_CONDUCTIVITY_H
#define MAINWINDOW_CONDUCTIVITY_H


#include <math.h>
#include <float.h>
#include <QFile>
#include <QMainWindow>
#include <QTextStream>

#include <glwidget.h>
#include <qwt3d_surfaceplot.h>

#include "global.h"
#include "maketetraidersobj.h"
#include "temperature.h"
#include "hat.h"
#include "bar.h"


using namespace Qwt3D;


namespace Ui {
class MainWindowConductivity;
}


class MainWindowConductivity : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindowConductivity(QWidget *parent = 0);
	~MainWindowConductivity();

private:
	Ui::MainWindowConductivity *ui;

	GLWidget *p_GLWidget;

	bool   ifchange;
	double ThirdKindCoeff[7][5]; // [1..6][1..4]

	void connect_GU();
	void disconnect_GU();
	void drawLayInit();

    long double max_solve;
    long double min_solve;
    long double mid_solve;
    long double offset;

	QColor value_to_color(long double value);

    Qwt3D::SurfacePlot* plot;
    Bar *bar;
    Hat *hat;

    bool rect_in_lay_y(const Bar::Rectangle &rect, int y);
	bool rect_in_lay_x(const Bar::Rectangle &rect, int y);
	bool rect_in_lay_z(const Bar::Rectangle &rect, int y);


private slots:
	void setFrontPerspective();
	void saveLay();
	void restoreSide(int side);
	void changeK(int K);
	void setIfchange();
	void drawLay();
};


#endif // MAINWINDOW_CONDUCTIVITY_H
