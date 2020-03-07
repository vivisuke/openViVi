#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	qDebug() << "sizeof(char*) = " << sizeof(char*);
	return a.exec();
}
