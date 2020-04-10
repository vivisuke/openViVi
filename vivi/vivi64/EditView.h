#pragma once

#include <QScrollArea>
#include <QFont>
#include "../buffer/Buffer.h"
//#include "typeSettings.h"
//#include "globalSettings.h"
class TypeSettings;
class GlobalSettings;
class Buffer;

class EditView : public QWidget		//QScrollArea
{
	Q_OBJECT
public:
	EditView(Buffer *buffer, TypeSettings* = nullptr);
	~EditView();
public:
	QString	typeName() const;
	TypeSettings	*typeSettings() { return m_typeSettings; }
	//const TypeSettings* cTypeSettings() { return (const TypeSettings*)m_typeSettings; }
	int	viewLineOffsetToPx(int vln, int offset) const;
	void	setLineNumberVisible(bool);
	void	updateFont();
	void	setPlainText(const QString&);
	void	setTypeSettings(TypeSettings *);
protected:
	void	drawLineNumbers();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	drawLineText(QPainter &, int &, int, int, pos_t, int, pos_t, bool&, bool&, QString&);
	void	drawMinMap(QPainter&);
	void	updateLineNumberInfo();
	void	buildMinMap();
	Buffer	*buffer() { return m_buffer; }
protected:
	void	paintEvent(QPaintEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	wheelEvent(QWheelEvent * event);
	void	keyPressEvent(QKeyEvent *);
	bool	eventFilter(QObject *obj, QEvent *event);
private:
	bool	m_lineNumberVisible;
	bool	m_minMapDragging;
	int		m_scrollX0;
	int		m_fontWidth;
	int		m_fontHeight;
	int		m_lineHeight;		//	行高（in Pixel）
	int		m_lineNumDigits;	//	log EOF行番号
	int		m_lineNumWidth;		//	行番号表示幅
	int		m_lineNumAreaWidth;
	int		m_nViewLine;
	int		m_preeditPos;			//	変換位置
	TypeSettings	*m_typeSettings;		//	タイプ設定へのウィークポインタ
	TypeSettings	*m_jsTypeSettings;		//	JavaScriptタイプ設定へのウィークポインタ
	TypeSettings	*m_phpTypeSettings;		//	PHPタイプ設定へのウィークポインタ
	//TextCursor	*m_textCursor;
	//TextCursor	*m_svTextCursor;
	Buffer		*m_buffer;
	QFont		m_font;
	QFont		m_fontBold;
	QFont		m_fontMB;				//	マルチバイト用フォント
	QWidget		m_lineNumArea;
	//	ミニマップ関連、undone:そのうち Document に移動
	double		m_mmScale;				//	1.0 未満であれば縮小されている
	QPixmap		m_minMap;
	//
};
