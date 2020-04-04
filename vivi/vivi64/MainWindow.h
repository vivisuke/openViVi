#pragma once

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit>
#include "ui_MainWindow.h"

typedef QPlainTextEdit	EditView;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

protected:
	void	connectMenuActions();
	//EditView	*createView(Document *doc = 0, TypeSettings* = 0);
	EditView	*createView();
	void	addNewView(EditView *, const QString &title);
    EditView	*openFile(const QString &pathName, bool forced = false);
    
protected:
	void	dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent* event);

private slots:
	void	on_action_New_triggered();
	void	on_action_Open_triggered();
	void	on_action_Close_triggered();
	//
	void	tabCloseRequested(int index);
	
private:
	Ui::MainWindowClass ui;
	
	int		m_curTabIndex;
	int		m_formerTabIndex;
	int		m_docNumber;
};
