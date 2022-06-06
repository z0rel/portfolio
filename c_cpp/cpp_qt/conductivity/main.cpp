#include <QtWidgets/QApplication>


#include "mainwindow_conductivity.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindowConductivity w;
	w.show();
	
	return a.exec();
}
