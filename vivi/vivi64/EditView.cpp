#include <QPainter>
#include <QDebug>
#include "EditView.h"
#include "typeSettings.h"

EditView::EditView()
	: m_typeSettings(nullptr)
	, m_lineNumDigits(3)		//	初期値は3桁 1〜999
{
	m_typeSettings = new TypeSettings();
	updateFont();
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
	updateLineNumberInfo();
}
void EditView::updateLineNumberInfo()
{
		m_lineNumWidth = QFontMetrics(m_font).width('8')*(m_lineNumDigits+1);
		m_lineNumAreaWidth = QFontMetrics(m_font).width('8')*(m_lineNumDigits + 3);
}
void EditView::paintEvent(QPaintEvent *event)
{
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
	rct.setWidth(m_lineNumWidth);
	pt.drawRect(rct);
}
void EditView::drawLineNumbers()
{
}
