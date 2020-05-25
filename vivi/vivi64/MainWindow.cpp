#include <assert.h>
#include <QDockWidget>
#include <QFileDialog>
#include <QMimeData>
#include <QSettings>
#include <QComboBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>
#include "version.h"
#include "MainWindow.h"
#include "CommandLine.h"
#include "CTabWidget.h"
#include "Document.h"
#include "EditView.h"
#include "settingsMgr.h"
#include "TypeStgDlg.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "charEncoding.h"
#include "FindLineEdit.h"
#include "TextCursor.h"
#include "ViEngine.h"
#include "../buffer/sssearch.h"

#ifdef		WIN32
#include	<windows.h>
#include	<imm.h>		//	for IME
#endif

#define		APP_NAME					"ViVi64"

#define		KEY_RECENTFILELIST			"recentFileList"
#define		KEY_FAVORITEFILELIST		"favoriteFileList"
#define		KEY_RECENTDIRLIST			"recentDirList"

#define		MAX_FIND_STR_HIST			64
#define		MAX_CLIPBOARD_HIST		100
#define		MAX_N_EXT_CMD				32

#define		MODE_WIDTH				48

extern QApplication* g_app;

int	g_docNumber = 0;
SettingsMgr	g_settingsMgr;
GlobalSettings	g_globSettings;
GlobalSettings* globSettings() { return &g_globSettings; }

//----------------------------------------------------------------------
/*
		
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
bool isValid(const QWidget *w, const QString &className)
{
	return w != 0 && QString(w->metaObject()->className()) == className;
}
bool isEditViewFocused(QWidget *w)
{
	return isValid(w, "EditView") && w->hasFocus();
}
bool isEditView(const QWidget *w)
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
	, m_incSearched(false)
	, m_searching(false)
	//, m_showMatchedBG(false)
	, m_searchAlgorithm(SSSearch::SAKUSAKU)
	, m_cmdLineEdit(nullptr)
	, m_process(nullptr)
	//, m_docNumber(0)
	, m_incSearchPos(0)
{
	globSettings()->readSettings();
	ui.setupUi(this);
	m_viEngine = new ViEngine();
	connect(m_viEngine, SIGNAL(modeChanged()), this, SLOT(viModeChanged()));
	connect(m_viEngine, SIGNAL(insertText(QString)), this, SLOT(insertText(QString)));
	connect(m_viEngine, SIGNAL(replaceText(QString)), this, SLOT(replaceText(QString)));
	connect(m_viEngine, SIGNAL(cmdFixed()), this, SLOT(viCmdFixed()));
	m_sssrc = new SSSearch();
	m_sssrc2 = new SSSearch();
	//g_settingsMgr = new SettingsMgr();
	//char *ptr = nullptr;
	//qDebug() << "sizeof(ptr) = " << sizeof(ptr) << "\n";
	//setWindowTitle(QString("ViVi64 ver %1").arg(VERSION_STR));
	setWindowIcon(QIcon(":/MainWindow/Resources/pencil_orange.png"));
	setupIcons();				//	各タイプアイコン設定
	createActions();
	createMenus();
	connect(ui.menu_ColorTheme, SIGNAL(aboutToShow()), this, SLOT(aboutToShowColorTheme()));
	//connectMenuActions();
	setAcceptDrops(true);		//	ドロップを有効化
	//
	ui.mainToolBar->setAttribute( Qt::WA_AlwaysShowToolTips );
	ui.searchToolBar->setAttribute( Qt::WA_AlwaysShowToolTips );
	ui.otherToolBar->setAttribute( Qt::WA_AlwaysShowToolTips );
	ui.searchToolBar->insertWidget(ui.action_SearchForward, m_findStringCB = new QComboBox());
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
	//m_findLineEdit->setText(QString());
	updateFindStringCB();
	m_findStringCB->setCurrentText(QString());
	//m_findStringCB->setCurrentIndex(-1);
	//connect(m_findStringCB->lineEdit(), SIGNAL(returnPressed()), this, SLOT(doFindString()));
	connect(m_findStringCB, SIGNAL(editTextChanged(const QString &)), this, SLOT(findStringChanged(const QString &)));
	connect(m_findStringCB->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onEnterFindCB()));
	connect(m_findLineEdit, SIGNAL(escPressed()), this, SLOT(onEscFindLineEdit()));
#if	0
	connect(m_findLineEdit, SIGNAL(focusIn()), this, SLOT(onFocusInFindLineEdit()));
#endif
	//ui.mainToolBar->setObjectName("MainToolBar");
	ui.mainToolBar->setWindowTitle(tr("MainToolBar"));
	ui.searchToolBar->setWindowTitle(tr("SearchToolBar"));
	ui.otherToolBar->setWindowTitle(tr("OtherToolBar"));
	//
	createDockWindows();
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
	ui.action_WordSearch->setChecked(globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY));
	ui.action_IgnoreCase->setChecked(globSettings()->boolValue(GlobalSettings::IGNORE_CASE));
	ui.action_RegExp->setChecked(globSettings()->boolValue(GlobalSettings::REGEXP));
	ui.action_Incremental->setChecked(globSettings()->boolValue(GlobalSettings::INC_SEARCH));
	ui.action_viCommand->setChecked(globSettings()->boolValue(GlobalSettings::VI_COMMAND));
	//
	if( globSettings()->boolValue(GlobalSettings::VI_COMMAND) )
		setMode(MODE_VI);
	//
	on_action_New_triggered();
}
MainWindow::~MainWindow()
{
	//delete m_settingsMgr;
}
void MainWindow::createDockWindows()
{
	m_outlineDock = new QDockWidget(tr("Outline"));
	m_outlineDock->setObjectName("Outline");
	m_outlineDock->setWidget(m_outlineWidget = new QTreeWidget());
	//m_outlineWidget->setColumnCount(1);
	m_outlineWidget->setHeaderHidden(true);
	m_outlineDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::LeftDockWidgetArea, m_outlineDock);
	//
	m_outputDock = new QDockWidget(tr("Output"));
	m_outputDock->setObjectName("Output");
	m_outputDock->setWidget(m_outputWidget = new QPlainTextEdit());
	//m_outputWidget->setTabStopDistance(48);		05/22 効かない
	m_outputDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);
	m_outputDock->hide();		//	undone: 状態保存・復帰
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
		if( !view->title().isEmpty() ) {
			if( view->isModified() )
				text = view->title() + "* - " + text;
			else
				text = view->title() + " - " + text;
		}
		if( !view->fullPathName().isEmpty() )
			text += " - " + view->fullPathName();
	}
	setWindowTitle(text);
}
void MainWindow::updateTabText(EditView *view)
{
	int ix = ui.tabWidget->indexOf(view);
	if( ix < 0 )
		return;
	QString fullPathName = view->document()->fullPathName();
	QString fileName = QFileInfo(fullPathName).fileName();	//	ファイル名部分だけゲット
	if( fullPathName.isEmpty() ) fileName = view->title();
	if( view->isModified() )
		fileName += "*";
	ui.tabWidget->setTabText(ix, fileName);
	ui.tabWidget->setTabToolTip(ix, fullPathName);
}
void MainWindow::setIcon(const QString &fileName, QAction *action)
{
    QString typeName = g_settingsMgr.typeNameForExt(getExtension(fileName));
    if( typeName == "CPP" )
    	action->setIcon(*m_iconCPP);
    else if( typeName == "CSS" )
    	action->setIcon(*m_iconCSS);
    else if( typeName == "C#" )
    	action->setIcon(*m_iconCS);
    else if( typeName == "F#" )
    	action->setIcon(*m_iconFS);
    else if( typeName == "HLSL" )
    	action->setIcon(*m_iconHLSL);
    else if( typeName == "HTML" )
    	action->setIcon(*m_iconHTML);
    else if( typeName == "JAVA" )
    	action->setIcon(*m_iconJAVA);
    else if( typeName == "JS" )
    	action->setIcon(*m_iconJS);
    else if( typeName == "PASCAL" )
    	action->setIcon(*m_iconPASCAL);
    else if( typeName == "PERL" )
    	action->setIcon(*m_iconPERL);
    else if( typeName == "PHP" )
    	action->setIcon(*m_iconPHP);
    else if( typeName == "PYTHON" )
    	action->setIcon(*m_iconPYTHON);
    else if( typeName == "RUBY" )
    	action->setIcon(*m_iconRUBY);
    else if( typeName == "SQL" )
    	action->setIcon(*m_iconSQL);
    else if( typeName == "MARKDN" )
    	action->setIcon(*m_iconMARKDN);
    else if( typeName == "LOG" )
    	action->setIcon(*m_iconLOG);
    else if( typeName == "TXT" )
    	action->setIcon(*m_iconTXT);
}
void MainWindow::setupIcons()
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
}
void MainWindow::setupStatusBar()
{
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
	statusBar()->addPermanentWidget(m_modeCB = new QComboBox());	//	モード：ins/rep/vi/ex
	m_modeCB->setMaximumWidth(MODE_WIDTH);
	m_modeCB->addItem(tr("ins"));
	m_modeCB->addItem(tr("rep"));
	if( globSettings()->boolValue(GlobalSettings::VI_COMMAND) ) {
		m_modeCB->addItem(tr("vi"));
		m_modeCB->addItem(tr("ex"));
	}
	m_modeCB->setMaxVisibleItems(4 /*m_modeCB->count()*/);
	connect(m_modeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));
}
int MainWindow::newLineType() const
{
	return m_newLineCodeCB->currentIndex();
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
void MainWindow::onModeChanged(int md)
{
	if( md < MODE_EX ) {
		EditView *view = currentWidget();
		if( isEditView(view) )
			view->setFocus();
	} else {
		if( m_cmdLineEdit == nullptr || !m_cmdLineEdit->isVisible() ) {
			m_viEngine->setCmdLineChar(':');
			commandLineMode(QChar(':'));
		}
	}
}
void MainWindow::viModeChanged()
{
#if	0
	setAttribute( Qt::WA_InputMethodEnabled, false );
#else
#ifdef	WIN32
	if( m_viEngine->mode() == Mode::COMMAND ) {
		HWND hwnd = (HWND)window()->winId();
		HIMC himc = ImmGetContext(hwnd);
		ImmSetOpenStatus(himc, FALSE);		//	IME OFF
	}
#endif
#endif
	int vm = m_viEngine->mode();
	setMode(vm);
	switch( vm ) {
	case Mode::INSERT:
		setupInsertModeShortcut();
		break;
	case Mode::REPLACE:
		setupInsertModeShortcut();
		break;
	case Mode::COMMAND:
		setupCommandModeShortcut();
		if( m_viEngine->prevMode() == Mode::INSERT || m_viEngine->prevMode() == Mode::REPLACE ) {
			EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
			if( isEditView(view) )
				view->curMove(TextCursor::LEFT, 1, true);
		}
		break;
	case Mode::CMDLINE:
		commandLineMode(m_viEngine->cmdLineChar());
		break;
	}
}
void MainWindow::setupCommandModeShortcut()
{
#if	0	//##
	ui.action_Search->setShortcut(QKeySequence());
	ui.action_ScrollDownPage->setShortcut(QKeySequence("Ctrl+F"));
	ui.action_Decrement->setShortcut(QKeySequence());
	ui.action_ScrollDownHalfPage->setShortcut(QKeySequence("Ctrl+D"));
	ui.action_InputPrevLineChar->setShortcut(QKeySequence());
	ui.action_ExposeBottomOfScreen->setShortcut(QKeySequence("Ctrl+E"));
#endif
}
void MainWindow::setupInsertModeShortcut()
{
#if	0	//##
	ui.action_ScrollDownPage->setShortcut(QKeySequence());
	ui.action_Search->setShortcut(QKeySequence("Ctrl+F"));
	ui.action_ScrollDownHalfPage->setShortcut(QKeySequence());
	ui.action_Decrement->setShortcut(QKeySequence("Ctrl+D"));
	ui.action_ExposeBottomOfScreen->setShortcut(QKeySequence());
	ui.action_InputPrevLineChar->setShortcut(QKeySequence("Ctrl+E"));
#endif
}
void MainWindow::textInserted(const QString &text)
{
	if( m_viEngine->isRedoRecording() )
		m_viEngine->appendInsertedText(text);
	qDebug() << "ViEngine::m_insertedText = " << m_viEngine->insertedText();
#if 0	//##
	if( m_reinserting || text.isEmpty() || isNewLineChar(text[0].unicode()) ) return;
	EditView *view = (EditView *)sender();
	if( !isEditView(view) ) return;
	pos_t pos = view->cursorPosition();
	if( m_textInsertedView != view
		|| m_textInsertedPos != pos - text.size() )
	{
		m_textInsertedView = view;
		if( !m_viEngine->redoing() )
			m_insertedText = text;
	} else if( !m_viEngine->redoing() )
		m_insertedText += text;
	qDebug() << "textInserted: MainWindow::m_insertedText = " << m_insertedText << ", text = " << text;
	m_textInsertedPos = pos;
#endif // 0
}
void MainWindow::insertText(QString text)
{
	EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if( isEditView(view) ) {
		view->insertText(text);
		onCursorPosChanged(view);
	}
}
void MainWindow::replaceText(QString text)
{
	EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if( isEditView(view) ) {
		view->replaceText(text);
		onCursorPosChanged(view);
	}
}
void MainWindow::viCmdFixed()
{
#if	0
	if( m_viEngine->cmd() == ViCmd::SAVE_ALL_EXIT ) {
		on_action_SaveAll_Exit_triggered();
		return;
	}
#endif
	if( m_viEngine->cmd() == ViCmd::EXEC_YANK_TEXT) {
		bool bLine;
		QString text = m_viEngine->yankText(bLine);
		m_viEngine->resetStatus();
		m_viEngine->processCommandText(text);
		return;
	}
	EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if( isEditView(view) ) {
		view->doViCommand();
		onCursorPosChanged(view);
	}
}
void MainWindow::onNewLineCodeChanged(int)
{
	assert(0);
}
void MainWindow::onCursorPosChanged()
{
	EditView* view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if (isEditView(view)) {
		onCursorPosChanged(view);
	}
}
void MainWindow::onCursorPosChanged(EditView* view)
{
	auto ln = view->textCursor()->viewLine();
	auto offset = view->textCursor()->positionInLine();
	onCursorPosChanged(view, ln, offset);
}
void MainWindow::onCursorPosChanged(EditView* view, int ln, int offset)
{
	qDebug() << "onCursorPosChanged()";
	//EditView *view = (EditView *)sender();
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
void MainWindow::resizeEvent(QResizeEvent *event)
{
	if( m_cmdLineEdit != nullptr && m_curCharCode != nullptr ) {
		auto g = m_curCharCode->geometry();
		auto rct = m_cmdLineEdit->geometry();
		rct.setRight(g.left());
		m_cmdLineEdit->setGeometry(rct);
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
	//	done: pathName を既にオープンしている場合対応
    QString absPath = QDir(pathName).absolutePath();
	int ix = isOpened(absPath);
	if( ix >= 0 ) {
		auto* view = nthWidget(ix);
		ui.tabWidget->setCurrentIndex(ix);
		return view;
	}
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
	EditView* view = new EditView(this, doc /*, typeSettings*/);	//QPlainTextEdit();	//createView();
	connect(view, SIGNAL(modifiedChanged()), this, SLOT(modifiedChanged()));
	connect(view, SIGNAL(updateUndoRedoEnabled()), this, SLOT(updateUndoRedoEnabled()));
	connect(view, SIGNAL(cursorPosChanged(EditView*, int, int)), this, SLOT(onCursorPosChanged(EditView*, int, int)));
	connect(view, SIGNAL(reloadRequest(EditView *)), this, SLOT(reloadRequested(EditView *)));
	connect(view, SIGNAL(textSearched(const QString&, bool)), this, SLOT(textSearched(const QString&, bool)));
	connect(view, SIGNAL(textInserted(const QString &)), this, SLOT(textInserted(const QString &)));
	connect(view, SIGNAL(showMessage(const QString&, int)), this, SLOT(showMessage(const QString&, int)));
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
	onCursorPosChanged(view);
	//	done: タイトルをアウトラインバーに追加
	addToOutlineBar(view->title(), doc->fullPathName());
	//
	return view;
}
void MainWindow::addToOutlineBar(const QString& title, const QString& fullPath)
{
	auto top = new QTreeWidgetItem(QStringList(title));
	m_outlineWidget->addTopLevelItem(top);
	top->setExpanded(true);
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
const EditView *MainWindow::nthWidget(int ix) const
{
	const QWidget *w = ui.tabWidget->widget(ix);
#if	0
	if( isSplitter(w) ) {
		QSplitter *sp = (QSplitter *)w;
		if( sp->count() > 1 && sp->widget(1)->hasFocus() )
			return (EditView *)sp->widget(1);
		else
			return (EditView *)sp->widget(0);
	}
#endif
	return (const EditView *)w;
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
#if	0
EditView *MainWindow::openFile(const QString &fileName, bool forced)
{
	assert(0);
	return 0;
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
	updateTabText(view);
	return true;
}
bool MainWindow::maybeSave()
{
	for(int i = 0; i < ui.tabWidget->count(); ++i) {
		EditView *view = nthWidget(i);
		if( isEditView(view) ) {
			//##updateMapFileLine(view);
			//##updateMapFileMarks(view);
			if( !maybeSave(view) )
				return false;		//	キャンセルが選択された場合
		}
	}
	return true;
}
bool MainWindow::maybeSave(EditView *view)
{
	QString fullPathName = view->fullPathName();
	//bool b = view->isModified();
    if( view->isModified() && !(view->document()->isEmpty() && fullPathName.isEmpty()) ) {
    	ui.tabWidget->setCurrentWidget(view);
		QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, APP_NAME,
                     tr("The '%1' has been modified.\n"
                        "Do you want to save your changes ?").arg(view->title()),
                     QMessageBox::Save | QMessageBox::Discard
		     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return doSave(view);
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}
void MainWindow::on_action_Close_triggered()
{
	qDebug() << "on_action_Close_triggered()";
	tabCloseRequested(ui.tabWidget->currentIndex());
}
void MainWindow::on_action_Reload_triggered()
{
	qDebug() << "on_action_Reload_triggered()";
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	QTextCodec *tc = QTextCodec::codecForName("UTF-8");
	reload(tc->fromUnicode(view->codecName()));
}
void MainWindow::reload(cchar *codecName)
{
	EditView *view = currentWidget();
	if( isEditView(view) && !view->fullPathName().isEmpty() ) {
		reloadRequested(view, codecName);
	}
}
void MainWindow::reloadRequested(EditView *view, cchar *codecName)
{
	if( view->isModified() ) {
		QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, APP_NAME,
                     tr("The '%1' has been modified.\n"
                        "Do you want to reload ?").arg(view->title()),
                     QMessageBox::Yes | QMessageBox::Cancel);
        if( ret != QMessageBox::Yes ) return;
	}
	int curLine = view->cursorLine();
	if( loadFile(view->document(), view->fullPathName(), /*codecName,*/ false) ) {
		//m_encodingLabel->setText(view->codecName());
		int ix = m_encodingCB->findText(view->codecName());
		if( ix >= 0 ) m_encodingCB->setCurrentIndex(ix);
		view->jumpToLine(curLine);
		view->makeCursorInView();
		view->setModified(false);
	}
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
	EditView *view = nthWidget(index);
	if( view == 0 ) return;
	if( isEditView(view) ) {
		if( !maybeSave(view) )
			return;		//	キャンセルが選択された場合
		ui.tabWidget->removeTab(index);
		//##updateMapFileLine(view);
		//##updateMapFileMarks(view);
		//??delete view;
	}
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
	//
	QString fullPathName = view->fullPathName();
	if( fullPathName.isEmpty() ) return;
	QDir dir(fullPathName);
	dir.cdUp();
	qDebug() << dir.absolutePath();
	QDir::setCurrent(dir.absolutePath());
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
void MainWindow::modifiedChanged()
{
	EditView *view = (EditView *)sender();
	if( !isEditView(view) ) return;
	updateTabText(view);
	updateWindowTitle();
}
void MainWindow::on_action_cpp_h_triggered()
{
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	QString fullPath = view->fullPathName();
	if( fullPath.isEmpty() ) return;
	QString ext = getExtension(fullPath).toLower();
	fullPath = fullPath.left(fullPath.size() - ext.size());
	QString ext2;
	if( ext == "c" || ext == "cpp" || ext == "cxx" )
		ext2 = "h";
	else if( ext == "h" ) {
		//	.cpp が存在せず、c が存在する場合は c を開く
		if( !QFileInfo(fullPath + "cpp").exists() && QFileInfo(fullPath + "c").exists() )
			ext2 = "c";
		else
			ext2 = "cpp";
	} else
		return;
	fullPath += ext2;
	createView(fullPath);
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
void MainWindow::on_action_SelectAll_triggered()
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
		view->selectAll();
	}
}
void MainWindow::on_action_ZenCoding_triggered()
{
	EditView *view = currentWidget();
	if( isEditViewFocused(view) )
		view->zenCoding();
}
uint MainWindow::getSearchOpt(bool vi) const
{
	uint opt = 0;
	if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) )
		opt |= SSSearch::IGNORE_CASE;
	if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) )
		opt |= SSSearch::WHOLE_WORD_ONLY;
	if( vi || globSettings()->boolValue(GlobalSettings::REGEXP) )
		opt |= SSSearch::REGEXP;
	return opt;
}
void MainWindow::updateSssrc()
{
	QString pat = m_findStringCB->lineEdit()->text();
	if( pat.isEmpty() ) return;
	auto opt = getSearchOpt();
	m_sssrc->setup((wchar_t*)pat.data(), pat.size(), opt);
}
void MainWindow::on_action_IgnoreCase_triggered()
{
	globSettings()->setBoolValue(GlobalSettings::IGNORE_CASE, ui.action_IgnoreCase->isChecked());
	globSettings()->writeSettings();
#if	1
	on_action_SearchForward_triggered();
#else
	updateSssrc();
	EditView *view = currentWidget();
	if( isEditView(view) ) view->update();
#endif
}
void MainWindow::on_action_Incremental_triggered()
{
	globSettings()->setBoolValue(GlobalSettings::INC_SEARCH, ui.action_Incremental->isChecked());
	globSettings()->writeSettings();
}
void MainWindow::on_action_WordSearch_triggered()
{
	globSettings()->setBoolValue(GlobalSettings::WHOLE_WORD_ONLY, ui.action_WordSearch->isChecked());
	globSettings()->writeSettings();
	updateSssrc();
#if	1
	on_action_SearchForward_triggered();
#else
	updateSssrc();
	EditView *view = currentWidget();
	if( isEditView(view) ) view->update();
#endif
}
void MainWindow::on_action_RegExp_triggered()
{
	globSettings()->setBoolValue(GlobalSettings::REGEXP, ui.action_RegExp->isChecked());
	globSettings()->writeSettings();
	updateSssrc();
#if	1
	on_action_SearchForward_triggered();
#else
	updateSssrc();
	EditView *view = currentWidget();
	if( isEditView(view) ) view->update();
#endif
}
void MainWindow::on_action_SearchBackward_triggered()
{
	statusBar()->clearMessage();
	QString pat = m_findStringCB->lineEdit()->text();
	m_matchedString = pat;
	EditView *view = currentWidget();
	if( isEditView(view) && !pat.isEmpty() ) {
		//bool word = ui.action_WordSearch->isChecked();
		view->findPrev(pat /*, word*/);
	}
}
void MainWindow::on_action_SearchForward_triggered()
{
	statusBar()->clearMessage();
	QString pat = m_findStringCB->lineEdit()->text();
	m_matchedString = pat;
	if( pat.isEmpty() ) return;
	EditView *view = currentWidget();
	if( isEditView(view) /*&& !pat.isEmpty()*/ ) {
		//bool word = ui.action_WordSearch->isChecked();
		view->findNext(pat /*, word*/);
	}
}
void MainWindow::on_action_Search_triggered()
{
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	QString txt;
	if( view->hasSelectionInALine() ) {
		txt = view->selectedText();
		m_findStringCB->lineEdit()->setText(txt);
	} else
		txt = m_findStringCB->lineEdit()->text();
	m_findStringCB->lineEdit()->setSelection(0, txt.size());
	m_findStringCB->lineEdit()->setFocus();
}
void MainWindow::setSearchWordOpt()
{
	ui.action_WordSearch->setChecked(true);
	globSettings()->setBoolValue(GlobalSettings::WHOLE_WORD_ONLY, true);
	globSettings()->writeSettings();
}
void MainWindow::on_action_SearchCurWord_triggered()
{
	setSearchWordOpt();		//	単語単位検索オプション強制ON
	//
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	QString txt;
	if( !view->searchCurWord(txt) ) return;
	setFindString(txt);
}
void MainWindow::onEscFindLineEdit()
{
	//assert(0);
	EditView *view = currentWidget();
	if( view != nullptr ) {
		m_matchedString.clear();	//	マッチ強調終了
		view->setFocus();
	}
}
void MainWindow::onEnterFindCB()
{
#if	1
	//	undone: Enter は次検索、Shift + Enter は前検索
	const bool shift = (g_app->keyboardModifiers() & Qt::ShiftModifier) != 0;
	if( !shift )
		on_action_SearchForward_triggered();
	else
		on_action_SearchBackward_triggered();
	m_findLineEdit->setFocus();		//	若干泥縄的
#else
	//	~~undone: インクリメンタルサーチOFFの場合~~
	EditView *view = currentWidget();
	if( view != nullptr ) {
		ui.tabWidget->setCurrentWidget(view);
		view->setFocus();
	}
#endif
}
void MainWindow::doFindString()
{
	m_searching = true;
	statusBar()->clearMessage();
	if( m_incSearched ) {		//	インクリメンタルサーチ済み
#if	0
		EditView *view = currentWidget();
		if( view != 0 )
			view->setFocus();
#endif
	} else {
		QString pat = m_findStringCB->lineEdit()->text();
		EditView *view = currentWidget();
		if( isEditView(view) && !pat.isEmpty() ) {
			uint opt = getSearchOpt();
			if( !view->findForward(m_findString = pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH)) )
				statusBar()->showMessage(tr("'%1' was not found.").arg(pat), 3000);
			//else
			setFindString(pat);
			//##view->setFocus();
		}
	}
	m_searching = false;
}
void MainWindow::findStringChanged(const QString &pat)
{
	if( m_searching ) return;	//	検索中の場合、再検索しないようにねっ
	m_findString = pat;
	m_matchedString = pat;
	EditView *view = currentWidget();
	if (isEditView(view)) {
		if( pat.isEmpty() ) {
			//	undone: インクリメンタルサーチ終了
			view->update();
			return;
		}
		uint opt = getSearchOpt();
		if (!view->findForward(m_findString = pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), false))
			statusBar()->showMessage(tr("'%1' was not found.").arg(pat), 3000);
		else {
			onCursorPosChanged();
#if	0
			auto ln = view->textCursor()->viewLine();
			auto offset = view->textCursor()->positionInLine();
			//auto pos = view->textCursor()->position();
			onCursorPosChanged(ln, offset);
#endif
		}
		//m_incSearched = true;
	}
}
void MainWindow::findStringChanged(int)
{
}
void MainWindow::onFocusInFindLineEdit()
{
}
void MainWindow::textSearched(const QString&txt, bool word)
{
	ui.action_WordSearch->setChecked(word);
}
void MainWindow::typesettingsChanged(EditView *view)
{
	view->typeSettings()->writeSettings();
	view->typeSettings()->loadKeyWords();
#if 0	//##
	const bool b = view->typeSettings()->boolValue(TypeSettings::VIEW_LINENUM);
	ui.action_LineNumber->setChecked(b);
	onViewLineNumberChanged(view->typeName(), b);
	const bool bLB = view->typeSettings()->boolValue(TypeSettings::LINE_BREAK_WIN_WIDTH);
	ui.action_LineBreakWinWidth->setChecked(bLB);
	for(int i = 0; i < ui.tabWidget->count(); ++i) {
		//EditView *view = (EditView *)ui.tabWidget->widget(i);
		EditView *ptr = nthWidget(i);
		if( isEditView(ptr) && ptr->typeName() == view->typeName() ) {
			ptr->updateFont();
			view->document()->buildWholeMap();
		}
	}
#endif
	view->update();
}
void MainWindow::colorTheme()
{
    QAction *action = qobject_cast<QAction *>(sender());
	EditView *view = currentWidget();
	if( !isEditView(view) ) return;
	TypeSettings *typeSettings = view->typeSettings();
#ifdef		_DEBUG
	QDir dir(qApp->applicationDirPath());
	dir.cdUp();
	dir.cdUp();
	dir.cd("colors");
	QString fileName = dir.absolutePath() + "/";
#else
	QString fileName(qApp->applicationDirPath() + "/colors/");
#endif
	fileName += action->text() + ".stg";
	if( !typeSettings->load(fileName, true) ) 		//	true for load only Colors
		return;
	//view->update();
	typesettingsChanged(view);
}
void MainWindow::aboutToShowColorTheme()
{
#if 1
	for (int i = 0; i < (int)m_colorThemeActions.size(); ++i) {
		delete m_colorThemeActions[i];
	}
	m_colorThemeActions.clear();
	ui.menu_ColorTheme->clear();
#ifdef		_DEBUG
	QDir dir(qApp->applicationDirPath());
	dir.cdUp();
	dir.cdUp();
	dir.cd("colors");
	//QDir dir("colors");
#else
	QDir dir(qApp->applicationDirPath() + "/colors/");
#endif
	QStringList lst = dir.entryList(QStringList("*.stg"));
	foreach(const QString fileName, lst) {
		QAction *act = ui.menu_ColorTheme->addAction(fileName.left(fileName.size() - 4));
		connect(act, SIGNAL(triggered()), this, SLOT(colorTheme()));
		m_colorThemeActions.push_back(act);
	}
#endif
}
bool MainWindow::hasSearchBoxFocus()		//	検索ボックスがフォーカスを持っているか？
{
	return m_findLineEdit->hasFocus();
}
void MainWindow::setFindString(const QString &txt)
{
	m_searching = true;		//	再検索を行わないようにおまじない
    QSettings settings;
    QStringList strList = settings.value("findStringList").toStringList();
    strList.removeAll(txt);
    strList.push_front(txt);
	while( strList.size() > MAX_FIND_STR_HIST )
		strList.pop_back();
    settings.setValue("findStringList", strList);
	updateFindStringCB();
	m_findString = txt;
	m_searching = false;
}
void MainWindow::updateFindStringCB()
{
	m_searching = true;
    QSettings settings;
    QStringList strList = settings.value("findStringList").toStringList();
    while( m_findStringCB->count() )
	    m_findStringCB->removeItem(0);
	m_findStringCB->addItems(strList);
	m_searching = false;
}
byte MainWindow::searchAlgorithm() const	// { return m_searchAlgorithm; }
{
	if( globSettings()->boolValue(GlobalSettings::REGEXP) )
		return SSSearch::STD_REGEX;
	else
		return m_searchAlgorithm;
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
int MainWindow::isOpened(const QString& pathName) const		//	既にオープンされていれば、MDITabs インデックスを返す、-1 for not opened
{
	for(int i = 0; i < ui.tabWidget->count(); ++i) {
		const EditView *view = nthWidget(i);
		if( isEditView(view) ) {
			auto pn = view->fullPathName();
			if( pn == pathName )
				return i;
		}
	}
	return -1;
}
bool MainWindow::isBoxSelectMode() const
{
	return ui.action_BoxSelect->isChecked();
}
bool MainWindow::isKeisenMode() const
{
	return ui.action_KeisenMode->isChecked();
}
void MainWindow::resetBoxKeisenMode()
{
	ui.action_BoxSelect->setChecked(false);
	ui.action_KeisenMode->setChecked(false);
}
void MainWindow::setMode(int md)
{
	m_modeCB->setCurrentIndex(md);
	switch( md ) {
	case MODE_INS:
		viEngine()->setMode(Mode::INSERT);
		break;
	case MODE_REP:
		viEngine()->setMode(Mode::REPLACE);
		break;
	case MODE_VI:
		viEngine()->setMode(Mode::COMMAND);
		if( m_cmdLineEdit != nullptr )
			m_cmdLineEdit->hide();
		break;
	case MODE_EX:
		viEngine()->setMode(Mode::CMDLINE);
		break;
	}
}
void MainWindow::on_action_BoxSelect_triggered()
{
	if( ui.action_BoxSelect->isChecked() )
		ui.action_KeisenMode->setChecked(false);
}
void MainWindow::on_action_KeisenMode_triggered()
{
	if( ui.action_KeisenMode->isChecked() )
		ui.action_BoxSelect->setChecked(false);
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
void MainWindow::on_action_viCommand_triggered()
{
	bool b = ui.action_viCommand->isChecked();
	globSettings()->setBoolValue(GlobalSettings::VI_COMMAND, b);
	globSettings()->writeSettings();
	if( b ) {
		m_modeCB->addItem(tr("vi"));
		m_modeCB->addItem(tr("ex"));
	} else {
		if( mode() >= MODE_VI ) setMode(MODE_INS);
		m_modeCB->removeItem(MODE_EX);
		m_modeCB->removeItem(MODE_VI);
	}
}
void MainWindow::on_action_viTutorial_triggered()
{
	QDesktopServices::openUrl(QUrl("https://github.com/vivisuke/openViVi/wiki/viTutorial"));
}
void MainWindow::on_action_helpViCommand_triggered()
{
	QDesktopServices::openUrl(QUrl("https://github.com/vivisuke/openViVi/wiki/viCommand"));
}
void MainWindow::on_action_helpExCommand_triggered()
{
	QDesktopServices::openUrl(QUrl("https://github.com/vivisuke/openViVi/wiki/exCommand"));
}
void MainWindow::on_action_Github_triggered()
{
	QDesktopServices::openUrl(QUrl("https://github.com/vivisuke/openViVi"));
}
void MainWindow::on_action_About_ViVi_triggered()
{
	QMessageBox::about(this, "about ViVi",
						tr("<p><a href=\"https://github.com/vivisuke/openViVi\">openViVi</a> version ") + QString(VERSION_STR)
						+ tr("<br>Copyright (C) 2020 by N.Tsuda")
						+ tr("<br>Powered by <a href=\"https://www.qt.io/\">Qt</a> ") + QT_VERSION_STR);
}
#if	0
bool MainWindow::focusNextPrevChild(bool next)
{
	EditView *view = currentWidget();
	if( isEditView(view) ) {
#if	1
		const bool ctrl = false;
		const bool shift = false;
		const bool alt = false;
#else
		const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
		const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
		const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
#endif
		view->doInsertText("\t", ctrl, shift, alt);
		return false;
	} else
		return QMainWindow::focusNextPrevChild(next);
}
#endif
void MainWindow::doNextTab(int n)
{
	if( ui.tabWidget->count() < 2 ) return;
	int ix = (ui.tabWidget->currentIndex() + n) % ui.tabWidget->count();
	ui.tabWidget->setCurrentIndex(ix);
}
void MainWindow::doPrevTab(int n)
{
	if( ui.tabWidget->count() < 2 ) return;
	int ix = (ui.tabWidget->currentIndex() - n);
	while( ix < 0 ) ix += ui.tabWidget->count();
	ui.tabWidget->setCurrentIndex(ix);
}
void MainWindow::on_action_NextTab_triggered()
{
	//assert(0);
	doNextTab();
}
void MainWindow::on_action_PrevTab_triggered()
{
	//assert(0);
	doPrevTab();
}
void MainWindow::on_action_FormerTab_triggered()
{
	assert(0);
}
#if	0
void MainWindow::onEnterCmdLineEdit()
{
	assert(0);
}
void MainWindow::doExCommand(QString, bool bGlobal)
{
	assert(0);
}
#endif
void MainWindow::clearOutput()
{
	m_outputWidget->document()->clear();
}
void MainWindow::doOutput(const QString &text)		//	アウトプットにテキスト出力
{
	if( !m_outputDock->isVisible() )
		m_outputDock->show();
	m_outputWidget->textCursor().insertText(text);
	//assert(0);
}
void MainWindow::doOutputToBar(const QString &)		//	アウトプットバーにテキスト出力
{
	assert(0);
}
void MainWindow::doOutputToGrepView(const QString &)		//	grepビューにテキスト出力
{
	assert(0);
}
void MainWindow::execCommand(const QString &cmd)
{
	//assert(0);
	if( m_process == 0 ) {
		m_process = new QProcess();
		connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
		connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
	}
	m_process->start(cmd);
	auto err = m_process->error();
	bool b = m_process->waitForFinished();
	QByteArray ba = m_process->readAllStandardOutput();
}
void MainWindow::readyReadStandardOutput()
{
	clearOutput();
	QByteArray ba = m_process->readAllStandardOutput();
	//qDebug() << ba;
	QTextCodec *codec = QTextCodec::codecForName("Shift_JIS");
	QString text = codec->toUnicode(ba);
	doOutput(text);
}
void MainWindow::readyReadStandardError()
{
	clearOutput();
	QByteArray ba = m_process->readAllStandardError();
	//qDebug() << ba;
	QTextCodec *codec = QTextCodec::codecForName("Shift_JIS");
	QString text = codec->toUnicode(ba);
	doOutput(text);
}
void MainWindow::onRecieved(const QString)
{
	assert(0);
}
void MainWindow::imeOpenStatusChanged()
{
	assert(0);
}
