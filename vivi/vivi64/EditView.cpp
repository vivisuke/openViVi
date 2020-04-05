#include <QPainter>
#include <QDebug>
#include "EditView.h"
#include "typeSettings.h"
#include "../buffer/Buffer.h"

#define		DRAW_Y_OFFSET		2
#define		MINMAP_WIDTH			80

EditView::EditView()
	: m_typeSettings(nullptr)
	, m_lineNumDigits(3)		//	初期値は3桁 1〜999
{
	m_typeSettings = new TypeSettings();
	m_lineNumberVisible = m_typeSettings->boolValue(TypeSettings::VIEW_LINENUM);
	updateFont();
	m_buffer = new Buffer();
}
EditView::~EditView()
{
	delete m_buffer;
}
void EditView::setPlainText(const QString& txt)
{
	m_buffer->clear();
	m_buffer->insertText(0, (cwchar_t*)txt.data(), txt.size());
	update();
}
void EditView::updateFont()
{
	m_font = QFont(m_typeSettings->textValue(TypeSettings::FONT_NAME),
					m_typeSettings->intValue(TypeSettings::FONT_SIZE));
	m_font.setKerning(false);
	setFont(m_font);
	m_fontBold = QFont(m_typeSettings->textValue(TypeSettings::FONT_NAME),
					m_typeSettings->intValue(TypeSettings::FONT_SIZE),
					QFont::Bold);
	m_fontBold.setKerning(false);
	//
	m_fontWidth = QFontMetrics(m_font).width('8');
	m_fontHeight = QFontInfo(m_font).pixelSize();
	m_lineHeight = (int)(m_fontHeight * 1.2);
	//
	updateLineNumberInfo();
}
void EditView::updateLineNumberInfo()
{
	if( m_lineNumberVisible ) {
		m_lineNumWidth = QFontMetrics(m_font).width('8')*(m_lineNumDigits+1);
		m_lineNumAreaWidth = QFontMetrics(m_font).width('8')*(m_lineNumDigits + 3);
	} else {
		m_lineNumWidth = 0;
		m_lineNumAreaWidth = QFontMetrics(m_font).width('8')*2;
	}
}
QString EditView::typeName() const
{
	return m_typeSettings->name();
}
void EditView::setLineNumberVisible(bool b)
{
	if( b == m_lineNumberVisible ) return;
	m_lineNumberVisible = b;
	updateLineNumberInfo();
	update();
}
void EditView::paintEvent(QPaintEvent *event)
{
	qDebug() << "lineCount = " << m_buffer->lineCount();
	QPainter pt(this);
	auto rct = rect();
	qDebug() << "rect = " << rct;
	//	全体背景描画
	pt.setPen(Qt::transparent);
	QColor col = m_typeSettings->color(TypeSettings::BACK_GROUND);
	pt.setBrush(col);
	pt.drawRect(rct);
	//	行番号部分背景描画
	col = m_typeSettings->color(TypeSettings::LINENUM_BG);
	pt.setBrush(col);
	rct.setWidth(m_lineNumAreaWidth);
	pt.drawRect(rct);
	//
	drawLineNumberArea(pt);		//	行番号エリア描画
	drawTextArea(pt);					//	テキストエイア描画
	drawMiniMap(pt);					//	ミニマップ描画
}
void EditView::drawLineNumberArea(QPainter& pt)
{
	pt.setPen(Qt::black);
	int py = DRAW_Y_OFFSET;
	for (int ln = 1; ln <= m_buffer->lineCount(); ++ln, py+=m_lineHeight) {
		QString number = QString::number(ln);
		int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
		pt.drawText(px, py+m_fontHeight, number);
	}
}
void EditView::drawTextArea(QPainter& pt)
{
	pt.setPen(Qt::black);
	int py = DRAW_Y_OFFSET;
	for (int ln = 1; ln <= m_buffer->lineCount(); ++ln, py+=m_lineHeight) {
		auto startIX = m_buffer->lineStartPosition(ln-1);
		auto lnsz = m_buffer->lineSize(ln-1);
		QString txt;
		for (int i = 0; i < lnsz; ++i) {
			txt += m_buffer->operator[](startIX+i);
		}
		pt.drawText(m_lineNumAreaWidth, py+m_fontHeight, txt);
	}
}
void EditView::drawMiniMap(QPainter& pt)
{
	auto rct = rect();
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	pt.setBrush(QColor("lightgray"));
	pt.setPen(Qt::transparent);
	pt.drawRect(rct);
}
