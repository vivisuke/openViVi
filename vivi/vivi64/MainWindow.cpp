#include "version.h"
#include "MainWindow.h"
//#include "EditView.h"
#include <QFileDialog>
#include <qmimedata.h>
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
	setWindowTitle(QString("ViVi64 ver %1").arg(VERSION_STR));
	connectMenuActions();
	setAcceptDrops(true);		//ドロップを有効化
	
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
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	qDebug() << "dragEnterEvent()";
    if (event->mimeData()->hasUrls()
    	|| event->mimeData()->hasFormat("text/plain"))
    {
        event->acceptProposedAction();
    }
}
void MainWindow::dropEvent(QDropEvent* event)
{
	qDebug() << "dropEvent()";
	if( event->mimeData()->hasFormat("text/plain") ) {
		QString text = event->mimeData()->text();
		return;
	}
	//qDebug() << "MainWindow::dropEvent";
	QList<QUrl> fileList = event->mimeData()->urls();
	foreach(QUrl url, fileList) {
		QString fileName = QDir::toNativeSeparators(url.toLocalFile());
		fileName.replace('\\', '/');
		//qDebug() << fileName;
		openFile(fileName);
	}
}
void MainWindow::on_action_New_triggered()
{
	qDebug() << "on_action_Open_triggered()";
	EditView* view = createView();
	QString title = tr("Untitled-%1").arg(++m_docNumber);
	addNewView(view, title);
}
EditView *MainWindow::createView()
{
	EditView* view = new QPlainTextEdit();	//createView();
	view->setAcceptDrops(false);		//ドロップを無効化
	view->setTabStopDistance(24);
	return view;
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
	foreach(const QString &pathName, fileNameList) {
		qDebug() << "pathName = " << pathName;
		openFile(pathName);
	}
}
EditView *MainWindow::openFile(const QString &pathName, bool forced)
{
	QFile inputFile(pathName);
    inputFile.open(QIODevice::ReadOnly);
	QTextStream in(&inputFile);
    QString buf = in.readAll();
    inputFile.close();    
    
	EditView* view = createView();
	view->setPlainText(buf);
	QFileInfo info(pathName);
	auto title = info.fileName();
	addNewView(view, title);

	return view;
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
