#include <QWidget>

#include "mainwindow_base.h"
#include "ui_mainwindow_fractals.h"


/// Конструктор главной формы
MainWindowBase::MainWindowBase(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindowConductivity)
{
    ui->setupUi(this);
    // Создание 3D виджета на котором будут рисоваться фракталы
    plot = new Qwt3D::GridPlot(this);
    ui->verticalLayout_surface->addWidget(plot, 0, Qt::Alignment());

    // Установка соотношения пропорций левой прибрной панели к правой области 3D рисования
    ui->verticalLayout_surface->setStretch(0, 1);
    ui->verticalLayout_surface->setStretch(1, 1);

    ui->verticalLayout_Logic->setStretch(0, 1);
    ui->verticalLayout_Logic->setStretch(1, 10);

    // Установка начального масштаба в 3D области.
    plot->setZoom(0.8);
    // Установка начальной проекции в X-Y
    plot->setRotation(90, 0, 0);
    // Усстановка центра увеличения в (0, 0)
    plot->setShift(0, 0, 0);
    // Подключение события обновления точки центра увеличения
    connect(plot, SIGNAL(vieportShiftChanged(double,double)), this, SLOT(shiftChange(double, double)));
    // Координаты в виде рамки
    plot->setCoordinateStyle(BOX);
}


void MainWindowBase::post_init()
{
    hat = new Hat(*plot);

    // Пределы координат по X и Y
    hat->setDomain(0, 1,  0, 1);
    hat->setMesh(10, 10);
    // Пределы координат по Z
    hat->setMaxZ(0);
    hat->setMinZ(0);
    hat->create();

    // количество основных и вспомогательных делений на осях
    for (size_t i = 0; i != plot->coordinates()->axes.size(); i++) {
        plot->coordinates()->axes[i].setMajors(5); // большие деления
        plot->coordinates()->axes[i].setMinors(5); // маленькие деления
    }

    plot->setMeshLineWidth(1);

    // Цает линий осей и шрифты чисел на них
    plot->coordinates()->setGridLinesColor(RGBA(0,0,0.5));
    plot->coordinates()->setNumberColor(RGBA(0,0,0));
    plot->coordinates()->setNumberFont("Courier",12);

    // Шрифт и значения подписей к осям
    plot->coordinates()->setLabelFont("Courier",14, QFont::Bold);
    plot->coordinates()->axes[X1].setLabelString("X");
    plot->coordinates()->axes[Y1].setLabelString("Y");
    plot->coordinates()->axes[Z1].setLabelString("Z");
    plot->coordinates()->axes[X2].setLabelString("X");
    plot->coordinates()->axes[Y2].setLabelString("Y");
    plot->coordinates()->axes[Z2].setLabelString("Z");
    plot->coordinates()->axes[X3].setLabelString("X");
    plot->coordinates()->axes[Y3].setLabelString("Y");
    plot->coordinates()->axes[Z3].setLabelString("Z");
    plot->coordinates()->axes[X4].setLabelString("X");
    plot->coordinates()->axes[Y4].setLabelString("Y");
    plot->coordinates()->axes[Z4].setLabelString("Z");

    // На верхней оси X и левой оси Y - деления не отображать
    plot->coordinates()->axes[X4].setMajors(0);
    plot->coordinates()->axes[X4].setMinors(0);
    plot->coordinates()->axes[X1].setMajors(0);
    plot->coordinates()->axes[X1].setMinors(0);

    plot->coordinates()->axes[Y3].setMajors(0);
    plot->coordinates()->axes[Y3].setMinors(0);
    plot->coordinates()->axes[Y4].setMajors(0);
    plot->coordinates()->axes[Y4].setMinors(0);

    // Установка сглаживания на координатных осях
    plot->coordinates()->setLineSmooth(true);

    // Отрисовка начального фрактала.
    plot->updateData();
    plot->update();
}


MainWindowBase::~MainWindowBase() { delete ui; }


/// Обработчик изменения центра увеличения по перетаскиванию
void MainWindowBase::shiftChange(double x, double y)
{
    plot->setShift(x, y, 0);
}


void MainWindowBase::changeMinMax(double xMin, double xMax, double yMin, double yMax)
{
    hat->setDomain(xMin, xMax, yMin, yMax);
}


/**
 * @brief MainWindowFractals::verifySpinBoxMaxLevel
 * Изменить максимальное значение spinBoxLevel (глубина построения фрактала)
 * И если новое максимальное значение больше текущего - изменить сначала текущее
 * до нового максимального. Изменять текущее значение нужно так, чтобы не был
 * сгенерирован сигнал на перерисовку.
 * @param maxLevel
 *  Новое максимальное значение spinBoxLevel.
 * @return
 *  Текущее значение spinBoxLevel.
 */
int MainWindowBase::verifySpinBoxMaxLevel(int maxLevel)
{
    if (ui->spinBoxLevel->value() > maxLevel) {
        disconnect( ui->spinBoxLevel, SIGNAL(valueChanged(int)), this, SLOT(drawLay()));
        ui->spinBoxLevel->setValue(maxLevel);
        connect(ui->spinBoxLevel, SIGNAL(valueChanged(int)), SLOT( drawLay()));
    }
    ui->spinBoxLevel->setMaximum(maxLevel);
    return ui->spinBoxLevel->value();
}


/// Слот для отрисовки нового фрактала.
void MainWindowBase::drawLay()
{
    drawLayAction();
    plot->coordinates()->setAutoScale(true);
    plot->coordinates()->setStandardScale();
    plot->coordinates()->setGridLines(false, false, Qwt3D::BACK);

    // Установить проекцию X-Y
    plot->setRotation(90 , 0, 0);

    // Обновить данные и геометрию. Отрисовать все.
    hat->create();
    plot->update();
    plot->updateData();
    plot->updateGeometry();
    plot->update();
}
