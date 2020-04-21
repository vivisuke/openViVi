#pragma once

#include <QScrollArea>
#include <QFont>
#include <QTimer>
#include "../buffer/Buffer.h"
//#include "typeSettings.h"
//#include "globalSettings.h"
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
	EditView(Document *doc /*, TypeSettings* = nullptr*/);
	~EditView();
public:
	wchar_t	charAt(pos_t pos) const;
	size_t	bufferSize() const;
	pos_t	cursorPosition() const;
	int		EOFLine() const;
	QString	fullPathName() const;
	QString	typeName() const;
	TypeSettings	*typeSettings();	// { return m_typeSettings; }
	const TypeSettings	*typeSettings() const;	// { return m_typeSettings; }
	//const TypeSettings* cTypeSettings() { return (const TypeSettings*)m_typeSettings; }
	bool	isModified() const;		// { return m_modified; }
	int	viewLineOffsetToPx(int vln, int offset) const;
	void	setLineNumberVisible(bool);
	void	setPlainText(const QString&);
	//void	setTypeSettings(TypeSettings *);
	Document*	document() { return m_document; }
	const Document*	document() const { return m_document; }
	Buffer	*buffer() { return m_buffer; }
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
	bool	makeCursorInView(bool bQuarter = false);
	void	deleteText(pos_t pos, ssize_t sz = 1, bool BS = false);
	void	onCursorPosChanged();
	void	insertTextRaw(pos_t pos, const QString &);
	void	updateFont();
	bool	saveFile() const;
protected:
	void	resetCursorBlinking();
	void	drawLineNumbers();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	drawLineText(QPainter &, int &, int, int, pos_t, int, pos_t, bool&, bool&, QString&);
	void	drawPreeditString(QPainter&);
	void	drawCursor(QPainter&);
	void	drawLineCursor(QPainter &);
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
	
private:
	bool	m_lineNumberVisible;
	bool	m_minMapDragging;
	bool	m_dispCursor;
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
	QWidget		m_lineNumAreaWidget;
	//	ミニマップ関連、undone:そのうち Document に移動
	QWidget		m_minMapWidget;
	//double		m_mmScale;				//	1.0 未満であれば縮小されている
	//QPixmap		m_minMap;
	//
};
