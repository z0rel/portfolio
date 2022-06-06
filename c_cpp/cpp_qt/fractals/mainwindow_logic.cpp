#include <QMessageBox>

#include "mainwindow_logic.h"
#include "ui_mainwindow_fractals.h"

#include "bar_logic.h"


/// Конструктор главной формы
MainWindowLogic::MainWindowLogic(QWidget *parent) :
    MainWindowBase(parent)
{
    static_cast<MainWindowBase*>(this)->init<BarLogic>();
    this->post_init();

    // Установка единого события перерисовки по изменению выбора на левой приборной панели
    connect(ui->radioButtonTree,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonCrystal, SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonFern,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonMatA,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonMatB,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonMatS,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonLeaf,    SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->checkBoxPoints,     SIGNAL(clicked()),         SLOT(drawLay()));
    connect(ui->spinBoxLevel,       SIGNAL(valueChanged(int)), SLOT(drawLay()));
    bar->noPaint = false;
    hat->create();
}


/// Слот для отрисовки нового фрактала.
void MainWindowLogic::drawLayAction()
{
    using T = BarLogic;
    // Выбрать то, что нужно рисовать.
    if (ui->radioButtonTree->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_tree, 5, verifySpinBoxMaxLevel(8), 65, 35, 34);
    }
    else if (ui->radioButtonCrystal->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_crystal, 4, verifySpinBoxMaxLevel(9), 0, 0, 255);
    }
    else if (ui->radioButtonFern->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_fern, 4, verifySpinBoxMaxLevel(9), 56, 166, 71);
    }
    else if (ui->radioButtonMatA->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_matA, 3, verifySpinBoxMaxLevel(11), 153,  91, 22);
    }
    else if (ui->radioButtonMatB->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_matB, 3, verifySpinBoxMaxLevel(11), 90,  55,  37);
    }
    else if (ui->radioButtonMatS->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_matS, 3, verifySpinBoxMaxLevel(11), 0, 0, 0);
    }
    else if (ui->radioButtonLeaf->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(T::T_leaf, 2, verifySpinBoxMaxLevel(19), 108, 167, 69);
    }

    bar->isPoints = ui->checkBoxPoints->isChecked();
}
