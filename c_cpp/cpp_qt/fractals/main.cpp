#include <QtWidgets/QApplication>

#include "bar_logic.h"
#include "mainwindow_logic.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    MainWindowLogic w;
	w.show();
	
	return a.exec();
}
