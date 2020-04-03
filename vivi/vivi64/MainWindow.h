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
	void	addNewView(EditView *, const QString &title);

private slots:
	void	on_action_New_triggered();
	void	on_action_Open_triggered();
	
private:
	Ui::MainWindowClass ui;
	
	int		m_curTabIndex;
	int		m_formerTabIndex;
	int		m_docNumber;
};
