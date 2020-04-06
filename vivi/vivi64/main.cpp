#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//
	app.setOrganizationName("VisualSoftwareLaboratory");
    app.setApplicationName("ViVi64");
#if	0
    auto path = app.applicationDirPath();
	QDir dir(path);
	dir.cdUp();
	dir.cdUp();
	path = dir.absolutePath();
#endif
    //
	MainWindow w;
	w.show();
	//
	return app.exec();
}
