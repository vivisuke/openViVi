#pragma once

#include <QScrollArea>
#include <QFont>
#include <QTimer>
#include "../buffer/Buffer.h"
//#include "typeSettings.h"
//#include "globalSettings.h"
class MainWindow;
class TypeSettings;
class GlobalSettings;
class Buffer;
class Document;
class TextCursor;
class ViewLineMgr;

class EditView : public QWidget		//QScrollArea
{
	Q_OBJECT
public:
	EditView(MainWindow*, Document *doc /*, TypeSettings* = nullptr*/);
	~EditView();
public:
	wchar_t	charAt(pos_t pos) const;
	size_t	bufferSize() const;
	pos_t	cursorPosition() const;
	int		EOFLine() const;
	QString	fullPathName() const;
	QString	title() const;
	QString	typeName() const;
	TypeSettings	*typeSettings();	// { return m_typeSettings; }
	const TypeSettings	*typeSettings() const;	// { return m_typeSettings; }
	//const TypeSettings* cTypeSettings() { return (const TypeSettings*)m_typeSettings; }
	bool	isModified() const;		// { return m_modified; }
	bool	hasSelection() const;
	bool	hasSelectionInALine() const;
	QString	text(pos_t pos, ssize_t sz) const;
	QString	selectedText() const;
	QString	newLineText() const;
	int	viewLineOffsetToPx(int vln, int offset) const;
	const Document*	document() const { return m_document; }
	const Buffer	*buffer() const { return m_buffer; }
	int		positionToLine(pos_t pos) const;
	int		docLineToViewLine(int dln) const;
	int		viewLineToDocLine(int vln) const;
	int		viewLineToDocLine(int vln, int &offset) const;
	int		viewLineStartPosition(int vln) const;
	const ViewLineMgr *viewLineMgr() const { return m_viewLineMgr; }
	int		lineStartPosition(int ln) const;
	int		fontHeight() const { return m_fontHeight; }
	int		lineHeight() const { return m_lineHeight; }
	int		textWidth(const QString &) const;
	int		textWidth(pos_t, ssize_t, pos_t, const Buffer* = 0) const;
	int		pxToOffset(int vln, int px) const;
	void	pointToLineOffset(const QPoint &, int &, int &) const;
public:
	MainWindow	*mainWindow() { return m_mainWindow; }
	void	setLineNumberVisible(bool);
	void	setPlainText(const QString&);
	Document*	document() { return m_document; }
	Buffer	*buffer() { return m_buffer; }
	bool	makeCursorInView(bool bQuarter = false);
	void	deleteText(pos_t pos, ssize_t sz = 1, bool BS = false);
	void	onCursorPosChanged();
	void	doInsertText(const QString &, bool, bool, bool);
	void	insertTextRaw(pos_t pos, const QString &);
	void	paste(const QString &);
	void	boxPaste(const QString &);
	void	updateFont();
	void	setFullPathName(const QString &);
	bool	saveFile() const;
	bool	searchCurWord(QString &);
	bool	findForward(const QString &, uint opt = 0, bool loop = false, bool next = true, bool vi = false);
	bool	findBackward(const QString &, uint opt = 0, bool loop = false, bool vi = false);
	void	findNext(const QString &, bool vi = false);
	void	findPrev(const QString &, bool vi = false);
protected:
	void	resetCursorBlinking();
	void	drawLineNumbers();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	drawLineText(QPainter&, int&, int, int, pos_t, int, pos_t, bool&, bool&, QString&);
	int		drawTokenText(QPainter& pt, QString& token, int& clmn, int& px, int py, int peDX, int wd,
							const int chWidth, const int descent /*, QColor& col*/ /*, bool bold*/);
	void	drawPreeditString(QPainter&);
	void	drawCursor(QPainter&);
	void	drawLineCursor(QPainter&);
	void	drawSelection(QPainter&);
	void	drawMinMap(QPainter&);
	void	updateLineNumberInfo();
	void	onResized();
	void	insertTextSub(QString, bool ctrl, bool shift, bool alt);
	void	onBackSpace(bool ctrl, bool shift, bool alt);
	void	onDelete(bool ctrl, bool shift, bool alt);
protected:
	void	paintEvent(QPaintEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	wheelEvent(QWheelEvent * event);
	void	keyPressEvent(QKeyEvent *);
    QVariant	inputMethodQuery ( Qt::InputMethodQuery query ) const;
	void	inputMethodEvent(QInputMethodEvent * event);
	void	resizeEvent(QResizeEvent *event);
	//bool	eventFilter(QObject *obj, QEvent *event);
	void	focusInEvent ( QFocusEvent * event );
	void	focusOutEvent ( QFocusEvent * event );
public slots:
	void	undo();
	void	redo();
	void	cut(bool append = false);
	int		copy(bool bCut = false, bool append = false);
	void	paste();
private slots:
	void	onTimer();
signals:
	void	modifiedChanged() const;
	void	cursorPosChanged(int, int);
	void	doOutput(const QString &) const;
	void	escPressed();
	void	showMessage(const QString &, int timeout = 0) const;
	void	reloadRequest(EditView *);
	void	textSearched(const QString &);
	void	openFile(const QString &);
	void	gameFinished(bool, QString);
	void	textInserted(const QString &);		//	挿入後の位置
	void	textBackSpaced();
	void	textCopied(const QString &);
	void	boxSelectionModeChanged(bool);
	void	vertScrolled(int);		//	垂直スクロール
	void	horzScrolled(int);		//	垂直スクロール
	void	tagJump(const QString &, int);
	void	tagsJump(const QString &, const QString &, const QString &);
	void	outputViewClosed();
    void	updateUndoRedoEnabled();
	
private:
	bool	m_lineNumberVisible;
	bool	m_minMapDragging;
	bool	m_dispCursor;
	int		m_scrollX0;			//	水平方向スクロール（0 org カラム位置）
	int		m_scrollY0;
	int		m_fontWidth;
	int		m_fontHeight;
	int		m_lineHeight;		//	行高（in Pixel）
	int		m_lineNumDigits;	//	log EOF行番号
	int		m_lineNumWidth;		//	行番号表示幅
	int		m_lineNumAreaWidth;
	int		m_nViewLine;
	int		m_tmCounter;
	int		m_preeditWidth;			//	変換中文字列表示幅
	int		m_preeditPos;			//	変換位置
	std::vector<int>	m_delForVarPos;			//	for 変数削除位置
	MainWindow	*m_mainWindow;
	//int		m_preeditLine;			//	変換行（論理行番号）
	QString	m_selectedString;		//	変換開始時に選択されていた文字列
	QString	m_preeditString;		//	変換中文字列
	QTimer	m_timer;
	//TypeSettings	*m_typeSettings;		//	タイプ設定へのウィークポインタ
	//##TypeSettings	*m_jsTypeSettings;		//	JavaScriptタイプ設定へのウィークポインタ
	//##TypeSettings	*m_phpTypeSettings;		//	PHPタイプ設定へのウィークポインタ
	//	undone: マルチカーソル対応
	TextCursor	*m_textCursor;
	//TextCursor	*m_svTextCursor;
	//
	ViewLineMgr	*m_viewLineMgr;
	Document	*m_document;
	Buffer		*m_buffer;				//	ポイントのみで、非所有
	QFont		m_font;
	QFont		m_fontBold;
	QFont		m_fontMB;				//	マルチバイト用フォント
	QWidget		m_lineNumAreaWidget;	//	行番号表示用ウィジェット？
	//QWidget		m_textAreaWidget;		//	テキスト表示用ウィジェット
	//QPixmap		m_textAreaPixmap;		//	テキスト表示用 Pixmap
	//	ミニマップ関連、undone:そのうち Document に移動
	QWidget		m_minMapWidget;
	//double		m_mmScale;				//	1.0 未満であれば縮小されている
	//QPixmap		m_minMap;
	//
};
