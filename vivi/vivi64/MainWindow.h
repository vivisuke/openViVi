#pragma once

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QCheckBox>
#include <QLabel>
#include <QCombobox>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QDir>
//#include <qnetwork.h>
#include "ui_MainWindow.h"
#include "EditView.h"
#include "OutlineBar.h"
#include "../buffer/Buffer.h"
class FindLineEdit;

//typedef QPlainTextEdit	EditView;

typedef const char cchar;
//typedef __uint64 pos_t;

enum {
	MODE_INS = 0,
	MODE_REP,
	MODE_VI,
	MODE_EX,
};

//class SettingsMgr;
class TypeSettings;
class GlobalSettings;
class SSSearch;
class ViEngine;
class CommandLine;
class FindLineEdit;
class AutoCompletionDlg;
class QNetworkAccessManager;

extern GlobalSettings	g_globSettings;
GlobalSettings *globSettings();

enum {
	NEWLINE_CRLF = 0,
	NEWLINE_LF,
	NEWLINE_CR,
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();
	
public:
	byte_t	searchAlgorithm() const;	// { return m_searchAlgorithm; }
	SSSearch	&sssrc() { return *m_sssrc; }
	SSSearch	&sssrc2() { return *m_sssrc2; }		//	カーソル位置単語検索用
	int		newLineType() const;
	uint	getSearchOpt(bool vi = false) const;
	bool	isBoxSelectMode() const;	// { return ui.action_BoxSelect->isChecked(); }
	bool	isKeisenMode() const;		// { return ui.action_Keisen->isChecked(); }
	//bool	willShowMatchedMG() const { return m_showMatchedBG; }
	QString	findString() const { return m_findString; }
	const ViEngine	*viEngine() const { return m_viEngine; }
	int		mode() const { return m_modeCB->currentIndex(); }
	int		isOpened(const QString&) const;		//	既にオープンされていれば、MDITabs インデックスを返す、-1 for not opened
	QString	matchedString() const { return m_matchedString; }

public:
	ViEngine	*viEngine() { return m_viEngine; }
	void	resetBoxKeisenMode();
	void	setMode(int);
	void	setSearchWordOpt();
	void	hideCmdLineEdit();
	void	commandLineMode(QChar = ':');
	bool	hasSearchBoxFocus();		//	検索ボックスがフォーカスを持っているか？
	//void	setShowMatchedBG(bool b) { m_showMatchedBG = b; }
	void	clearMatchedString() { m_matchedString.clear(); }
	void	setMatchedString(const QString& txt) { m_matchedString = txt; }
	void	setCurrentView(EditView*);

protected:
	//bool	focusNextPrevChild(bool next);
	void	createActions();
	void	createMenus();
	void	createDockWindows();
	void	connectMenuActions();
    void	addToFavoriteFileList(const QString &);		//	レジストリの "favoriteFileList" に追加
    void	addToRecentFileList(const QString &);		//	レジストリの "recentFileList" に追加
    void	updateRecentFileActions();
    void	updateFavoriteFileActions();
    void	updateWindowTitle();
	void	updateTabText(EditView *);
	void	updateFindStringCB();
	void	setIcon(const QString &fileName, QAction *action);
	void	setupIcons();
	void	setupStatusBar();
	//EditView	*createView(Document *doc = 0, TypeSettings* = 0);
	//EditView	*createView(TypeSettings* = nullptr);
	EditView	*createView(QString fullPathName = QString());
	void	addNewView(EditView *, QIcon*, const QString &title, const QString &pathName);
    bool	loadFile(Document *, const QString &fileName, /*cchar *codecName = 0,*/ bool = true);
    //EditView	*openFile(const QString &pathName, bool forced = false);
	EditView	*currentWidget();
	EditView	*nthWidget(int);
	const EditView	*nthWidget(int) const;
	void	setTypeSettings(EditView *, TypeSettings *);
	void	onViewLineNumberChanged(const QString &, bool);
	QIcon	*typeNameToIcon(const QString&);
	void	closeNullDocs();			//	空のドキュメントをクローズ
	bool	doSave(EditView *);
	bool	doSaveAs(EditView *);
    bool	maybeSave();
    bool	maybeSave(EditView *);
	void	reload(cchar *codecName = 0);
	void	typesettingsChanged(EditView *view);
	void	updateSssrc();
	void	doNextTab(int n=1);
	void	doPrevTab(int n=1);
    
protected:
	void	dragEnterEvent(QDragEnterEvent *event);
	void	dropEvent(QDropEvent* event);
	void	closeEvent(QCloseEvent *event);
	void	resizeEvent(QResizeEvent *event);
	void	execCommand(const QString &cmd);
	void	updateMapFileLine(EditView *);
	void	updateMapFileMarks(EditView *);
	void	showCurrentDir();
	void	doMoveCopyCommand(EditView *view, QString &arg);
	void	doSubstitute(EditView *view, const QString &arg);
	bool	parseSubstisute(const QString &arg, QString &pat, QString &rep, QString &opt);
	void	activateDockBar(const QString &);
	void	appendToExCmdHist(const QString &cmd);
	void	showAutoCompletionDlg(const QStringList &, QString = QString() /*, bool = false*/);
	void closeAutoCompletionDlg();
	void	setupCommandModeShortcut();
	void	setupInsertModeShortcut();
	void	makeSureOutputView();
	void	setEnabledViewMenues(bool);
	void	clearOutput();
protected:
	//void	addToOutlineBar(const QString&, const QString&);
	void	addToOutlineBar(EditView*);
	void	removeFromOutlineBar(EditView*);
	void	currentViewChangedAtOutlineBar(EditView*);
	QTreeWidgetItem*	pathToOutlineBarItem(const QString&);
	QTreeWidgetItem*	viewToOutlineBarItem(EditView*);

protected slots:
	void	onEditedCmdLineEdit(QString);
	void	readyReadStandardOutput();
	void	readyReadStandardError();
	//void	onFocusOutCmdLineEdit();
	void	onCmdLineTextChanged(const QString &);
	void	onEnterCmdLineEdit();
	void	doSearchCommand(EditView* view, QString& text);
	void	onEscCmdLineEdit();
	void	onSpaceCmdLineEdit();
	void	onSlashCmdLineEdit();
	void	onColonCmdLineEdit();
	void	onTabCmdLineEdit();
	bool	isEditCommand(QString &arg);
	void	fileNameCompletion(QDir &, QString = QString());
	void	onUpCmdLineEdit();
	void	onDownCmdLineEdit();
    //EditView	*openFile(const QString &fileName, bool forced = false);

private slots:
	void autoCmplKeyPressed(QString);
	void autoCmplBackSpace();
	void autoCmplDelete(bool, bool);
	void autoCmplLeft(bool, bool);
	void autoCmplRight(bool, bool);
	void autoCmplZenCoding();
	void autoCmplPasted();
	void autoCmplDecided(QString, bool);
	void autoCmplRejected();
	
private slots:
	void	on_action_NewWindow_triggered();
	void	on_action_New_triggered();
	void	on_action_Open_triggered();
	void	on_action_Save_triggered();
	void	on_action_SaveAs_triggered();
	void	on_action_Close_triggered();
	void	on_action_Reload_triggered();
	void	on_action_OpenOpenedFiles_triggered();
	void	on_action_AddCurrentFile_triggered();
	void	on_action_RemoveFile_triggered();
	void	on_action_cpp_h_triggered();
	void	on_action_eXit_triggered();
	void	on_action_Undo_triggered();
	void	on_action_Redo_triggered();
	void	on_action_Cut_triggered();
	void	on_action_Copy_triggered();
	void	on_action_Paste_triggered();
	void	on_action_SelectAll_triggered();
	void	on_action_DynamicCompletion_triggered();
	void	on_action_ZenCoding_triggered();
	void	on_action_IgnoreCase_triggered();
	void	on_action_WordSearch_triggered();
	void	on_action_RegExp_triggered();
	void	on_action_Search_triggered();
	void	on_action_SearchCurWord_triggered();
	void	on_action_SearchBackward_triggered();
	void	on_action_SearchForward_triggered();
	void	on_action_Incremental_triggered();
	void	on_action_LineNumber_triggered();
	void	on_action_BoxSelect_triggered();
	void	on_action_KeisenMode_triggered();
	void	on_action_TypeSettings_triggered();
	void	on_action_GlobalSettings_triggered();
	void	on_action_viCommand_triggered();
	void	on_action_viTutorial_triggered();
	void	on_action_helpViCommand_triggered();
	void	on_action_helpExCommand_triggered();
	void	on_action_Github_triggered();
	void	on_action_About_ViVi_triggered();
	void	on_action_ExCommand_triggered();
	void	on_action_NextTab_triggered();
	void	on_action_PrevTab_triggered();
	void	on_action_FormerTab_triggered();
	//void	on_action_ZenCoding_triggered();
	//
public slots:
    void	reloadRequested(EditView *, cchar *codecName = 0);
    void	openRecentFile();
    void	openFavoriteFile();
	void	tabCloseRequested(int index);
	void	tabCurrentChanged(int index);
    void	onCharEncodingChanged(const QString&);
    void	onBomChanged(bool);
	void	onTypeChanged(const QString&);
	void	onModeChanged(int);
	void	onNewLineCodeChanged(int);
    void	onCursorPosChanged();
    void	onCursorPosChanged(EditView*);
    void	onCursorPosChanged(EditView*, int, int);
	void	showMessage(const QString&, int timeout = 0);
    void	updateUndoRedoEnabled();
    void	modifiedChanged();
    void	onEnterFindCB();
    void	doFindString();
    void	findStringChanged(const QString&);
    void	findStringChanged(int);
    void	onFocusInFindLineEdit();
    void	onEscFindLineEdit();
    void	setFindString(const QString &txt);
    void	textSearched(const QString&txt, bool word);
	void	colorTheme();
	void	aboutToShowColorTheme();
	void	viModeChanged();
	void	insertText(QString);
	void	replaceText(QString);
	void	textInserted(const QString &);
	void	viCmdFixed();
	void	doExCommand(QString, bool bGlobal);
	void	doExCommand(QString cmd) { doExCommand(cmd, false); }
    //void	setFindString(const QString &txt);
	void	onRecieved(const QString);
	void	imeOpenStatusChanged();
	void	doOutput(const QString &);		//	アウトプットにテキスト出力
	void	doOutputToBar(const QString &);		//	アウトプットバーにテキスト出力
	void	doOutputToGrepView(const QString &);		//	grepビューにテキスト出力
	void	onOutlineItemDblClicked(QTreeWidgetItem*);
	void	onOutlineBarEnterPressed();
	void	onOutlineBarColonPressed();
#if	0
	void	onOutlineBarKeyHPressed();
	void	onOutlineBarKeyJPressed();
	void	onOutlineBarKeyKPressed();
	void	onOutlineBarKeyLPressed();
#endif
	
private:
	Ui::MainWindowClass ui;
	
	//SettingsMgr		*m_settingsMgr;
	//GlobalSettings		*m_globSettings;
	
	bool	m_searching;			//	検索中
	bool	m_incSearched;			//	インクリメンタルサーチ済み
	bool	m_modeChanging;			//	モード変更中
	//bool	m_showMatchedBG;		//	マッチ背景強調 at vi mode
	QString	m_matchedString;		//	マッチ強調文字列
	byte_t	m_searchAlgorithm;
	int		m_curTabIndex;
	int		m_formerTabIndex;
	//int		m_docNumber;
	ViEngine	*m_viEngine;
	QStringList		m_clipboardHist;		//	クリップボード履歴
	//QList<InsData>	m_insDataList;		//	挿入文字列履歴
	QString			m_insertedText;
	EditView			*m_textInsertedView;
	int					m_textInsertedPos;
	int					m_jumpLineNumber;		//	1 org
	int					m_possibleGrepCount;			//	Grep 実行可能回数
	int					m_possibleReplaceCount;		//	Replace 実行可能回数
	int					m_possibleCmdModeCount;		//	vi コマンドモード遷移可能回数
	pos_t			m_incSearchPos;				//	インクリメンタルサーチ開始位置
	QStringList		m_grepDirHist;				//	検索ディレクトリ履歴
    QStringList		m_exCmdHist;				//	ex-command 履歴
    int				m_exCmdHistIndex;
	QString		m_sbMessage;
	QTimer			m_timer;
	QTime			m_time;
	QThread			m_thread;
	QThread			m_idleThread;
	class GrepEngine		*m_grepEngine;
    //QNetworkAccessManager	*m_networkAccessManager;
    bool		m_reinserting;			//	再入力中
    QString		m_tagsJumpFileName;		//	tagsJump を行った文書のフルパス名
    int				m_tagsJumpLine;				//	tagsJump を行った文書行 0..*
    std::vector<QString>	m_tagsJumpFileNames;		//	tagsJump を行った文書のフルパス名
    std::vector<int>		m_tagsJumpLines;				//	tagsJump を行った文書行 0..*
    CommandLine	*m_cmdLineEdit;
    QString			m_cmdLineText;					//	入力されたテキスト for 履歴フィルタ
    int				m_autoCmplIndex;				//	自動補完開始位置
	AutoCompletionDlg	*m_autoCompletionDlg;
	bool			m_autoCmplDlgClosed;			//	自動補完ダイアログを閉じたばかり
    QProcess	*m_process;					//	外部コマンド実行用プロセス
    QMap<QString, quint64>	m_mapFilePathToLine;		//	値の上位は (uint)time(0) の値
    //	上位24bit (uint)time(0) の値の上位24bit、その次にマーク文字8ビット、下位32bit は位置
    QMultiMap<QString, quint64>	m_mapFilePathToMarks;
    int		m_seqGrepView;
    EditView	*m_currentView;			//	現在ビュー
    EditView	*m_grepView;			//	grep結果出力ビュー
    EditView	*m_outputView;			//	Outputビュー
    EditView*m_lastView;				//	Output にフォーカスが移る前のフォーカスビュー
    //##EditView*m_testView;				//	単体テスト用ビュー
    bool		m_testing;					//	テスト中フラグ
	
	QComboBox	*m_findStringCB;
	QString			m_findString;
	QStringList		m_findStringHist;			//	検索文字列履歴
	int				m_findStringHistIndex;
	FindLineEdit	*m_findLineEdit;
	//
	SSSearch		*m_sssrc;
	SSSearch		*m_sssrc2;
	QDockWidget		*m_outlineDock;
	//QTreeWidget		*m_outlineWidget;
	OutlineBar		*m_outlineBar;
	QDockWidget		*m_outputDock;
	QPlainTextEdit	*m_outputWidget;
	
	QLabel		*m_lineOffsetLabel;
	QLabel		*m_curCharCode;			//	カーソル位置文字コード
	QCheckBox	*m_bomChkBx;
	QComboBox	*m_encodingCB;
	QComboBox	*m_newLineCodeCB;
	QComboBox	*m_typeCB;
	QComboBox	*m_modeCB;				//	ins/rep/vi/ex
	QIcon	*m_iconCPP;
	QIcon	*m_iconCS;
	QIcon	*m_iconCSS;
	QIcon	*m_iconMARKDN;
	QIcon	*m_iconFS;
	QIcon	*m_iconHLSL;
	QIcon	*m_iconHTML;
	QIcon	*m_iconJAVA;
	QIcon	*m_iconJS;			//	JavaScript
	QIcon	*m_iconPASCAL;
	QIcon	*m_iconPERL;
	QIcon	*m_iconPHP;
	QIcon	*m_iconPYTHON;
	QIcon	*m_iconRUBY;
	QIcon	*m_iconSQL;
	QIcon	*m_iconLOG;
	QIcon	*m_iconSPR;
	QIcon	*m_iconTXT;
    enum { MaxRecentFiles = 10 + 26 };
    QAction *m_recentFileActs[MaxRecentFiles];
    enum { MaxFavoriteFiles = 10 + 26 };
    QAction *m_favoriteFileActs[MaxFavoriteFiles];
    enum { MaxRecentDirs = 10 + 26 };
    QAction *m_recentDirActs[MaxRecentDirs];
    QAction *m_recentFileSystemActs[MaxRecentDirs];
    enum { MaxClipboardHist = 10 + 26 };
    QAction *m_clipboardHistActs[MaxClipboardHist];
    std::vector<QAction *>	m_colorThemeActions;
};
