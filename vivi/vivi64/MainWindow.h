#pragma once

#include <QtWidgets/QMainWindow>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QLabel>
#include "ui_MainWindow.h"
#include "EditView.h"
class FindLineEdit;

//typedef QPlainTextEdit	EditView;

typedef const char cchar;

//class SettingsMgr;
class TypeSettings;
class GlobalSettings;
class SSSearch;

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
	byte	searchAlgorithm() const;	// { return m_searchAlgorithm; }
	SSSearch	&sssrc() { return *m_sssrc; }
	SSSearch	&sssrc2() { return *m_sssrc2; }		//	カーソル位置単語検索用
	int		newLineType() const;
protected:
	//bool	focusNextPrevChild(bool next);
	void	createActions();
	void	createMenus();
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
	void	setTypeSettings(EditView *, TypeSettings *);
	void	onViewLineNumberChanged(const QString &, bool);
	QIcon	*typeNameToIcon(const QString&);
	void	closeNullDocs();			//	空のドキュメントをクローズ
	bool	doSave(EditView *);
	bool	doSaveAs(EditView *);
    bool	maybeSave();
    bool	maybeSave(EditView *);
	void	reload(cchar *codecName = 0);
    
protected:
	void	dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent* event);

private slots:
	void	on_action_NewWindow_triggered();
	void	on_action_New_triggered();
	void	on_action_Open_triggered();
	void	on_action_Save_triggered();
	void	on_action_SaveAs_triggered();
	void	on_action_Close_triggered();
	void	on_action_Reload_triggered();
	void	on_action_AddCurrentFile_triggered();
	void	on_action_RemoveFile_triggered();
	void	on_action_eXit_triggered();
	void	on_action_Undo_triggered();
	void	on_action_Redo_triggered();
	void	on_action_Cut_triggered();
	void	on_action_Copy_triggered();
	void	on_action_Paste_triggered();
	void	on_action_SelectAll_triggered();
	void	on_action_Search_triggered();
	void	on_action_SearchCurWord_triggered();
	void	on_action_SearchBackward_triggered();
	void	on_action_SearchForward_triggered();
	void	on_action_LineNumber_triggered();
	void	on_action_TypeSettings_triggered();
	void	on_action_GlobalSettings_triggered();
	void	on_action_About_ViVi_triggered();
	//
public slots:
    void	reloadRequested(EditView *, cchar *codecName = 0);
    void	openRecentFile();
    void	openFavoriteFile();
	void	tabCloseRequested(int index);
	void	currentChanged(int index);
    void	onCharEncodingChanged(const QString&);
    void	onBomChanged(bool);
	void	onTypeChanged(const QString&);
	void	onNewLineCodeChanged(int);
    void	onCursorPosChanged(int, int);
	void	showMessage(const QString&, int timeout = 0);
    void	updateUndoRedoEnabled();
    void	modifiedChanged();
    void	doFindString();
    void	findStringChanged(const QString&);
    void	findStringChanged(int);
    void	onFocusInFindLineEdit();
    void	onEscFindLineEdit();
    void	setFindString(const QString &txt);
    void	textSearched(const QString&txt, bool word);
	
private:
	Ui::MainWindowClass ui;
	
	//SettingsMgr		*m_settingsMgr;
	//GlobalSettings		*m_globSettings;
	
	bool	m_searching;			//	検索中
	bool	m_incSearched;		//	インクリメンタルサーチ済み
	byte	m_searchAlgorithm;
	int		m_curTabIndex;
	int		m_formerTabIndex;
	//int		m_docNumber;
	
	QComboBox	*m_findStringCB;
	QString			m_findString;
	QStringList		m_findStringHist;			//	検索文字列履歴
	int				m_findStringHistIndex;
	FindLineEdit	*m_findLineEdit;
	//
	SSSearch		*m_sssrc;
	SSSearch		*m_sssrc2;
	QDockWidget	*m_outlineDock;
	
	QLabel		*m_lineOffsetLabel;
	QLabel		*m_curCharCode;			//	カーソル位置文字コード
	QCheckBox	*m_bomChkBx;
	QComboBox	*m_encodingCB;
	QComboBox	*m_newLineCodeCB;
	QComboBox	*m_typeCB;
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
};
