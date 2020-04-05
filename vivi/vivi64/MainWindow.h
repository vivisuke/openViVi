#pragma once

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit>
#include "ui_MainWindow.h"
#include "EditView.h"

//typedef QPlainTextEdit	EditView;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

protected:
	void	createActions();
	void	createMenus();
	void	connectMenuActions();
    void	addToFavoriteFileList(const QString &);		//	レジストリの "favoriteFileList" に追加
    void	addToRecentFileList(const QString &);		//	レジストリの "recentFileList" に追加
    void	updateRecentFileActions();
    void	updateFavoriteFileActions();
	void	setIcon(const QString &fileName, QAction *action);
	//EditView	*createView(Document *doc = 0, TypeSettings* = 0);
	EditView	*createView();
	void	addNewView(EditView *, const QString &title);
    EditView	*openFile(const QString &pathName, bool forced = false);
	EditView	*currentWidget();
	EditView	*nthWidget(int);
	void	onViewLineNumberChanged(const QString &, bool);
    
protected:
	void	dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent* event);

private slots:
	void	on_action_New_triggered();
	void	on_action_Open_triggered();
	void	on_action_Close_triggered();
	void	on_action_eXit_triggered();
	void	on_action_LineNumber_triggered();
	//
    void	openRecentFile();
	void	tabCloseRequested(int index);
	
private:
	Ui::MainWindowClass ui;
	
	int		m_curTabIndex;
	int		m_formerTabIndex;
	int		m_docNumber;
	
	QDockWidget	*m_outlineDock;
	
    enum { MaxRecentFiles = 10 + 26 };
    QAction *m_recentFileActs[MaxRecentFiles];
    enum { MaxFavoriteFiles = 10 + 26 };
    QAction *m_favoriteFileActs[MaxFavoriteFiles];
    enum { MaxRecentDirs = 10 + 26 };
    QAction *m_recentDirActs[MaxRecentDirs];
    QAction *m_recentFileSystemActs[MaxRecentDirs];
    enum { MaxClipboardHist = 10 + 26 };
    QAction *m_clipboardHistActs[MaxClipboardHist];
};
