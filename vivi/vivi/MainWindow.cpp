#include "MainWindow.h"
#include <qdebug.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	char* ptr = nullptr;
	qDebug() << "sizeof(ptr) = " << sizeof(ptr) << "\n";
}
