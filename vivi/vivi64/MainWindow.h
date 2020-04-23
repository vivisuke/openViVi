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

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

protected:
	void	createActions();
	void	createMenus();
	void	connectMenuActions();
    void	addToFavoriteFileList(const QString &);		//	レジストリの "favoriteFileList" に追加
    void	addToRecentFileList(const QString &);		//	レジストリの "recentFileList" に追加
    void	updateRecentFileActions();
    void	updateFavoriteFileActions();
    void	updateWindowTitle();
	void	setIcon(const QString &fileName, QAction *action);
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
	void	on_action_AddCurrentFile_triggered();
	void	on_action_RemoveFile_triggered();
	void	on_action_eXit_triggered();
	void	on_action_Undo_triggered();
	void	on_action_Redo_triggered();
	void	on_action_Cut_triggered();
	void	on_action_Copy_triggered();
	void	on_action_Paste_triggered();
	void	on_action_LineNumber_triggered();
	void	on_action_TypeSettings_triggered();
	void	on_action_GlobalSettings_triggered();
	//
    void	openRecentFile();
    void	openFavoriteFile();
	void	tabCloseRequested(int index);
	void	currentChanged(int index);
    void	onCharEncodingChanged(const QString &);
    void	onBomChanged(bool);
	void	onTypeChanged(const QString &);
	void	onNewLineCodeChanged(int);
    void	onCursorPosChanged(int, int);
	void	showMessage(const QString &, int timeout = 0);
	
private:
	Ui::MainWindowClass ui;
	
	//SettingsMgr		*m_settingsMgr;
	//GlobalSettings		*m_globSettings;
	
	int		m_curTabIndex;
	int		m_formerTabIndex;
	//int		m_docNumber;
	
	QComboBox	*m_findStringCB;
	QString			m_findString;
	QStringList		m_findStringHist;			//	検索文字列履歴
	int				m_findStringHistIndex;
	FindLineEdit	*m_findLineEdit;
	//
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
