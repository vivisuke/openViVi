#include "MainWindow.h"
#include "SingleApplication.h"
#include <QtWidgets/QApplication>
#include <QDir>
#include <QAbstractNativeEventFilter>
#include <QDebug>
#include <windows.h>

QApplication* g_app = nullptr;
//SingleApplication* g_app = nullptr;


int main(int argc, char *argv[]) 
{
	//QApplication app(argc, argv);
	QString uniqKeyString = "ViVi64";		//	ローカルサーバ名
#ifdef	_DEBUG
	uniqKeyString += "_DEBUG";
#endif
	SingleApplication app(argc, argv, uniqKeyString);
	//
    QStringList files;
#if	0	//def	_DEBUG
	app.isFirstApp();		//	listen 起動のためにコール
#else
	if( !app.isFirstApp() /*&& argc > 1*/ ) {		//	~~ファイル指定ありの場合~~
		int nArgs = 0;
		wchar_t **argvw = CommandLineToArgvW(GetCommandLine(), &nArgs);
		for(int i = 1; i < nArgs; ++i) {
			QString a((QChar *)argvw[i]);
			if( argvw[i][0] != '-' ) {
				QDir path(a);;
				//if( !path.isAbsolute() )
				//	path.makeAbsolute();
				files += path.absolutePath();
			} else {
				files += a;
			}
		}
		LocalFree(argvw);
		QString a = files.join("\t");
		if( a.isEmpty() ) a = ":";		//	引数無しの場合は ":" を送る → 新規メインウィンドウ表示
		app.sendMessage(a);
		return 0;
	}
#endif
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
