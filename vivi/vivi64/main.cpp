#include "MainWindow.h"
#include "singleapplication.h"
#include <QtWidgets/QApplication>
#include <QDir>

QApplication* g_app = nullptr;
//SingleApplication* g_app = nullptr;

int main(int argc, char *argv[]) 
{
	QApplication app(argc, argv);
	//SingleApplication app(argc, argv);
	g_app = &app;
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
	QObject::connect(&app, SIGNAL(imeOpenStatusChanged()), &w, SLOT(imeOpenStatusChanged()));
	w.show();
	//
	return app.exec();
}
