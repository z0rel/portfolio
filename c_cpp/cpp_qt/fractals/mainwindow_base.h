#ifndef MAINWINDOW_BASE_H
#define MAINWINDOW_BASE_H


#include <QMainWindow>
#include <QFile>
#include <QTextStream>

#include <hat.h>
#include <float.h>
#include <math.h>

#include <qwt3d_gridplot.h>

#include "bar_base.h"


using namespace Qwt3D;

namespace Ui {
class MainWindowConductivity;
}


/// Главная форма приложения
class MainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowBase(QWidget *parent = 0);

    template <typename BarLogicT>
    void init() { bar = (BarBase*)plot->setPlotStyle(BarLogicT(this)); } /* То, что мы будем рисовать. */

    void post_init();

    ~MainWindowBase();

    void changeMinMax(double _xMin, double _xMax, double _yMin, double _yMax);

protected:
    /// Виджеты формы
    Ui::MainWindowConductivity *ui;

    /// Стиль рисования. В нем рисуется фрактал.
    BarBase* bar;

    /// Виджет 3D рисования
    Qwt3D::GridPlot *plot;

    /// Заглушка для того, чтобы все выводилось.
    Hat *hat;

    int verifySpinBoxMaxLevel( int maxLevel );
    virtual void drawLayAction() = 0;

protected slots:
    void shiftChange( double x, double y );
    void drawLay();
};


#endif // MAINWINDOW_BASE_H
