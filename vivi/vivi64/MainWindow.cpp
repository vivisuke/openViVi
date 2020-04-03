#include "MainWindow.h"
//#include "EditView.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_curTabIndex(-1)
	, m_formerTabIndex(-1)
	, m_docNumber(0)
{
	ui.setupUi(this);
	//char *ptr = nullptr;
	//qDebug() << "sizeof(ptr) = " << sizeof(ptr) << "\n";
	connectMenuActions();
	
	//	デザイナでタブの消し方がわからないので、ここで消しておく
	while( ui.tabWidget->count() )
		ui.tabWidget->removeTab(0);
	//
		on_action_New_triggered();
}
void MainWindow::connectMenuActions()
{
	//QObject::connect(ui.action_New, SIGNAL(triggered()), this, SLOT(on_action_New_triggered()));
}
void MainWindow::on_action_New_triggered()
{
	qDebug() << "on_action_Open_triggered()";
	EditView* view = new QPlainTextEdit();	//createView();
	QString title = tr("Untitled-%1").arg(++m_docNumber);
	addNewView(view, title);
}
void MainWindow::addNewView(EditView *view, const QString &title)
{
	auto cur = ui.tabWidget->addTab(view, title);
	ui.tabWidget->setCurrentIndex(cur);
	view->setFocus();
}
void MainWindow::on_action_Open_triggered()
{
	qDebug() << "on_action_Open_triggered()";
}
void MainWindow::on_action_Close_triggered()
{
	qDebug() << "on_action_Close_triggered()";
	tabCloseRequested(ui.tabWidget->currentIndex());
}
void MainWindow::tabCloseRequested(int index)
{
		ui.tabWidget->removeTab(index);
}
