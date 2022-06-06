#include <QMessageBox>


#include "mainwindow_logic.h"
#include "ui_mainwindow_fractals.h"

#include "bar_logic_lsystem.h"


/// Конструктор главной формы
MainWindowLogic::MainWindowLogic(QWidget *parent)
    : MainWindowBase(parent)
{
    static_cast<MainWindowBase*>(this)->init<BarLogicLSystem>();
    this->post_init();

    // Установка единого события перерисовки по изменению выбора на левой приборной панели
    connect(ui->radioButtonBush,          SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonChain,         SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonDragon,        SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonFlower,        SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonHosper,        SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonHylbert,       SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonIsland,        SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonMatSerpinsky,  SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonMosaic,        SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonPeano,         SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonSerpinsky,     SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonSnowflake,     SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonSnowflakeKosh, SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->radioButtonWeed,          SIGNAL(toggled(bool)),     SLOT(drawLay()));
    connect(ui->spinBoxLevel,             SIGNAL(valueChanged(int)), SLOT(drawLay()));
    connect(ui->checkBoxPoints,           SIGNAL(clicked()),         SLOT(drawLay()));

    reinterpret_cast<BarLogicLSystem*>(bar)->changeT(&BarLogicLSystem::axiomFlower, verifySpinBoxMaxLevel(5), 255, 0, 0);
    bar->noPaint = false;
    hat->create();
}


/// Бизнес-логика для настройки вывода на форме.
void MainWindowLogic::drawLayAction()
{
    plot->adjustSize();
    using T = BarLogicLSystem;
    // Выбрать то, что нужно рисовать.
    if (ui->radioButtonBush->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomBush, verifySpinBoxMaxLevel(6), 56, 166, 71);
    }
    else if (ui->radioButtonChain->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomChain, verifySpinBoxMaxLevel(5), 65, 35, 34);
    }
    else if (ui->radioButtonDragon->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomDragon, verifySpinBoxMaxLevel(14), 65, 35, 34);
    }
    else if (ui->radioButtonFlower->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomFlower, verifySpinBoxMaxLevel(5), 255, 0, 0);
    }
    else if (ui->radioButtonHosper->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomHosper, verifySpinBoxMaxLevel(5), 65, 35, 34);
    }
    else if (ui->radioButtonHylbert->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomHylbert, verifySpinBoxMaxLevel(8), 65, 35, 34);
    }
    else if (ui->radioButtonIsland->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomIsland, verifySpinBoxMaxLevel(4), 65, 35, 34);
    }
    else if (ui->radioButtonMatSerpinsky->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomMatSerpinsky, verifySpinBoxMaxLevel(9), 0, 0, 0);
    }
    else if (ui->radioButtonMosaic->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomMosaic, verifySpinBoxMaxLevel(3), 65, 35, 34 );
    }
    else if (ui->radioButtonPeano->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomPeano, verifySpinBoxMaxLevel(5), 65, 35, 34);
    }
    else if (ui->radioButtonSerpinsky->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomSerpinsky, verifySpinBoxMaxLevel(7), 65, 35, 34);
    }
    else if (ui->radioButtonSnowflake->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomSnowflake, verifySpinBoxMaxLevel(4), 0, 0, 255);
    }
    else if (ui->radioButtonSnowflakeKosh->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomSnowflakeKosh, verifySpinBoxMaxLevel(7), 0, 0, 255 );
    }
    else if (ui->radioButtonWeed->isChecked()) {
        reinterpret_cast<T*>(bar)->changeT(&T::axiomWeed, verifySpinBoxMaxLevel(7),  20, 98, 30 );
    }

    bar->isPoints = ui->checkBoxPoints->isChecked();
    bar->noPaint = true;
    hat->create();
    bar->noPaint = false;
}
