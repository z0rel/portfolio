#ifndef MAINWINDOW_CONDUCTIVITY_H
#define MAINWINDOW_CONDUCTIVITY_H

#include "mainwindow_base.h"


using namespace Qwt3D;

/// Главная форма приложения
class MainWindowLogic : public MainWindowBase
{
	Q_OBJECT
	
public:
    explicit MainWindowLogic(QWidget *parent = 0);

protected:
    void drawLayAction();
};


#endif // MAINWINDOW_CONDUCTIVITY_H
