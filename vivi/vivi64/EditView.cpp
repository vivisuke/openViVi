#include <QtGui>
//#include <QPainter>
#include <QDebug>
#include "EditView.h"
#include "ViewTokenizer.h"
#include "typeSettings.h"
#include "../buffer/Buffer.h"

#define		DRAW_Y_OFFSET		2
#define		MINMAP_LN_WD		4			//	行番号部分幅
#define		MINMAP_WIDTH		80
#define		MAX_MINMAP_HEIGHT	10000		//	ピックスマップ最大高さ

//----------------------------------------------------------------------
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
//----------------------------------------------------------------------
EditView::EditView(TypeSettings* typeSettings)
	: m_typeSettings(nullptr)
	, m_lineNumDigits(3)		//	初期値は3桁 1〜999
	, m_scrollX0(0)
{
	m_typeSettings = typeSettings == nullptr ? new TypeSettings() : typeSettings;
	qDebug() << "typeSettings type = " << m_typeSettings->name();
	m_lineNumberVisible = m_typeSettings->boolValue(TypeSettings::VIEW_LINENUM);
	updateFont();
	m_buffer = new Buffer();
	buildMinMap();
}
EditView::~EditView()
{
	delete m_buffer;
}
void EditView::setPlainText(const QString& txt)
{
	m_buffer->clear();
	m_buffer->insertText(0, (cwchar_t*)txt.data(), txt.size());
	buildMinMap();
	update();
}
int EditView::viewLineOffsetToPx(int vln, int offset) const
{
	Q_ASSERT(0);
	return 0;
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
void EditView::mousePressEvent(QMouseEvent *)
{
}
void EditView::mouseMoveEvent(QMouseEvent *)
{
}
void EditView::mouseReleaseEvent(QMouseEvent *)
{
}
void EditView::mouseDoubleClickEvent(QMouseEvent *)
{
}
void EditView::wheelEvent(QWheelEvent * event)
{
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    qDebug() << "numPixels = " << numPixels;
    qDebug() << "numDegrees = " << numDegrees;
    if (!numDegrees.isNull()) {
    	QPoint numSteps = numDegrees / 15;
    	if( (m_scrollX0 -= numSteps.y()*3) < 0 )
    		m_scrollX0 = 0;
    	update();
    }
}
void EditView::keyPressEvent(QKeyEvent *)
{
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
	drawMinMap(pt);					//	ミニマップ描画
}
void EditView::drawLineNumberArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
	int py = DRAW_Y_OFFSET;
	for (int ln = 1 + m_scrollX0; ln <= m_buffer->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		QString number = QString::number(ln);
		int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
		pt.drawText(px, py+m_fontHeight, number);
	}
}
void EditView::drawTextArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
	int py = DRAW_Y_OFFSET;
	bool inBlockComment = false;
	bool inLineComment = false;
	QString quotedText;
	for (int ln = m_scrollX0; ln < m_buffer->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		inLineComment = false;		//	undone: 折返し行対応
		int px = m_lineNumAreaWidth;
		auto startIX = m_buffer->lineStartPosition(ln);
		auto lnsz = m_buffer->lineSize(ln);
		drawLineText(pt, px, py+m_fontHeight, ln, startIX, lnsz, startIX+lnsz, inBlockComment, inLineComment, quotedText);
#if	0
		QString txt;
		for (int i = 0; i < lnsz; ++i) {
			txt += m_buffer->operator[](startIX+i);
		}
		pt.drawText(m_lineNumAreaWidth, py+m_fontHeight, txt);
#endif
	}
}
//	１行表示
void EditView::drawLineText(QPainter &pt, int &px, int py,
												int ln,			//	論理行番号, 0 org
												pos_t ls,			//	表示行先頭位置
												int vlnsz,		//	表示行サイズ
												pos_t nxdls,		//	次の論理行先頭位置
												bool &inBlockComment,
												bool &inLineComment,
												QString &quotedText)
{
	QFontMetrics fm(m_font);
	QFontMetrics fmBold(m_fontBold);
	const auto spcWidth = fm.width("8");
	int nTab = m_typeSettings->intValue(TypeSettings::TAB_WIDTH);
	int ix = 0;
	const int last = ls + vlnsz;
	const QString lineComment = m_typeSettings->textValue(TypeSettings::LINE_COMMENT);
	//const QString lineComment = "//";		//	暫定コード
	ViewTokenizer tkn(typeSettings(), buffer(), ls, vlnsz, nxdls);
	tkn.setInLineComment(inLineComment);
	tkn.setInBlockComment(inBlockComment);
	QString token = tkn.nextToken();
	while( !token.isEmpty() ) {
		if( tkn.tokenix() + token.size() > last )
			token = token.left(last - tkn.tokenix());
		qDebug() << "type = " << tkn.tokenType() << ", token = " << token;
		pt.setFont(m_font);
		QColor col = m_typeSettings->color(inBlockComment || inLineComment ? TypeSettings::COMMENT : TypeSettings::TEXT);
		auto wd = fm.width(token);
		switch( tkn.tokenType() ) {
		case ViewTokenizer::ALNUM:
			if( m_typeSettings->isKeyWord1(token) ) {
				col = m_typeSettings->color(TypeSettings::KEYWORD1);
				if( m_typeSettings->boolValue(TypeSettings::KEYWORD1_BOLD) )
					pt.setFont(m_fontBold);
			} else if( m_typeSettings->isKeyWord2(token) ) {
				col = m_typeSettings->color(TypeSettings::KEYWORD2);
				if( m_typeSettings->boolValue(TypeSettings::KEYWORD2_BOLD) )
					pt.setFont(m_fontBold);
			}
			//pt.setPen(m_typeSettings->color(TypeSettings::TEXT));
			//pt.drawText(px, py, token);
			//px += fm.width(token);
			break;
		case ViewTokenizer::DIGITS:
			if( !inLineComment && !inBlockComment )
				col = m_typeSettings->color(TypeSettings::DIGITS);
			//pt.setPen(m_typeSettings->color(TypeSettings::DIGITS));
			//pt.drawText(px, py, token);
			//px += fm.width(token);
			break;
		case ViewTokenizer::QUOTED:
			if( !inLineComment && !inBlockComment )
				col = m_typeSettings->color(TypeSettings::STRING);
			//pt.setPen(m_typeSettings->color(TypeSettings::STRING));
			//pt.drawText(px, py, token);
			//px += fm.width(token);
			break;
		case ViewTokenizer::CTRL:
			if( token == "\t" ) {
				token = ">";
				col = m_typeSettings->color(TypeSettings::TAB);
				//pt.setPen(m_typeSettings->color(TypeSettings::TAB));
				//pt.drawText(px, py, ">");
				int clmn = (px - m_lineNumAreaWidth) / spcWidth;
				wd = (nTab - (clmn % nTab)) * spcWidth;
				//clmn += nTab - (clmn % nTab);
				//px = m_lineNumAreaWidth + clmn * spcWidth;
			} else {
				//pt.setPen(m_typeSettings->color(TypeSettings::TEXT));
				//pt.drawText(px, py, token);
				//px += fm.width(token);
			}
			break;
		case ViewTokenizer::NEWLINE:
			col = m_typeSettings->color(TypeSettings::NEWLINE);
			break;
		case ViewTokenizer::COMMENT:
			if( !inBlockComment ) {
				if( token == m_typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG) ) {
					col = m_typeSettings->color(TypeSettings::COMMENT);
					inBlockComment = true;
				} else if( token == m_typeSettings->textValue(TypeSettings::LINE_COMMENT) ) {
					inLineComment = true;
					col = m_typeSettings->color(TypeSettings::COMMENT);
				}
			} else {
				col = m_typeSettings->color(TypeSettings::COMMENT);
				if( token == m_typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END) ) {
					inBlockComment = false;
				}
			}
			break;
#if	0
		case ViewTokenizer::OTHER:
			if( !inLineComment && !lineComment.isEmpty() && token.startsWith(lineComment) ) {
				inLineComment = true;
				col = m_typeSettings->color(TypeSettings::COMMENT);
			}
			//else
			//	pt.setPen(m_typeSettings->color(TypeSettings::TEXT));
			//pt.drawText(px, py, token);
			//px += fm.width(token);
			break;
#endif
		}
		pt.setPen(col);
		pt.drawText(px, py, token);
		px += wd;
		//
		token = tkn.nextToken();
	}
}
void EditView::buildMinMap()
{
	if( m_buffer->lineCount() > 10000 ) return;		//	最大1万行
	int ht = qMin(MAX_MINMAP_HEIGHT, m_buffer->lineCount());
	m_mmScale = (double)ht / m_buffer->lineCount();
	m_minMap = QPixmap(MINMAP_WIDTH, ht);
	auto ts = m_typeSettings;
	m_minMap.fill(ts->color(TypeSettings::BACK_GROUND));
	QPainter painter(&m_minMap);
	painter.fillRect(QRect(0, 0, MINMAP_LN_WD, ht), ts->color(TypeSettings::LINENUM_BG));
	//if( lineCount() > 10000 ) return;
	painter.setPen(ts->color(TypeSettings::TEXT));
	bool inBlockComment = false;
	for (int ln = 0; ln < m_buffer->lineCount(); ++ln) {
		int p = m_buffer->lineStartPosition(ln);
		int last= m_buffer->lineStartPosition(ln+1);
		int px = MINMAP_LN_WD;
		if( m_buffer->charAt(p) == '\t' ) {
			while (m_buffer->charAt(p) == '\t') {
				++p;
				px += ts->intValue(TypeSettings::TAB_WIDTH);
			}
		} else {
			while (m_buffer->charAt(p) == ' ') {
				++p;
				++px;
			}
		}
		if( p >= m_buffer->size() || isNewLine(m_buffer->charAt(p)) ) continue;
		while( last > p && isNewLine(m_buffer->charAt(last - 1)) )
			--last;
		painter.drawLine(px, ln*m_mmScale, px + last - p, ln*m_mmScale);
	}
}
void EditView::drawMinMap(QPainter& pt)
{
	auto rct = rect();
	int px = rct.width() - m_minMap.width();
	int py = 0;
	//
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	//pt.setBrush(QColor("lightgray"));
	pt.setBrush(m_typeSettings->color(TypeSettings::BACK_GROUND));
	pt.setPen(Qt::transparent);
	pt.drawRect(rct);
	//
	pt.setOpacity(0.5);
	pt.drawPixmap(px, py, m_minMap);
}
