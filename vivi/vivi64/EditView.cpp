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
	buffer()->clear();
	buffer()->insertText(0, (cwchar_t*)txt.data(), txt.size());
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
	m_font = QFont(typeSettings()->textValue(TypeSettings::FONT_NAME),
					typeSettings()->intValue(TypeSettings::FONT_SIZE));
	m_font.setKerning(false);
	setFont(m_font);
	m_fontBold = QFont(typeSettings()->textValue(TypeSettings::FONT_NAME),
					typeSettings()->intValue(TypeSettings::FONT_SIZE),
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
void EditView::mousePressEvent(QMouseEvent *event)
{
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	if( pnt.x() >= rct.width() - MINMAP_WIDTH ) {
		int nLines = rct.height() / m_lineHeight;
		m_scrollX0 = qMax(0, pnt.y() - nLines / 2);
    	m_scrollX0 = qMin(m_scrollX0, buffer()->lineCount());		//	undone: 折返し処理対応
		update();
	}
}
void EditView::mouseMoveEvent(QMouseEvent *event)
{
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	if( pnt.x() >= rct.width() - MINMAP_WIDTH ) {
		int nLines = rct.height() / m_lineHeight;
		m_scrollX0 = qMax(0, pnt.y() - nLines / 2);
    	m_scrollX0 = qMin(m_scrollX0, buffer()->lineCount());		//	undone: 折返し処理対応
		update();
	}
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
    	m_scrollX0 = qMin(m_scrollX0, buffer()->lineCount());		//	undone: 折返し処理対応
    	update();
    }
}
void EditView::keyPressEvent(QKeyEvent *)
{
}
void EditView::paintEvent(QPaintEvent *event)
{
	qDebug() << "lineCount = " << buffer()->lineCount();
	QPainter pt(this);
	auto rct = rect();
	qDebug() << "rect = " << rct;
	//	全体背景描画
	pt.setPen(Qt::transparent);
	QColor col = typeSettings()->color(TypeSettings::BACK_GROUND);
	pt.setBrush(col);
	pt.drawRect(rct);
	//	行番号部分背景描画
	col = typeSettings()->color(TypeSettings::LINENUM_BG);
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
	int limit = buffer()->lineCount() + (buffer()->isBlankEOFLine() ? 1 : 0);
	for (int ln = 1 + m_scrollX0; ln <= limit && py < rct.height(); ++ln, py+=m_lineHeight) {
		QString number = QString::number(ln);
		int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
		pt.drawText(px, py+m_fontHeight, number);
	}
}
void EditView::drawTextArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
	int px, py = DRAW_Y_OFFSET;
	bool inBlockComment = false;
	bool inLineComment = false;
	QString quotedText;
	for (int ln = m_scrollX0; ln < buffer()->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		inLineComment = false;		//	undone: 折返し行対応
		px = m_lineNumAreaWidth;
		auto startIX = buffer()->lineStartPosition(ln);
		auto lnsz = buffer()->lineSize(ln);
		drawLineText(pt, px, py+m_fontHeight, ln, startIX, lnsz, startIX+lnsz, inBlockComment, inLineComment, quotedText);
		if( !buffer()->isBlankEOFLine() && ln == buffer()->lineCount() - 1 ) {
			pt.setPen(typeSettings()->color(TypeSettings::EOF_MARK));
			pt.drawText(px, py+m_fontHeight, "[EOF]");
		}
	}
	if( buffer()->isBlankEOFLine() ) {
		pt.setPen(typeSettings()->color(TypeSettings::EOF_MARK));
		pt.drawText(m_lineNumAreaWidth, py+m_fontHeight, "[EOF]");
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
	int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int ix = 0;
	const int last = ls + vlnsz;
	const QString lineComment = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
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
		QColor col = typeSettings()->color(inBlockComment || inLineComment ? TypeSettings::COMMENT : TypeSettings::TEXT);
		auto wd = fm.width(token);
		if( inLineComment ) {
			col = typeSettings()->color(TypeSettings::COMMENT);
		} else if( inBlockComment ) {
			col = typeSettings()->color(TypeSettings::COMMENT);
			if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_END) ) {
				inBlockComment = false;
			}
		} else {
			switch( tkn.tokenType() ) {
			case ViewTokenizer::ALNUM:
				if( typeSettings()->isKeyWord1(token) ) {
					col = typeSettings()->color(TypeSettings::KEYWORD1);
					if( typeSettings()->boolValue(TypeSettings::KEYWORD1_BOLD) )
						pt.setFont(m_fontBold);
				} else if( typeSettings()->isKeyWord2(token) ) {
					col = typeSettings()->color(TypeSettings::KEYWORD2);
					if( typeSettings()->boolValue(TypeSettings::KEYWORD2_BOLD) )
						pt.setFont(m_fontBold);
				}
				//pt.setPen(typeSettings()->color(TypeSettings::TEXT));
				//pt.drawText(px, py, token);
				//px += fm.width(token);
				break;
			case ViewTokenizer::DIGITS:
				if( !inLineComment && !inBlockComment )
					col = typeSettings()->color(TypeSettings::DIGITS);
				//pt.setPen(typeSettings()->color(TypeSettings::DIGITS));
				//pt.drawText(px, py, token);
				//px += fm.width(token);
				break;
			case ViewTokenizer::QUOTED:
				if( !inLineComment && !inBlockComment )
					col = typeSettings()->color(TypeSettings::STRING);
				//pt.setPen(typeSettings()->color(TypeSettings::STRING));
				//pt.drawText(px, py, token);
				//px += fm.width(token);
				break;
			case ViewTokenizer::ZEN_SPACE:
				token = QChar(L'□');
				col = typeSettings()->color(TypeSettings::ZEN_SPACE);
				break;
			case ViewTokenizer::CTRL:
				if( token == "\t" ) {
					token = ">";
					col = typeSettings()->color(TypeSettings::TAB);
					//pt.setPen(typeSettings()->color(TypeSettings::TAB));
					//pt.drawText(px, py, ">");
					int clmn = (px - m_lineNumAreaWidth) / spcWidth;
					wd = (nTab - (clmn % nTab)) * spcWidth;
					//clmn += nTab - (clmn % nTab);
					//px = m_lineNumAreaWidth + clmn * spcWidth;
				} else {
					//pt.setPen(typeSettings()->color(TypeSettings::TEXT));
					//pt.drawText(px, py, token);
					//px += fm.width(token);
				}
				break;
			case ViewTokenizer::NEWLINE:
				col = typeSettings()->color(TypeSettings::NEWLINE);
				break;
			case ViewTokenizer::COMMENT:
				//if( !inBlockComment ) {
					if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_BEG) ) {
						col = typeSettings()->color(TypeSettings::COMMENT);
						inBlockComment = true;
					} else if( token == typeSettings()->textValue(TypeSettings::LINE_COMMENT) ) {
						inLineComment = true;
						col = typeSettings()->color(TypeSettings::COMMENT);
					}
				//}
				break;
			}
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
	if( buffer()->lineCount() > 10000 ) return;		//	最大1万行
	int ht = qMin(MAX_MINMAP_HEIGHT, buffer()->lineCount());
	m_mmScale = (double)ht / buffer()->lineCount();
	m_minMap = QPixmap(MINMAP_WIDTH, ht);
	auto ts = typeSettings();
	m_minMap.fill(ts->color(TypeSettings::BACK_GROUND));
	QPainter painter(&m_minMap);
	painter.fillRect(QRect(0, 0, MINMAP_LN_WD, ht), ts->color(TypeSettings::LINENUM_BG));
	//if( lineCount() > 10000 ) return;
	painter.setPen(ts->color(TypeSettings::TEXT));
	bool inBlockComment = false;
	for (int ln = 0; ln < buffer()->lineCount(); ++ln) {
		int p = buffer()->lineStartPosition(ln);
		int last= buffer()->lineStartPosition(ln+1);
		int px = MINMAP_LN_WD;
		if( buffer()->charAt(p) == '\t' ) {
			while (buffer()->charAt(p) == '\t') {
				++p;
				px += ts->intValue(TypeSettings::TAB_WIDTH);
			}
		} else {
			while (buffer()->charAt(p) == ' ') {
				++p;
				++px;
			}
		}
		if( p >= buffer()->size() || isNewLine(buffer()->charAt(p)) ) continue;
		while( last > p && isNewLine(buffer()->charAt(last - 1)) )
			--last;
		painter.drawLine(px, ln*m_mmScale, px + last - p, ln*m_mmScale);
	}
}
void EditView::drawMinMap(QPainter& pt)
{
	auto rct = rect();
	int nLines = rct.height() / m_lineHeight;
	int px = rct.width() - m_minMap.width();
	int py = 0;
	//
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	//pt.setBrush(QColor("lightgray"));
	pt.setBrush(typeSettings()->color(TypeSettings::BACK_GROUND));
	pt.setPen(Qt::transparent);
	pt.drawRect(rct);		//	背景（テキスト背景と同一）描画
	//
	pt.setOpacity(0.5);
	pt.drawPixmap(px, py, m_minMap);		//	テキストPixmap描画
	//
	pt.setOpacity(0.25);
	pt.setBrush(Qt::black);
	if( m_scrollX0 != 0 ) {
		rct.setHeight(m_scrollX0);
		pt.drawRect(rct);			//	現エリアより上部（前）描画
	}
	if( m_minMap.height() - (m_scrollX0+nLines) > 0 ) {
		rct.setY(m_scrollX0+nLines);
		rct.setHeight(m_minMap.height() - (m_scrollX0+nLines));
		pt.drawRect(rct);			//	現エリアより下部（後）描画
	}
	rct.setY(m_scrollX0);
	rct.setHeight(nLines);
	pt.setBrush(Qt::transparent);
	pt.setPen(Qt::red);
	pt.drawRect(rct);				//	現エリアに赤枠描画
}
