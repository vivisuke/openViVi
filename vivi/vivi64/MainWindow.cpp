#include "MainWindow.h"
//#include "EditView.h"
#include <QFileDialog>
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
	ui.tabWidget->setTabsClosable(true);		//	タブクローズ可能
	ui.tabWidget->setMovable(true);				//	タブ移動可能
	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)),
			this, SLOT(tabCloseRequested(int)));
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
	QStringList filter;
	filter	<< tr("all files (*.*)")
			<< tr("Text files (*.txt)")
			<< tr("cpp files (*.cpp *.cxx *.cp *.cc *.c)")
			<< tr("h files (*.h *.hpp *.hxx)")
			<< tr("Java (*.java)")
			<< tr("Pascal(*.pas *.inc *.int)")
			<< tr("Ruby (*.rb)")
			<< tr("Python (*.py)")
			<< tr("C#(*.cs)")
			<< tr("F#(*.fs *.fsi *.fsx *.fsscript *.ml *.mli)")
			<< tr("HTML files (*.html)")
			<< tr("XML files (*.xml)")
			<< tr("Markdown files (*.md)")
			<< tr("JavaScript (*.js)")
			<< tr("css files (*.css)")
			<< tr("CGI files (*.cgi)")
			<< tr("Perl files (*.pl)")
			<< tr("PHP files (*.php)")
			<< tr("LOG files (*.log)")
			<< tr("SQL files (*.sql)")
			<< tr("CSV files (*.csv)")
			<< tr("TSV files (*.tsv)")
			<< tr("HLSL files (*.fx)");
	QStringList fileNameList = QFileDialog::getOpenFileNames(this, tr("Open File"),
																QString(), filter.join(";;"));
	if( fileNameList.isEmpty() ) return;
	foreach(const QString &fileName, fileNameList) {
		qDebug() << "fineMame = " << fileName;
		//openFile(fileName);
	}
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
