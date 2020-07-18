#include <QtGui>
#include "GrepDlg.h"
#include "globalSettings.h"
#include "version.h"

GrepDlg::GrepDlg(GlobalSettings *globSettings, const QStringList &grepDirHist, int cnt, QWidget *parent)
	: QDialog(parent)
	, m_globSettings(globSettings)
{
	ui.setupUi(this);
	//if( !globSettings->certified() ) 
	//	setWindowTitle(tr("GrepDlg   (you can grep %1 times, including this one)").arg(cnt));
	//ui.ignoreCase->setIcon(QIcon(QPixmap(":/MainWindow/Resources/icSearch.png")));
	//ui.wholeWordOnly->setIcon(QIcon(QPixmap(":/MainWindow/Resources/wordSearch.png")));
	//ui.regExp->setIcon(QIcon(QPixmap(":/MainWindow/Resources/regexp.png")));
	ui.findString->setCompleter(0);
	ui.ignoreCase->setChecked(m_globSettings->boolValue(GlobalSettings::IGNORE_CASE));
	ui.wholeWordOnly->setChecked(m_globSettings->boolValue(GlobalSettings::WHOLE_WORD_ONLY));
	ui.regExp->setChecked(m_globSettings->boolValue(GlobalSettings::REGEXP));
	ui.grepSubDir->setChecked(m_globSettings->boolValue(GlobalSettings::GREP_SUB_DIR));
	connect(ui.regexpHelp, SIGNAL(clicked()), this, SLOT(regexpHelp()));
	ui.outputBar->setChecked(!m_globSettings->boolValue(GlobalSettings::GREP_VIEW));
	ui.grepView->setChecked(m_globSettings->boolValue(GlobalSettings::GREP_VIEW));
	
	QSettings settings;
    QStringList strList1 = settings.value("findStringList").toStringList();
    ui.findString->addItems(strList1);
    ui.findString->lineEdit()->setSelection(0, ui.findString->lineEdit()->text().size());
    QStringList strList2;
    strList2	<< "*.*"									//	0	setTypeName() の番号と一致すること！
    			<< "*.h;*.cpp;*.c;*.cxx"				//	1
    			<< "*.cs"								//	2
    			<< "*.java"								//	3
    			<< "*.pas;*.inc;*.int"					//	4
    			<< "*.rb"								//	5
    			<< "*.cgi;*.pm;*.pl;*.t"				//	6
    			<< "*.py"								//	7
    			<< "*.html;*.htm;*.php,*phtml"	//	8
    			<< "*.js;*.ts"								//	9
    			<< "*.css"								//	10
    			<< "*.fs,*fsi,*fsx,*fsscript,*ml,*mli"	//	11
    			<< "*.log"								//	12
    			<< "*.sql"								//	13
    			<< "*.txt";								//	14
    ui.extensions->addItems(strList2);
    ui.dir->addItems(grepDirHist);
    ui.dir->lineEdit()->setText(QDir::currentPath());
    ui.exclude->setText(settings.value("grepExclude").toString());
    m_fileSystemModel.setRootPath(QDir::currentPath());
    m_fileSystemModel.setFilter(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    ui.treeView->setModel(&m_fileSystemModel);
    connect(ui.treeView, SIGNAL(doubleClicked ( QModelIndex )),
    				this, SLOT(dirViewDoubleClicked ( QModelIndex )));
    //connect(ui.treeView, SIGNAL(clicked ( QModelIndex)),
    //				this, SLOT(dirViewClicked ( QModelIndex)));
    QHeaderView *header = ui.treeView->header();
    header->hideSection(1);
    header->hideSection(2);
    header->hideSection(3);
}
void GrepDlg::setTypeName(const QString &typeName)
{
	if( typeName == "CPP" )
		ui.extensions->setCurrentIndex(1);
	else if( typeName == "C#" )
		ui.extensions->setCurrentIndex(2);
	else if( typeName == "JAVA" )
		ui.extensions->setCurrentIndex(3);
	else if( typeName == "PASCAL" )
		ui.extensions->setCurrentIndex(4);
	else if( typeName == "RUBY" )
		ui.extensions->setCurrentIndex(5);
	else if( typeName == "PERL" )
		ui.extensions->setCurrentIndex(6);
	else if( typeName == "PYTHON" )
		ui.extensions->setCurrentIndex(7);
	else if( typeName == "HTML" || typeName == "PHP" )
		ui.extensions->setCurrentIndex(8);
	else if( typeName == "JS" || typeName == "TS" )
		ui.extensions->setCurrentIndex(9);
	else if( typeName == "CSS" )
		ui.extensions->setCurrentIndex(10);
	else if( typeName == "F#" )
		ui.extensions->setCurrentIndex(11);
	else if( typeName == "LOG" )
		ui.extensions->setCurrentIndex(12);
	else if( typeName == "SQL" )
		ui.extensions->setCurrentIndex(13);
	else if( typeName == "TXT" )
		ui.extensions->setCurrentIndex(14);
}

GrepDlg::~GrepDlg()
{
}
void GrepDlg::dirViewClicked( QModelIndex index )
{
}
void GrepDlg::dirViewDoubleClicked( QModelIndex index )
{
	//int row = index.row();
	QString path = m_fileSystemModel.filePath(index);
	ui.dir->lineEdit()->setText(path);
}
QString GrepDlg::findString() const
{
	return ui.findString->lineEdit()->text();
}
QString GrepDlg::extentions() const
{
	return ui.extensions->lineEdit()->text();
}
QString GrepDlg::dir() const
{
	return ui.dir->lineEdit()->text();
}
QString GrepDlg::exclude() const
{
	return ui.exclude->text();
}
bool GrepDlg::ignoreCase() const
{
	return ui.ignoreCase->isChecked();
}
bool GrepDlg::wholeWordOnly() const
{
	return ui.wholeWordOnly->isChecked();
}
bool GrepDlg::regExp() const
{
	return ui.regExp->isChecked();
}
bool GrepDlg::grepSubDir() const
{
	return ui.grepSubDir->isChecked();
}
bool GrepDlg::grepView() const
{
	return ui.grepView->isChecked();
}
void GrepDlg::setFindString(const QString &txt)
{
	ui.findString->lineEdit()->setText(txt);
}
void GrepDlg::regexpHelp()
{
	QString url = "http://vivi.dyndns.org/sse/regexp.html?from=" + QString(VERSION_STR);
	QDesktopServices::openUrl(QUrl(url));
}
void GrepDlg::keyPressEvent(QKeyEvent *event)
{
	QDialog::keyPressEvent(event);
}
