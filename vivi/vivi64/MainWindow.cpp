#include "version.h"
#include "MainWindow.h"
#include "CTabWidget.h"
#include "Document.h"
#include "EditView.h"
#include "settingsMgr.h"
#include "TypeStgDlg.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "charEncoding.h"
#include "FindLineEdit.h"
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
	//g_settingsMgr = new SettingsMgr();
	//char *ptr = nullptr;
	//qDebug() << "sizeof(ptr) = " << sizeof(ptr) << "\n";
	//setWindowTitle(QString("ViVi64 ver %1").arg(VERSION_STR));
	createActions();
	createMenus();
	//connectMenuActions();
	setAcceptDrops(true);		//ドロップを有効化
	//
	ui.mainToolBar->setAttribute( Qt::WA_AlwaysShowToolTips );
	ui.mainToolBar->insertWidget(ui.action_SearchForward, m_findStringCB = new QComboBox());
	m_findLineEdit = new FindLineEdit;
	m_findLineEdit->setPlaceholderText(tr("search text"));
	QFont fnt = m_findLineEdit->font();
	fnt.setPointSize(10);
	fnt.setFamily(QString((QChar *)L"ＭＳ ゴシック"));
	m_findLineEdit->setFont(fnt);
	m_findStringCB->setFont(fnt);
	m_findStringCB->setLineEdit(m_findLineEdit);
	//connect(lineEdit, SIGNAL(escPressed()), this, SLOT(escPressed()));
	m_findStringCB->setMinimumWidth(160);
	m_findStringCB->setMaximumWidth(160);m_findStringCB->setEditable(true);
	m_findStringCB->setCompleter(0);
	m_findStringCB->setInsertPolicy(QComboBox::InsertAtTop);
#if	0
	updateFindStringCB();
	connect(m_findStringCB->lineEdit(), SIGNAL(returnPressed()), this, SLOT(doFindString()));
	connect(m_findStringCB, SIGNAL(editTextChanged(const QString &)),
					this, SLOT(findStringChanged(const QString &)));
	connect(m_findLineEdit, SIGNAL(escPressed()), this, SLOT(onEscFindLineEdit()));
	connect(m_findLineEdit, SIGNAL(focusIn()), this, SLOT(onFocusInFindLineEdit()));
#endif
	ui.mainToolBar->setObjectName("MainToolBar");
	ui.mainToolBar->setWindowTitle(tr("MainToolBar"));
	//
	m_outlineDock = new QDockWidget(tr("Outline"));
	m_outlineDock->setObjectName("Outline");
	m_outlineDock->setWidget(new QPlainTextEdit());
	m_outlineDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::LeftDockWidgetArea, m_outlineDock);
	//	デザイナでタブの消し方がわからないので、ここで消しておく
	while( ui.tabWidget->count() )
		ui.tabWidget->removeTab(0);
	ui.tabWidget->tabBar()->setShape(QTabBar::RoundedNorth);
	//ui.tabWidget->setTabsClosable(true);		//	タブクローズ可能
	ui.tabWidget->setMovable(true);				//	タブ移動可能
	//ui.tabWidget->setTabShape(QTabWidget::Triangular);				//	タブ形状指定
	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
	//
	//ui.action_New->setIconText(tr("New"));
	//
	setupStatusBar();		//	ステータスバーセットアップ
	updateWindowTitle();
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
    for (int i = 0; i < MaxFavoriteFiles; ++i) {
        m_favoriteFileActs[i] = new QAction(this);
        m_favoriteFileActs[i]->setVisible(false);
        connect(m_favoriteFileActs[i], SIGNAL(triggered()), this, SLOT(openFavoriteFile()));
    }
    for (int i = 0; i < MaxClipboardHist; ++i) {
        m_clipboardHistActs[i] = new QAction(this);
        m_clipboardHistActs[i]->setVisible(false);
        connect(m_clipboardHistActs[i], SIGNAL(triggered()), this, SLOT(insertClipboardHistText()));
    }
    m_clipboardHistActs[0]->setText(tr("No Clipboard Data"));
    m_clipboardHistActs[0]->setEnabled(false);
    m_clipboardHistActs[0]->setVisible(true);
}
void MainWindow::createMenus()
{
    QMenu *MRU = ui.menu_RecentFiles;
    for (int i = 0; i < MaxRecentFiles; ++i)
        MRU->addAction(m_recentFileActs[i]);
    updateRecentFileActions();
    QMenu *fvMRU = ui.menu_FavoriteFiles;
    for (int i = 0; i < MaxFavoriteFiles; ++i)
        fvMRU->addAction(m_favoriteFileActs[i]);
    updateFavoriteFileActions();
#if	0
    QMenu *menu = new QMenu();
    for (int i = 0; i < MaxClipboardHist; ++i) {
    	menu->addAction(m_clipboardHistActs[i]);
    }
    m_clipboardToolButton->setMenu(menu);
#endif
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
    QSettings settings;
    QStringList files = settings.value(KEY_FAVORITEFILELIST).toStringList();
    int numRecentFiles = qMin(files.size(), (int)MaxFavoriteFiles);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString fileName = files[i].replace('\\', '/');
        QString text = tr("&%1 %2")
        				.arg(QChar(i < 10 ? '0' + (i + 1) % 10 : 'A' + i - 10))
        				.arg(fileName.replace("&", "&&"));
        				//.arg(strippedName(files[i]));
        m_favoriteFileActs[i]->setText(text);
		//qDebug() << fileName;
        m_favoriteFileActs[i]->setData(fileName);
        m_favoriteFileActs[i]->setStatusTip(fileName);
        setIcon(fileName, m_favoriteFileActs[i]);
        m_favoriteFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxFavoriteFiles; ++j)
        m_favoriteFileActs[j]->setVisible(false);
}
void MainWindow::updateWindowTitle()
{
	QString text = "ViVi64 ";
	text += QString(VERSION_STR);
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		if( !view->title().isEmpty() )
			text = view->title() + " - " + text;
		if( !view->fullPathName().isEmpty() )
			text += " - " + view->fullPathName();
	}
	setWindowTitle(text);
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
	m_iconSPR = new QIcon(":/MainWindow/Resources/SPR.png");
	m_iconTXT = new QIcon(":/MainWindow/Resources/TXT.png");
	//
	statusBar()->addPermanentWidget(m_curCharCode = new QLabel());			//	カーソル位置文字コード
	statusBar()->addPermanentWidget(m_lineOffsetLabel = new QLabel());		//	カーソル位置
	statusBar()->addPermanentWidget(m_bomChkBx = new QCheckBox("BOM"));		//	BOM
	statusBar()->addPermanentWidget(m_encodingCB = new QComboBox());		//	文字エンコーディング
	QStringList encList;
	encList  << "Shift_JIS" << "EUC-JP"<< "UTF-8" << "UTF-16LE" << "UTF-16BE";
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
	m_typeCB->addItem(*m_iconSPR, "SPR");
	m_typeCB->addItem(*m_iconTXT, "TXT");
	m_typeCB->setMaxVisibleItems(m_typeCB->count());
	connect(m_typeCB, SIGNAL(currentIndexChanged(const QString &)),
			this, SLOT(onTypeChanged(const QString &)));
}
void MainWindow::showMessage(const QString &mess0, int timeout)
{
	QString mess = mess0;
#if	0
	if( !m_sbMessage.isEmpty() ) {
		mess += m_sbMessage;
		m_sbMessage.clear();
	}
#endif
	statusBar()->showMessage(mess, timeout);
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
	//setTypeSettings(view, typeSettings);
	setTypeSettings(view, typeSettings);
	view->setFocus();
	//view->update();
}
void MainWindow::onNewLineCodeChanged(int)
{
}
void MainWindow::onCursorPosChanged(int ln, int offset)
{
	EditView *view = (EditView *)sender();
	if( !isEditView(view) ) return;
	m_lineOffsetLabel->setText(QString("pos: %1 (%2:%3)")
												.arg(view->cursorPosition()).arg(ln).arg(offset));
	//##updateCurPosCharCode(view);
}
void MainWindow::setTypeSettings(EditView *view, TypeSettings *typeSettings)
{
	view->document()->setTypeSettings(typeSettings);
#if	0
	if( typeSettings->name() == "HTML" ) {
		view->setJSTypeSettings(g_settingsMgr.typeSettings("JS"));		//	for JavaScript
		view->setPHPTypeSettings(g_settingsMgr.typeSettings("PHP"));		//	for PHP
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
void MainWindow::closeNullDocs()			//	空のドキュメントをクローズ
{
	for(int i = ui.tabWidget->count(); --i >= 0; ) {
		//EditView *view = (EditView *)ui.tabWidget->widget(i);
		EditView *view = nthWidget(i);
		if( isEditView(view) && view->fullPathName().isEmpty()
			&& view->bufferSize() == 0 )
		{
			ui.tabWidget->removeTab(i);
			view->close();
			delete view;
		}
	}
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
	Document *doc = new Document(typeName);
	doc->setTitle(title);
	//Buffer* buffer = doc->buffer();
	//auto* typeSettings = new TypeSettings(typeName);
	EditView* view = new EditView(doc /*, typeSettings*/);	//QPlainTextEdit();	//createView();
	connect(view, SIGNAL(cursorPosChanged(int, int)), this, SLOT(onCursorPosChanged(int, int)));
	connect(view, SIGNAL(showMessage(const QString &, int)), this, SLOT(showMessage(const QString &, int)));
	if( !pathName.isEmpty() ) {
		if( !loadFile(doc, pathName) ) {
			//	undone: ファイルオープンに失敗した場合の後始末処理
			return nullptr;
		}
		addToRecentFileList(pathName);
		updateRecentFileActions();
	}
	addNewView(view, typeNameToIcon(typeName), title, pathName);
	updateWindowTitle();
	return view;
}
QIcon *MainWindow::typeNameToIcon(const QString& typeName)
{
    if( typeName == "CPP" )
    	return m_iconCPP;
    else if( typeName == "CSS" )
    	return m_iconCSS;
    else if( typeName == "C#" )
    	return m_iconCS;
    else if( typeName == "F#" )
    	return m_iconFS;
    else if( typeName == "HLSL" )
    	return m_iconHLSL;
    else if( typeName == "HTML" )
    	return m_iconHTML;
    else if( typeName == "JAVA" )
    	return m_iconJAVA;
    else if( typeName == "JS" )
    	return m_iconJS;
    else if( typeName == "PASCAL" )
    	return m_iconPASCAL;
    else if( typeName == "PERL" )
    	return m_iconPERL;
    else if( typeName == "PHP" )
    	return m_iconPHP;
    else if( typeName == "PYTHON" )
    	return m_iconPYTHON;
    else if( typeName == "RUBY" )
    	return m_iconRUBY;
    else if( typeName == "SQL" )
    	return m_iconSQL;
    else if( typeName == "MARKDN" )
    	return m_iconMARKDN;
    else if( typeName == "LOG" )
    	return m_iconLOG;
    else if( typeName == "SPR" )
    	return m_iconSPR;
    else //if( typeName == "TXT" )
    	return m_iconTXT;
}
void MainWindow::addNewView(EditView *view, QIcon *icon, const QString &title, const QString &pathName)
{
	auto cur = ui.tabWidget->addTab(view, *icon, title);
	ui.tabWidget->setCurrentIndex(cur);
	ui.tabWidget->tabBar()->setTabToolTip(cur, pathName.isEmpty() ? title : pathName);
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
void MainWindow::openFavoriteFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
		createView(action->data().toString());
}
bool MainWindow::loadFile(Document *doc, const QString &pathName, /*cchar *codecName,*/
										bool bJump)		//	保存カーソル位置にジャンプ
{
	uchar charEncoding;
	int bomLength;
	QString buf, errMess;
	if( !::loadFile(pathName, buf, errMess, charEncoding, bomLength) )
		return false;
	closeNullDocs();		//	空のドキュメントをクローズ
	qDebug() << "charEncoding = " << (int)charEncoding;
	doc->setPathName(pathName);
	doc->setPlainText(buf);
	doc->setCharEncoding(charEncoding);
	doc->setBOM(bomLength != 0);
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
void MainWindow::on_action_Save_triggered()
{
	qDebug() << "on_action_Save_triggered()";
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	doSave(view);
}
void MainWindow::on_action_SaveAs_triggered()
{
	qDebug() << "on_action_SaveAs_triggered()";
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	doSaveAs(view);
	updateWindowTitle();
}
bool MainWindow::doSave(EditView *view)
{
	if( view->fullPathName().isEmpty() ) {
		return doSaveAs(view);
	} else
		return view->saveFile();
}
bool MainWindow::doSaveAs(EditView *view)
{
	QString fileName = view->fullPathName();
	if( fileName.isEmpty() ) {
#if	0
		QString tn = view->typeSettings()->name();
		if( tn == "Default" || tn == "TXT" || tn == "MARKDN" ) {
			fileName = getLineText(*view->buffer(), 0).trimmed();
			fileName.replace("\t", " ");
			fileName.replace(QRegExp("[\\/:*?\"<>|]"), "");
		}
		else if( tn == "HTML" ) {
			const int slen = strlen("<title>");
			int ix = view->buffer()->strstr(L"<title>", slen, 0, -1, /*ic=*/true);
			if( ix >= 0 ) {
				int ix2 = view->buffer()->strstr(L"</title>", strlen("</title>"), ix, -1, /*ic=*/true);
				if( ix2 >= 0 ) {
					fileName = getText(*view->buffer(), ix+slen, ix2-ix-slen);
					fileName.replace(QRegExp("[\\/:*?\"<>|]"), "");
					fileName = fileName.trimmed();
				}
			}
		}
#endif
#if	0
		else if( tn == "PASCAL" ) {
			SSSearch sss;
			const int slen = strlen("program");
			int ix = view->buffer()->indexOf(sss, L"\\bprogram\\b", slen+2, 0, 0, /*last=*/-1, SSSearch::STD_REGEX);
			if( ix >= 0 ) {
				ix += slen;
				while( isSpaceChar(view->charAt(ix)) ) ++ix;
				if( isAlphaOrUnderbar(view->charAt(ix)) ) {
					int ix2 = ix;
					wchar_t c;
					//while( isAlnumOrUnderbar(view->charAt(ix2)) ) ++ix2;
					while( !isNewLineChar(c = view->charAt(ix2)) && c != ';' ) ++ix2;
					if( c == ';' ) {
						fileName = getText(*view->buffer(), ix, ix2-ix);
						fileName.replace(QRegExp("[\\/:*?\"<>|]"), "");
						fileName = fileName.trimmed();
					}
				}
			}
		}
#endif
	}
	if( fileName.isEmpty() )
		fileName = view->title();
	QString ext0 = getExtension(fileName);
	if (ext0.isEmpty()) {
		//	undone: 拡張子が無い場合は、タイプのデフォルト拡張子
		ext0 = view->typeSettings()->defaultExt();
	}
	QStringList filter;
	filter	<< tr("Text files (*.txt)")
			<< tr("cpp files (*.cpp)")
			<< tr("h files (*.h)")
			<< tr("Java (*.java)")
			<< tr("Pascal (*.pas)")
			<< tr("Ruby (*.rb)")
			<< tr("Python (*.py)")
			<< tr("C#(*.cs)")
			<< tr("F#(*.fs)")
			<< tr("HTML files (*.html)")
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
			<< tr("HLSL files (*.fx)")
			<< tr("all files (*.*)");
	QString *selptr = 0;
	const QString e = "(*." + ext0.toLower() + ")";
	int ix = 0;
	for(; ix < filter.size(); ++ix) {
		if( filter[ix].endsWith(e) ) break;
	}
	if( ix < filter.size() )
		selptr = &filter[ix];
	fileName = QFileDialog::getSaveFileName(this, tr("Save File"), fileName,
											filter.join(";;"),
											selptr);
	if( fileName.isEmpty() ) return false;
	QString ext = getExtension(fileName);
	if( !ext.isEmpty() && ext != ext0 ) {
		TypeSettings *typeSettings = g_settingsMgr.typeSettingsForExt(ext);
		setTypeSettings(view, typeSettings);
		int ix = m_typeCB->findText(view->typeName());
		if( ix < 0 ) ix = 0;
		m_typeCB->setCurrentIndex(ix);
	}
	view->setFullPathName(fileName);
	view->saveFile();
	addToRecentFileList(fileName);
	updateRecentFileActions();
	//##updateTabText(view);
	return true;
}
void MainWindow::on_action_Close_triggered()
{
	qDebug() << "on_action_Close_triggered()";
	tabCloseRequested(ui.tabWidget->currentIndex());
}
void MainWindow::on_action_RemoveFile_triggered()
{
	qDebug() << "on_action_RemoveFile()";
}
void MainWindow::on_action_AddCurrentFile_triggered()
{
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	QString fullPathName = view->fullPathName();
	if( fullPathName.isEmpty() ) return;
	addToFavoriteFileList(fullPathName);
	updateFavoriteFileActions();
}
//	レジストリの "favoriteFileList" に追加
void MainWindow::addToFavoriteFileList(const QString &fullPathName)		//	レジストリの "favoriteFileList" に追加
{
    QSettings settings;
    QStringList files = settings.value(KEY_FAVORITEFILELIST).toStringList();
    QString absPath = QDir(fullPathName).absolutePath();
    files.removeAll(absPath);
    files.push_front(absPath);
    while (files.size() > MaxFavoriteFiles)
        files.removeLast();
    settings.setValue(KEY_FAVORITEFILELIST, files);
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
	Document* doc = view->document();
	//	undo/redo
	updateUndoRedoEnabled();
	//	ドキュメントタイプ
	int ix = m_typeCB->findText(view->document()->typeName());
	if( ix < 0 ) ix = 0;
	m_typeCB->setCurrentIndex(ix);
	//	文字エンコーディング
	m_encodingCB->setCurrentIndex(doc->charEncoding()-1);
	m_bomChkBx->setCheckState(doc->bom() ? Qt::Checked : Qt::Unchecked);
	//
	auto* ts = doc->typeSettings();
	ui.action_LineNumber->setChecked(ts->boolValue(TypeSettings::VIEW_LINENUM));
	//
	updateWindowTitle();
}
void MainWindow::updateUndoRedoEnabled()
{
	EditView *view = currentWidget();
	bool canUndo = false, canRedo = false;
	if( isEditView(view) ) {
		canUndo = view->document()->canUndo();
		canRedo = view->document()->canRedo();
	}
	if( canUndo ) {
		ui.action_Undo->setEnabled(true);
		ui.action_Undo->setIcon(QIcon(":/MainWindow/Resources/undo_black.png"));
	} else {
		ui.action_Undo->setEnabled(false);
		ui.action_Undo->setIcon(QIcon(":/MainWindow/Resources/undo_lightgray.png"));
	}
	if( canRedo ) {
		ui.action_Redo->setEnabled(true);
		ui.action_Redo->setIcon(QIcon(":/MainWindow/Resources/redo_black.png"));
	} else {
		ui.action_Redo->setEnabled(false);
		ui.action_Redo->setIcon(QIcon(":/MainWindow/Resources/redo_lightgray.png"));
	}
}
void MainWindow::on_action_eXit_triggered()
{
	qDebug() << "on_action_eXit_triggered()";
	//	undone: 修了確認
	close();
}
void MainWindow::on_action_Undo_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->undo();
	}
}
void MainWindow::on_action_Redo_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->redo();
	}
}
void MainWindow::on_action_Cut_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->cut();
	}
}
void MainWindow::on_action_Copy_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		auto sz = view->copy();
		statusBar()->showMessage(QString(tr("%1 chars copyed")).arg(sz));
	}
}
void MainWindow::on_action_Paste_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->paste();
	}
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
	view->typeSettings()->writeSettings();
}
void MainWindow::on_action_GlobalSettings_triggered()
{
}
