#include "MainWindow.h"
#include "SingleApplication.h"
#include <QtWidgets/QApplication>
#include <QDir>
#include <QAbstractNativeEventFilter>
#include <QDebug>

QApplication* g_app = nullptr;
//SingleApplication* g_app = nullptr;

#if	0
class WinNativeEventFilter: public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override
    {
    	//qDebug() << "nativeEventFilter()";
    	if (eventType == "windows_generic_MSG") {
	    	qDebug() << "windows_generic_MSG";
	    	windows_generic_MSG* ev = static_cast<windows_generic_MSG *>(message);
    	}
        return false;
    }
};
#endif

int main(int argc, char *argv[]) 
{
	//QApplication app(argc, argv);
	SingleApplication app(argc, argv, "ViVi64");
	g_app = &app;
	//app.installNativeEventFilter(new WinNativeEventFilter());
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
	//QObject::connect(&app, SIGNAL(imeOpenStatusChanged()), &w, SLOT(imeOpenStatusChanged()));
	QObject::connect(&app, SIGNAL(onRecieved(const QString)), &w, SLOT(onRecieved(const QString)));
	w.show();
	//
	return app.exec();
}
