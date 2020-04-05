#pragma once

#include <QScrollArea>
#include <QFont>
//#include "typeSettings.h"
//#include "globalSettings.h"
class TypeSettings;
class GlobalSettings;
class Buffer;

class EditView : public QWidget		//QScrollArea
{
	Q_OBJECT
public:
	EditView();
	~EditView();
public:
	QString	typeName() const;
	TypeSettings	*typeSettings() { return m_typeSettings; }
	void	setLineNumberVisible(bool);
	void	setPlainText(const QString&);
protected:
	void	updateFont();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	updateLineNumberInfo();
protected:
	void	paintEvent(QPaintEvent *);
#if	0
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	wheelEvent(QWheelEvent * event);
	void	keyPressEvent(QKeyEvent *);
#endif
private:
	bool		m_lineNumberVisible;
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
	QWidget		*m_lineNumArea;
};
