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
	int	viewLineOffsetToPx(int vln, int offset) const;
	void	setLineNumberVisible(bool);
	void	updateFont();
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
	void	onCursorPosChanged();
protected:
	void	drawLineNumbers();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	drawLineText(QPainter &, int &, int, int, pos_t, int, pos_t, bool&, bool&, QString&);
	void	drawMinMap(QPainter&);
	void	drawCursor(QPainter&);
	void	updateLineNumberInfo();
	void	onResized();
	//void	buildMinMap();
protected:
	void	paintEvent(QPaintEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	wheelEvent(QWheelEvent * event);
	void	keyPressEvent(QKeyEvent *);
	void	resizeEvent(QResizeEvent *event);
	//bool	eventFilter(QObject *obj, QEvent *event);
private slots:
	void	onTimer();
signals:
	void	cursorPosChanged(int, int);
	
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
	int		m_preeditPos;			//	変換位置
	int		m_tmCounter;
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
