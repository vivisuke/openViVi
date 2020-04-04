#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//
	app.setOrganizationName("VisualSoftwareLaboratory");
    app.setApplicationName("ViVi64");
    //
	MainWindow w;
	w.show();
	//
	return app.exec();
}
