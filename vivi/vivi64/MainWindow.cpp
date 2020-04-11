#include "version.h"
#include "MainWindow.h"
#include "Document.h"
#include "EditView.h"
#include "settingsMgr.h"
#include "TypeStgDlg.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "charEncoding.h"
#include <QDockWidget>
#include <QFileDialog>
#include <QMimeData>
#include <QSettings>
#include <QComboBox>
#include <QDebug>

#define		KEY_RECENTFILELIST			"recentFileList"
#define		KEY_FAVORITEFILELIST		"favoriteFileList"
#define		KEY_RECENTDIRLIST			"recentDirList"

#define		MAX_FIND_STR_HIST			64
#define		MAX_CLIPBOARD_HIST		100
#define		MAX_N_EXT_CMD				32

int	g_docNumber = 0;
SettingsMgr	g_settingsMgr;
GlobalSettings	g_globalSettings;

//----------------------------------------------------------------------
/*
		on_action_New_triggered()
			createView();
			addNewView(view, title);
		
		on_action_Open_triggered()
			QFileDialog::getOpenFileNames()
			openFile(pathName)
				createView(typeName)
				loadFile(view, pathName)
				addNewView(view, title)

		↓
		
		on_action_New_triggered()
			createView();
				.....
		
		openRecentFile()
			createView(pathName)
				.....
		on_action_Open_triggered()
			QFileDialog::getOpenFileNames()
			createView(pathName)
				openFile(pathName)
				.....
				addNewView(view, title);


*/
//----------------------------------------------------------------------
bool isValid(QWidget *w, const QString &className)
{
	return w != 0 && QString(w->metaObject()->className()) == className;
}
bool isEditViewFocused(QWidget *w)
{
	return isValid(w, "EditView") && w->hasFocus();
}
bool isEditView(QWidget *w)
{
	return isValid(w, "EditView");
}
bool isSplitter(QWidget *w)
{
	return isValid(w, "QSplitter");
}
bool isStartPage(QWidget *w)
{
	return isValid(w, "SSEStartPage");
}
QString getExtension(const QString &fullPath)
{
	QString name = QFileInfo(fullPath).fileName();		//	ディレクトリを除去
	int ix = name.lastIndexOf('.');
	if( ix < 0 )
		return QString();
	else
		return name.mid(ix+1);
}
//----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_curTabIndex(-1)
	, m_formerTabIndex(-1)
	//, m_docNumber(0)
{
	ui.setupUi(this);
	//m_settingsMgr = new SettingsMgr();
	//char *ptr = nullptr;
	//qDebug() << "sizeof(ptr) = " << sizeof(ptr) << "\n";
	setWindowTitle(QString("ViVi64 ver %1").arg(VERSION_STR));
	createActions();
	createMenus();
	//connectMenuActions();
	setAcceptDrops(true);		//ドロップを有効化
	//
	m_outlineDock = new QDockWidget(tr("Outline"));
	m_outlineDock->setObjectName("Outline");
	m_outlineDock->setWidget(new QPlainTextEdit());
	m_outlineDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::LeftDockWidgetArea, m_outlineDock);
	//	デザイナでタブの消し方がわからないので、ここで消しておく
	while( ui.tabWidget->count() )
		ui.tabWidget->removeTab(0);
	ui.tabWidget->setTabsClosable(true);		//	タブクローズ可能
	ui.tabWidget->setMovable(true);				//	タブ移動可能
	ui.tabWidget->setTabShape(QTabWidget::Triangular);				//	タブ形状指定
	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
	//
	setupStatusBar();		//	ステータスバーセットアップ
	//
		on_action_New_triggered();
}
MainWindow::~MainWindow()
{
	//delete m_settingsMgr;
}
void MainWindow::createActions()
{
    //	RecentFilesMenu のための初期化
    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        connect(m_recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }
}
void MainWindow::createMenus()
{
    QMenu *MRU = ui.menuRecentFiles;
    for (int i = 0; i < MaxRecentFiles; ++i)
        MRU->addAction(m_recentFileActs[i]);
    updateRecentFileActions();
}
//	settings から RecentFile 情報を取り出し、m_recentFileActs に設定
void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value(KEY_RECENTFILELIST).toStringList();
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString fileName = files[i].replace('\\', '/');
        QString text = tr("&%1 %2")
        				.arg(QChar(i < 10 ? '0' + (i + 1) % 10 : 'A' + i - 10))
        				.arg(fileName.replace("&", "&&"));
        				//.arg(strippedName(files[i]));
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(fileName);
        m_recentFileActs[i]->setStatusTip(fileName);
        setIcon(fileName, m_recentFileActs[i]);
        m_recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        m_recentFileActs[j]->setVisible(false);
}
void MainWindow::updateFavoriteFileActions()
{
}
void MainWindow::setIcon(const QString &fileName, QAction *action)
{
}
void MainWindow::setupStatusBar()
{
	m_iconCPP = new QIcon(":/MainWindow/Resources/CPP.png");
	m_iconCS = new QIcon(":/MainWindow/Resources/CS.png");
	m_iconCSS = new QIcon(":/MainWindow/Resources/CSS.png");
	m_iconMARKDN = new QIcon(":/MainWindow/Resources/Markdown.png");
	m_iconFS = new QIcon(":/MainWindow/Resources/FS.png");
	m_iconHLSL = new QIcon(":/MainWindow/Resources/HLSL.png");
	m_iconHTML = new QIcon(":/MainWindow/Resources/HTML.png");
	m_iconJAVA = new QIcon(":/MainWindow/Resources/JAVA.png");
	m_iconJS = new QIcon(":/MainWindow/Resources/JS.png");
	m_iconLOG = new QIcon(":/MainWindow/Resources/LOG.png");
	m_iconPASCAL = new QIcon(":/MainWindow/Resources/PASCAL.png");
	m_iconPERL = new QIcon(":/MainWindow/Resources/PERL.png");
	m_iconPHP = new QIcon(":/MainWindow/Resources/PHP.png");
	m_iconPYTHON = new QIcon(":/MainWindow/Resources/PYTHON.png");
	m_iconRUBY = new QIcon(":/MainWindow/Resources/RUBY.png");
	m_iconSQL = new QIcon(":/MainWindow/Resources/SQL.png");
	m_iconTXT = new QIcon(":/MainWindow/Resources/TXT.png");
	//
	statusBar()->addPermanentWidget(m_bomChkBx = new QCheckBox("BOM"));		//	BOM
	statusBar()->addPermanentWidget(m_encodingCB = new QComboBox());		//	文字エンコーディング
	QStringList encList;
	encList << "UTF-8" << "UTF-16LE" << "UTF-16BE" << "Shift_JIS" << "EUC-JP";
	m_encodingCB->addItems(encList);
	connect(m_encodingCB, SIGNAL(currentIndexChanged(const QString &)),
			this, SLOT(onCharEncodingChanged(const QString &)));
	statusBar()->addPermanentWidget(m_newLineCodeCB = new QComboBox());		//	改行コード
	QStringList nlList; nlList << "CRLF" << "LF" << "CR";
	m_newLineCodeCB->addItems(nlList);
	connect(m_newLineCodeCB, SIGNAL(currentIndexChanged(int)),
			this, SLOT(onNewLineCodeChanged(int)));
	statusBar()->addPermanentWidget(m_typeCB = new QComboBox());	//	タイプ
	m_typeCB->addItem("Default");
	m_typeCB->addItem(*m_iconCPP, "CPP");
	m_typeCB->addItem(*m_iconCS, "C#");
	m_typeCB->addItem(*m_iconCSS, "CSS");
	m_typeCB->addItem(*m_iconFS, "F#");
	m_typeCB->addItem(*m_iconHLSL, "HLSL");
	m_typeCB->addItem(*m_iconHTML, "HTML");
	m_typeCB->addItem(*m_iconJAVA, "JAVA");
	m_typeCB->addItem(*m_iconJS, "JS");
	m_typeCB->addItem(*m_iconLOG, "LOG");
	m_typeCB->addItem(*m_iconMARKDN, "MARKDN");
	m_typeCB->addItem(*m_iconPASCAL, "PASCAL");
	m_typeCB->addItem(*m_iconPERL, "PERL");
	m_typeCB->addItem(*m_iconPHP, "PHP");
	m_typeCB->addItem(*m_iconPYTHON, "PYTHON");
	m_typeCB->addItem(*m_iconRUBY, "RUBY");
	m_typeCB->addItem(*m_iconSQL, "SQL");
	m_typeCB->addItem(*m_iconTXT, "TXT");
	connect(m_typeCB, SIGNAL(currentIndexChanged(const QString &)),
			this, SLOT(onTypeChanged(const QString &)));
}
void MainWindow::onCharEncodingChanged(const QString &)
{
}
void MainWindow::onBomChanged(bool)
{
}
void MainWindow::onTypeChanged(const QString &type)
{
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	TypeSettings *typeSettings = g_settingsMgr.typeSettings(type);
	setTypeSettings(view, typeSettings);
	view->setFocus();
	//view->update();
}
void MainWindow::onNewLineCodeChanged(int)
{
}
void MainWindow::setTypeSettings(EditView *view, TypeSettings *typeSettings)
{
	view->setTypeSettings(typeSettings);
#if	0
	if( typeSettings->name() == "HTML" ) {
		view->setJSTypeSettings(m_settingsMgr->typeSettings("JS"));		//	for JavaScript
		view->setPHPTypeSettings(m_settingsMgr->typeSettings("PHP"));		//	for PHP
	} else {
		view->setJSTypeSettings(0);
		view->setPHPTypeSettings(0);
	}
	view->setLineNumberVisible(typeSettings->boolValue(TypeSettings::VIEW_LINENUM));
	view->setLineBreak(typeSettings->boolValue(TypeSettings::LINE_BREAK_WIN_WIDTH));
	view->updateScrollBarInfo();
#endif
	view->updateFont();
	view->update();
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
		createView(fileName);
		//openFile(fileName);
	}
}
void MainWindow::on_action_NewWindow_triggered()
{
	qDebug() << "on_action_NewWindow_triggered()";
	MainWindow* w = new MainWindow();
	w->show();
}
void MainWindow::on_action_New_triggered()
{
	qDebug() << "on_action_New_triggered()";
	createView();
#if	0
	EditView* view = createView();
	QString title = tr("Untitled-%1").arg(++m_docNumber);
	addNewView(view, title);
#endif
}
EditView *MainWindow::createView(QString pathName)
{
	//	undone: pathName を既にオープンしている場合対応
	QFileInfo info(pathName);
	QString typeName, title;
	if( !pathName.isEmpty() ) {
		typeName = g_settingsMgr.typeNameForExt(getExtension(pathName));
		title = info.fileName();
	} else {
		title = tr("Untitled-%1").arg(++g_docNumber);
	}
	Document *doc = new Document();
	//Buffer* buffer = doc->buffer();
	auto* typeSettings = new TypeSettings(typeName);
	EditView* view = new EditView(doc, typeSettings);	//QPlainTextEdit();	//createView();
	if( !pathName.isEmpty() ) {
		if( !loadFile(view, pathName) ) {
			//	undone: ファイルオープンに失敗した場合の後始末処理
			return nullptr;
		}
		addToRecentFileList(pathName);
		updateRecentFileActions();
	}
	addNewView(view, title);
	return view;
}
#if	0
EditView *MainWindow::createView(QString typeName)
{
	auto* buffer = new Buffer();
	auto* typeSettings = new TypeSettings(typeName);
	EditView* view = new EditView(buffer, typeSettings);	//QPlainTextEdit();	//createView();
	//view->setAcceptDrops(false);		//ドロップを無効化
	//view->setTabStopDistance(24);
	return view;
}
#endif
void MainWindow::addNewView(EditView *view, const QString &title)
{
	auto cur = ui.tabWidget->addTab(view, title);
	ui.tabWidget->setCurrentIndex(cur);
	view->setFocus();
}
EditView *MainWindow::currentWidget()
{
	return nthWidget(ui.tabWidget->currentIndex());
}
EditView *MainWindow::nthWidget(int ix)
{
	QWidget *w = ui.tabWidget->widget(ix);
#if	0
	if( isSplitter(w) ) {
		QSplitter *sp = (QSplitter *)w;
		if( sp->count() > 1 && sp->widget(1)->hasFocus() )
			return (EditView *)sp->widget(1);
		else
			return (EditView *)sp->widget(0);
	}
#endif
	return (EditView *)w;
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
		//openFile(pathName);
		createView(pathName);
	}
}
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
    	createView(action->data().toString());
        //openFile(action->data().toString());
    }
}
#if	0
EditView *MainWindow::openFile(const QString &pathName, bool forced)
{
	//	undone: 既にファイルがオープンされている場合は、それをアクティブに
    //	undone: Document オブジェクト作成
    QString typeName = g_settingsMgr.typeNameForExt(getExtension(pathName));
	EditView* view = createView(typeName);
	if( !loadFile(view, pathName) ) {		//	Document オブジェクトを作成し、view を管理
		//	undone: ファイルオープンに失敗した場合の後始末処理
		return nullptr;
	}
	//view->setPlainText(buf);
	QFileInfo info(pathName);
	auto title = info.fileName();
	addNewView(view, title);
	addToRecentFileList(pathName);
	updateRecentFileActions();
	return view;
}
#endif
bool MainWindow::loadFile(Document *doc, const QString &pathName, /*cchar *codecName,*/
										bool bJump)		//	保存カーソル位置にジャンプ
{
	uchar charEncoding;
	int bomLength;
	QString buf, errMess;
	if( !::loadFile(pathName, buf, errMess, charEncoding, bomLength) )
		return false;
	doc->setPlainText(buf);
	QFileInfo info(pathName);
	doc->setLastModified(info.lastModified());
	return true;
}
void MainWindow::addToRecentFileList(const QString &fullPath)		//	レジストリの "recentFileList" に追加
{
    QSettings settings;
    QStringList files = settings.value(KEY_RECENTFILELIST).toStringList();
    QString absPath = QDir(fullPath).absolutePath();
    files.removeAll(absPath);
    files.push_front(absPath);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    settings.setValue(KEY_RECENTFILELIST, files);
}
void MainWindow::on_action_Close_triggered()
{
	qDebug() << "on_action_Close_triggered()";
	tabCloseRequested(ui.tabWidget->currentIndex());
}
void MainWindow::tabCloseRequested(int index)
{
	//	undone: 保存確認
		ui.tabWidget->removeTab(index);
}
void MainWindow::currentChanged(int index)
{
	if (index < 0) return;
	EditView *view = nthWidget(index);
	//auto typeName = view->typeName();
	int ix = m_typeCB->findText(view->typeName());
	if( ix < 0 ) ix = 0;
	m_typeCB->setCurrentIndex(ix);
}
void MainWindow::on_action_eXit_triggered()
{
	qDebug() << "on_action_eXit_triggered()";
	//	undone: 修了確認
	close();
}
void MainWindow::on_action_LineNumber_triggered()
{
	bool b = ui.action_LineNumber->isChecked();
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->typeSettings()->setBoolValue(TypeSettings::VIEW_LINENUM, b);
		onViewLineNumberChanged(view->typeName(), b);
	}
}
void MainWindow::onViewLineNumberChanged(const QString &typeName, bool b)
{
	for(int i = 0; i < ui.tabWidget->count(); ++i) {
		EditView *view = nthWidget(i);
		if( isEditView(view) && view->typeName() == typeName )
			view->setLineNumberVisible(b);
	}
}
void MainWindow::on_action_TypeSettings_triggered()
{
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	TypeStgDlg aDlg(view, view->typeSettings());
	aDlg.exec();
	//##typesettingsChanged(view);
}
void MainWindow::on_action_GlobalSettings_triggered()
{
}
