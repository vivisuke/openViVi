#include <QtGui>
//#include <QPainter>
#include <QDebug>
#include "Document.h"
#include "EditView.h"
#include "ViewTokenizer.h"
#include "typeSettings.h"
#include "TextCursor.h"
#include "viewLineMgr.h"
#include "../buffer/Buffer.h"
#include "../buffer/UTF16.h"

#define		DRAW_Y_OFFSET		2
#define		MINMAP_LN_WD		4			//	行番号部分幅
#define		MINMAP_WIDTH		80
#define		MAX_MINMAP_HEIGHT	10000		//	ピックスマップ最大高さ

#define		TM_FPS			50		//	50FPS
#define		HS_MARGIN		60
#define		PAI				3.1415926535
#define		MAX_HSCB		(1024*40)

#define		CURSOR_WD		2

//----------------------------------------------------------------------
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
//----------------------------------------------------------------------
EditView::EditView(Document *doc /*, TypeSettings* typeSettings*/)
	: m_document(doc)
	, m_buffer(doc->buffer())
	//, m_buffer(buffer)
	//, m_typeSettings(nullptr)
	, m_lineNumDigits(3)		//	初期値は3桁 1〜999
	,m_scrollY0(0)
	, m_minMapDragging(false)
	, m_dispCursor(true)
	//, m_viewLineMgr(new ViewLineMgr(this))
{
	Q_ASSERT(doc != nullptr);
	setAttribute(Qt::WA_InputMethodEnabled);
	//m_typeSettings = typeSettings == nullptr ? new TypeSettings() : typeSettings;
	auto typeSettings = doc->typeSettings();
	qDebug() << "typeSettings type = " << typeSettings->name();
	m_lineNumberVisible = typeSettings->boolValue(TypeSettings::VIEW_LINENUM);
	m_textCursor = new TextCursor(this);
	//m_buffer = new Buffer();
	//m_lineNumArea = new QWidget(this);
	m_lineNumAreaWidget.setParent(this);
	m_lineNumAreaWidget.setCursor(Qt::ArrowCursor);
	//m_lineNumAreaWidget.installEventFiler(this);
	m_viewLineMgr = new ViewLineMgr(this);
	//	ミニマップ
	m_minMapWidget.setParent(this);
	m_minMapWidget.setCursor(Qt::ArrowCursor);
	//m_minMapWidget.setGeometry();
	document()->buildMinMap();
	onResized();
	//
	updateFont();
	//
	setCursor(Qt::IBeamCursor);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_tmCounter = TM_FPS / 2;
	m_timer.start(1000/TM_FPS);
}
EditView::~EditView()
{
	delete m_buffer;
	//delete m_viewLineMgr;
}
void EditView::onResized()
{
	auto rct = rect();
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	m_minMapWidget.setGeometry(rct);
	//
	rct.setX(0);
	rct.setWidth(m_lineNumAreaWidth);
	m_lineNumAreaWidget.setGeometry(rct);
}
void EditView::setPlainText(const QString& txt)
{
	buffer()->clear();
	buffer()->insertText(0, (cwchar_t*)txt.data(), txt.size());
	document()->buildMinMap();
	update();
}
TypeSettings *EditView::typeSettings()
{
	return document()->typeSettings();
}
const TypeSettings *EditView::typeSettings() const
{
	return document()->typeSettings();
}
#if	0
void EditView::setTypeSettings(TypeSettings *typeSettings)
{
	m_typeSettings = typeSettings;
}
#endif
int EditView::viewLineOffsetToPx(int vln, int offset) const
{
	auto start = viewLineMgr()->viewLineStartPosition(vln);
	auto next = viewLineMgr()->viewLineStartPosition(vln + 1);
	return textWidth(start, offset, next);
	//Q_ASSERT(0);
	//return 0;
}
int EditView::positionToLine(pos_t pos) const
{
	return document()->positionToLine(pos);
}
int EditView::docLineToViewLine(int dln) const
{
	return dln;
}
int EditView::viewLineToDocLine(int vln) const
{
	return vln;
}
int EditView::viewLineToDocLine(int vln, int &offset) const
{
	offset = 0;
	return vln;
}
int EditView::viewLineStartPosition(int vln) const
{
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	return lineStartPosition(dln) + offset;
}
int EditView::lineStartPosition(int ln) const
{
	return document()->lineStartPosition(ln);
}
void EditView::updateFont()
{
	//	暫定コード
	m_fontMB = QFont("ＭＳ ゴシック",
					typeSettings()->intValue(TypeSettings::FONT_SIZE));
	m_fontMB.setFixedPitch(true);
	m_fontMB.setKerning(false);
	//
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
	QRect rct = /*viewport()->*/rect();
	auto wd = rct.width();
	//rct.moveTo(2, 2);
	rct.setWidth(m_lineNumAreaWidth);
	m_lineNumAreaWidget.setGeometry(rct);
#if	0
	//
	rct.setX(wd - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	m_minMapWidget.setGeometry(rct);
#endif
}
wchar_t EditView::charAt(pos_t pos) const
{
	return document()->charAt(pos);
}
pos_t EditView::cursorPosition() const
{
	return m_textCursor->position();
}
int EditView::EOFLine() const
{
	return m_viewLineMgr->EOFLine();
}
size_t EditView::bufferSize() const
{
	return buffer()->size();
}
void EditView::onCursorPosChanged()
{
	resetCursorBlinking();
	//m_dispCursor = true;
	//m_tmCounter = TM_FPS / 2;
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	emit cursorPosChanged(ln+1, pos - lineStartPosition(ln));
}
//	return:	スクロールしたかどうか
bool EditView::makeCursorInView(bool bQuarter)
{
	pos_t pos = m_textCursor->position();
	int dln = positionToLine(pos);
	int vln = m_textCursor->viewLine();
#if	0
	if( viewLineToDocLine(vln) != dln ) {	//	折畳まれている場合
		expand(vln);
		m_textCursor->updateViewLine();
		vln = m_textCursor->viewLine();
	}
#endif
	bool scrolled = false;
	if( m_textCursor->position() > bufferSize() )
		m_textCursor->movePosition(TextCursor::END_DOC);
	//int nLine = viewport()->rect().height() / lineHeight();		//	表示可能行数
	int nLine = rect().height() / lineHeight();		//	表示可能行数
	int anchor = m_textCursor->anchor();
#if	0
	if( m_viewLineMgr->isLineBreakMode() ) {
		//	表示範囲を折り返し処理
		bool lineBreaked = false;
		int v = qMax(0, vln - nLine);
		while ( v < viewLineCount() && viewLineStartPosition(v) <= pos ) {
			if( viewLineMgr()->ensureLineBreak(v++) )
				lineBreaked = true;
		}
		if( lineBreaked ) {
			m_textCursor->setPosition(pos);
			if( anchor != pos )
				m_textCursor->setAnchor(anchor);
			vln = m_textCursor->viewLine();
		}
	}
#endif
	//int ln = m_doc->positionToLine(pos);
	int vpos = m_scrollY0;	//verticalScrollBar()->value();
	if( vln >= vpos && vln < vpos + nLine ) {
	} else {
		if (nLine >= EOFLine() ) {		//	全行を表示可能な場合
			//verticalScrollBar()->setValue(0);
			m_scrollY0 = 0;
			return true;
		} else if( bQuarter ) {
			//verticalScrollBar()->setValue(vln - nLine / 4);
			m_scrollY0 = vln - nLine / 4;
			scrolled = true;
		} else {
			if( vln < vpos ) {
				//verticalScrollBar()->setValue(vln);
				m_scrollY0 = vln;
			} else {
				//int nLine = rect().height() / lineHeight();
				if( vln >= vpos + nLine ) {
					if( vln - (vpos + nLine) <= 5 ) {		//	直ぐ下の場合
						//verticalScrollBar()->setValue(vln - nLine + 1);
						m_scrollY0 = vln - nLine + 1;
					} else {
						//verticalScrollBar()->setValue(vln - nLine / 4);
						m_scrollY0 = vln - nLine / 4;
					}
					scrolled = true;
				}
			}
		}
	}
	//qDebug() << verticalScrollBar()->value();
#if	0
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
	int hpos = horizontalScrollBar()->value();
	int wd = viewport()->rect().width() /*- m_lineNumAreaWidth*/;
	if( px < hpos ) {
		horizontalScrollBar()->setValue(qMax(0, px - HS_MARGIN));
		scrolled = true;
	} else if( px >= hpos + wd - HS_MARGIN / 2 ) {
		horizontalScrollBar()->setValue(px - wd + HS_MARGIN);
		scrolled = true;
	}
#endif
	return scrolled;
}
QString EditView::typeName() const
{
	return document()->typeName();
}
QString EditView::fullPathName() const
{
	return document()->fullPathName();
}
void EditView::setLineNumberVisible(bool b)
{
	if( b == m_lineNumberVisible ) return;
	m_lineNumberVisible = b;
	updateLineNumberInfo();
	update();
}
void EditView::resetCursorBlinking()
{
	m_dispCursor = true;
	m_tmCounter = TM_FPS / 2;
	m_timer.start(1000/TM_FPS);
}
void EditView::onTimer()
{
	//qDebug() << "onTimer()";
	if( hasFocus() && --m_tmCounter < 0 ) {
		m_dispCursor = !m_dispCursor;
		m_tmCounter = TM_FPS / 2;
		update();
	}
}
#if	0
bool EditView::eventFilter(QObject *obj, QEvent *event)
{
#if	0
	if( obj == &m_lineNumAreaWidget ) {
		if( event->type() == QEvent::Paint ) {
			drawLineNumbers();
			return true;
		}
		if( event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			lineNumberMousePressed(mouseEvent);
			return true;
		}
		if( event->type() == QEvent::MouseMove) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			lineNumberMouseMoved(mouseEvent);
			return true;
		}
		if( event->type() == QEvent::ContextMenu) {
			QContextMenuEvent *cmEvent = static_cast<QContextMenuEvent*>(event);
			contextMenuEvent(cmEvent);
			return true;
		}
		if( event->type() == QEvent::Wheel) {
			QWheelEvent *wEvent = static_cast<QWheelEvent*>(event);
			wheelEvent(wEvent);
			return true;
		}
	}
#endif
	return false;
}
#endif
void EditView::resizeEvent(QResizeEvent *event)
{
	onResized();
}
void EditView::mousePressEvent(QMouseEvent *event)
{
	setFocus();
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	if( pnt.x() >= rct.width() - MINMAP_WIDTH ) {
		m_minMapDragging = true;
		double scale = qMin(1.0, (double)rct.height() / document()->minMap().height());
		int nLines = rct.height() / m_lineHeight;
		m_scrollY0 = qMax(0, (int)(pnt.y() / scale) - nLines / 2);
    	m_scrollY0 = qMin(m_scrollY0, buffer()->lineCount());		//	undone: 折返し処理対応
		update();
	}
}
void EditView::mouseMoveEvent(QMouseEvent *event)
{
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	//if( pnt.x() >= rct.width() - MINMAP_WIDTH )
	if( m_minMapDragging )
	{
		double scale = qMin(1.0, (double)rct.height() / document()->minMap().height());
		int nLines = rct.height() / m_lineHeight;
		m_scrollY0 = qMax(0, (int)(pnt.y()/scale) - nLines / 2);
    	m_scrollY0 = qMin(m_scrollY0, buffer()->lineCount());		//	undone: 折返し処理対応
		update();
	}
}
void EditView::mouseReleaseEvent(QMouseEvent *)
{
	m_minMapDragging = false;
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
    	if( (m_scrollY0 -= numSteps.y()*3) < 0 )
    		m_scrollY0 = 0;
    	m_scrollY0 = qMin(m_scrollY0, buffer()->lineCount());		//	undone: 折返し処理対応
    	update();
    }
}
void EditView::keyPressEvent(QKeyEvent *event)
{
	auto rct = rect();
	int nLines = rct.height() / m_lineHeight;
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	int mvmd = shift ? TextCursor::KEEP_ANCHOR : TextCursor::MOVE_ANCHOR;
	QString text = event->text();
	switch( event->key() ) {
	case Qt::Key_Left:
		if( ctrl )
			m_textCursor->movePosition(TextCursor::PREV_WORD, mvmd);
		else {
			m_textCursor->movePosition(TextCursor::LEFT, mvmd);
		}
		break;
	case Qt::Key_Right:
		if( ctrl )
			m_textCursor->movePosition(TextCursor::NEXT_WORD, mvmd);
		else {
			m_textCursor->movePosition(TextCursor::RIGHT, mvmd);
		}
		break;
	case Qt::Key_Up:
		m_textCursor->movePosition(TextCursor::UP, mvmd);
		break;
	case Qt::Key_Down:
		m_textCursor->movePosition(TextCursor::DOWN, mvmd);
		break;
	case Qt::Key_Home:
		if( ctrl ) {
			m_textCursor->movePosition(TextCursor::BEG_DOC, mvmd);
			m_scrollY0 = 0;		//	暫定コード
		} else {
			m_textCursor->movePosition(TextCursor::HOME_LINE, mvmd);
		}
		break;
	case Qt::Key_End:
		if( ctrl ) {
			m_textCursor->movePosition(TextCursor::END_DOC, mvmd);
			m_scrollY0 = buffer()->lineCount();		//	暫定コード
		} else {
			m_textCursor->movePosition(TextCursor::END_LINE, mvmd);
		}
		break;
	case Qt::Key_PageUp:
		m_scrollY0 = qMax(0, m_scrollY0 - nLines);		//	暫定コード
		m_textCursor->movePosition(TextCursor::UP, mvmd, nLines);
		break;
	case Qt::Key_PageDown:
		m_scrollY0 = qMin(buffer()->lineCount(), m_scrollY0 + nLines);		//	暫定コード
		m_textCursor->movePosition(TextCursor::DOWN, mvmd, nLines);
		break;
	case Qt::Key_Delete:
		onDelete(ctrl, shift, alt);
		break;
	case Qt::Key_Backspace:
			onBackSpace(ctrl, shift, alt);
			break;
#if	0
		if( mainWindow()->viEngine()->mode() != Mode::COMMAND ) {
			mainWindow()->viEngine()->onBackSpace();
			onBackSpace(ctrl, shift, alt);
			break;
		}
#endif
		//	下にスルー
	default: {
		if( text.isEmpty() )
			return;
		if( text[0].unicode() == 0x08 && ctrl ) {	//	Ctrl + H
			onBackSpace(false, shift, alt);
			break;
		}
	ins:
		insertTextSub(text, ctrl, shift, alt);
		//##if( isModified() != im )
		//##	emit modifiedChanged();
		makeCursorInView();
		resetCursorBlinking();
		update();
		return;
	}
	}
	onCursorPosChanged();
	makeCursorInView();
	update();
}
QVariant EditView::inputMethodQuery( Qt::InputMethodQuery query ) const
{
	if( query == Qt::ImMicroFocus ) {
			pos_t pos = /*m_toDeleteIMEPreeditText ? m_preeditPos :*/ m_textCursor->position();
			int vln = m_textCursor->viewLine();
			int x = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) /*- horizontalScrollBar()->value()*/;
			int y = (vln - m_scrollY0) * lineHeight();
			return QVariant(QRect(x + m_lineNumAreaWidth, y, 2, lineHeight() + 4));
	}
	return QWidget::inputMethodQuery(query);
}
void EditView::inputMethodEvent(QInputMethodEvent * event)
{
	const QString &text = event->commitString();
	if( !text.isEmpty() ) {		//	■ IME入力が確定した場合
		m_textCursor->insertText(text);
	}
}
void EditView::paintEvent(QPaintEvent *event)
{
	//qDebug() << "lineCount = " << buffer()->lineCount();
	QPainter pt(this);
	auto rct = rect();
	//qDebug() << "rect = " << rct;
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
	drawTextArea(pt);			//	テキストエイア描画
	drawMinMap(pt);				//	ミニマップ描画
	drawCursor(pt);				//	テキストカーソル表示
}
void EditView::drawLineNumberArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
	int py = 0 /*DRAW_Y_OFFSET*/;
	int limit = buffer()->lineCount() + (buffer()->isBlankEOFLine() ? 1 : 0);
	for (int ln = 1 + m_scrollY0; ln <= limit && py < rct.height(); ++ln, py+=m_lineHeight) {
		QString number = QString::number(ln);
		int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
		pt.drawText(px, py+m_fontHeight, number);
	}
}
#if	0
void EditView::drawLineNumbers()
{
}
#endif
void EditView::drawTextArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
	int px, py = 0 /*DRAW_Y_OFFSET*2*/;
	bool inBlockComment = false;
	bool inLineComment = false;
	QString quotedText;
	for (int ln = m_scrollY0; ln < buffer()->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
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
//
//			
void EditView::drawLineText(QPainter &pt, int &px,
												int py,			//	ベースライン位置
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
	QFontMetrics fmMB(m_fontMB);
	const auto descent = fm.descent();
	const auto chWidth = m_fontWidth;		//fm.width(QString("8"));
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
		//qDebug() << "type = " << tkn.tokenType() << ", token = " << token;
		bool bold = false;
		//pt.setFont(m_font);
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
					bold = typeSettings()->boolValue(TypeSettings::KEYWORD1_BOLD);
					//if( typeSettings()->boolValue(TypeSettings::KEYWORD1_BOLD) )
					//	pt.setFont(m_fontBold);
				} else if( typeSettings()->isKeyWord2(token) ) {
					col = typeSettings()->color(TypeSettings::KEYWORD2);
					bold = typeSettings()->boolValue(TypeSettings::KEYWORD2_BOLD);
					//if( typeSettings()->boolValue(TypeSettings::KEYWORD2_BOLD) )
					//	pt.setFont(m_fontBold);
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
					int clmn = (px - m_lineNumAreaWidth) / chWidth;
					wd = (nTab - (clmn % nTab)) * chWidth;
					//clmn += nTab - (clmn % nTab);
					//px = m_lineNumAreaWidth + clmn * chWidth;
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
		if( !token.isEmpty() ) {
			pt.setPen(col);
			if( bold ) {
				pt.setFont(m_fontBold);
				pt.drawText(px, py, token);
			} else if( token[0] < 0x100 ) {
				pt.setFont(m_font);
				pt.drawText(px, py, token);
			} else {
				auto x = px;
				for (int i = 0; i != token.size(); ++i) {
					QString txt = token[i];
					pt.drawText(x, py-m_fontHeight+descent, chWidth*2, m_fontHeight, Qt::AlignHCenter|Qt::AlignBottom, txt);
					x += chWidth * 2;
				}
				wd = chWidth * 2 * token.size();
#if	0
				pt.setFont(m_fontMB);
				wd = fmMB.width(token);
#endif
			}
			//pt.drawText(px, py, token);
		}
		px += wd;
		//
		token = tkn.nextToken();
	}
}
void EditView::drawMinMap(QPainter& pt)
{
	QPixmap& minMap = document()->minMap();
	auto rct = rect();
	int nLines = rct.height() / m_lineHeight;
	int px = rct.width() - minMap.width();
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
	//	undone: minMap 高さがビュー高さより高い場合は、縮小表示
	double scale = 1.0;
	if( minMap.height() <= rct.height() )	//	ミニマップが全部表示できる場合
		pt.drawPixmap(px, py, minMap);		//	テキストPixmap描画
	else {
		scale = (double)rct.height() / minMap.height();
		pt.drawPixmap(rct, minMap, minMap.rect());
	}
	//
	pt.setOpacity(0.25);
	pt.setBrush(Qt::black);
	if( m_scrollY0 != 0 ) {
		rct.setHeight(m_scrollY0*scale);
		pt.drawRect(rct);			//	現エリアより上部（前）描画
	}
	if( minMap.height() - (m_scrollY0+nLines) > 0 ) {
		rct.setY((m_scrollY0+nLines)*scale);
		rct.setHeight((minMap.height() - (m_scrollY0+nLines))*scale);
		pt.drawRect(rct);			//	現エリアより下部（後）描画
	}
	rct.setY(m_scrollY0*scale);
	rct.setHeight(nLines*scale);
	pt.setBrush(Qt::transparent);
	pt.setPen(Qt::red);
	pt.drawRect(rct);				//	現エリアに赤枠描画
}
void EditView::drawCursor(QPainter& pt)
{
	if( !m_dispCursor ) return;
	pt.setOpacity(1.0);
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	//int py = (vln - verticalScrollBar()->value()) * lineHeight() + DRAW_Y_OFFSET*2;
	int py = (vln - m_scrollY0) * lineHeight() + DRAW_Y_OFFSET*2;
	QRect rct = rect();
	if( py < 0 || py >= rct.height() )
		return;		//	画面外の場合
	int hv = 0;		//horizontalScrollBar()->value();
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) + m_lineNumAreaWidth;
	int ht = QFontMetrics(m_font).ascent();
	pt.fillRect(QRect(px /*- hv*/, py, CURSOR_WD, ht),
						typeSettings()->color(TypeSettings::CURSOR));
}
int EditView::textWidth(const QString &text) const
{
	if( text.isEmpty() ) return 0;
	Buffer b;
	b.basicInsertText(0, (wchar_t *)text.data(), text.size());
	return textWidth(0, b.size(), b.size(), &b);
}
int EditView::textWidth(pos_t first, ssize_t sz, pos_t last, const Buffer* pbuffer) const
{
	if( !sz ) return 0;
	if( pbuffer == 0 ) pbuffer = buffer();
	QFontMetrics fm = fontMetrics();
	QFontMetrics fmBold(m_fontBold);
	int nTab = document()->typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	//int tabWidth = fm.width(QString(nTab, QChar(' ')));
	int wd = 0;
#if	1
	auto chWidth = m_fontWidth;		//fm.width(QString("8"));
	const auto endpos = first + sz;
	auto pos = first;
	while( pos != endpos ) {
		auto ch = pbuffer->operator[](pos);
		if( ch == '\t' ) {
			wd += chWidth * (nTab - (pos - first) % nTab);
		} else if( ch == '\r' || ch == '\n' ) {
			break;
		} else if( ch < 0x100 )
			wd += chWidth;
		else {
			wd += chWidth * 2;
		}
		++pos;
	}
#else
	bool bHTML = typeSettings()->name() == "HTML";
	int ln = pbuffer->positionToLine(first);
	const TypeSettings *pTypeSettings = typeSettings();
	uint lineFlags = pbuffer->lineFlags(ln);
#if	0		//##
	if( (lineFlags & Buffer::LINEFLAG_IN_SCRIPT) != 0 ) {
		bHTML = false;
		if( m_jsTypeSettings != 0 ) {
			pTypeSettings = m_jsTypeSettings;
		}
	} else if( (lineFlags & Buffer::LINEFLAG_IN_PHP) != 0 ) {
		bHTML = false;
		///dt.setInPHP(true);
		if( m_phpTypeSettings != 0 ) {
			pTypeSettings = m_phpTypeSettings;
		}
	}
#endif
	ViewTokenizer dt(pTypeSettings, pbuffer, first, sz, last);
	dt.setCursorLine();		//	HTML特殊文字を置換しませんように
	if( (lineFlags & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0 )
		dt.setInBlockComment(true);
	QString token;
	for(;;) {
		token = dt.nextToken();
		if( token.isEmpty() ) break;
		if( token == "\t" ) {
			wd = (wd / tabWidth + 1) * tabWidth;
			//wd += tabWidth;
		} else if( token[0].unicode() < 0x20 ) {
			wd += fm.width(QString(QChar('@' + token[0].unicode())));
		} else {
			//QString token0 = token;
			if( dt.ix() > first + sz ) {		//	URL 対応
				token = token.left(first + sz - dt.tokenix());
			}
			QString fullText = dt.fullText();
			if( !dt.isInLineComment() && !dt.isInBlockComment()
					&& dt.tokenType() == ViewTokenizer::ALNUM
					&& (!bHTML || dt.isInHTMLTag())
					&& (pTypeSettings->boolValue(TypeSettings::KEYWORD1_BOLD)
							&& pTypeSettings->isKeyWord1(fullText)
						|| pTypeSettings->boolValue(TypeSettings::KEYWORD2_BOLD)
							&& pTypeSettings->isKeyWord2(fullText)) )
			{
				wd += fmBold.width(token);
			} else
				wd += fm.width(token);
		}
	}
#endif
	return wd;
}
void EditView::pointToLineOffset(const QPoint &pnt, int &vln, int &offset) const
{
	vln = qMin(pnt.y() / lineHeight() + m_scrollY0, EOFLine());
	offset = pxToOffset(vln, pnt.x());
}
int EditView::pxToOffset(int vln, int px) const
{
	if( px == 0 ) return 0;
	QFontMetrics fm = fontMetrics();
	QFontMetrics fmBold(m_fontBold);
	const int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	const int chWidth = m_fontWidth;		//fm.width(QString(nTab, QChar('8')));
	int pos = viewLineMgr()->viewLineStartPosition(vln);
	const int nextLinePos = viewLineMgr()->viewLineStartPosition(vln+1);
	int wd = 0;
	int offset = 0;
	const Buffer* buf = buffer();
	while( wd < px && pos < nextLinePos ) {
		auto ch = buf->operator[](pos);
		if( ch == '\t' ) {
			wd += chWidth * (nTab - offset % nTab);
		} else if( ch == '\r' || ch == '\n' ) {
			break;
		} else if( ch < 0x100 )
			wd += chWidth;
		else {
			wd += chWidth * 2;
		}
		++offset;
		++pos;
	}
	return px < wd ? offset - 1 : offset;
#if	0
	QFontMetricsF fm = QFontMetricsF(fontMetrics());
	QFontMetricsF fmBold(m_fontBold);
	int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int tabWidth = fm.width(QString(nTab, QChar(' ')));
	//QString txt = m_doc->lineText(ln);
	//DrawTokenizer dt(txt);
	bool bHTML = typeSettings()->name() == "HTML";
	const TypeSettings *pTypeSettings = typeSettings();
	int offset0;
	int ln = viewLineToDocLine(vln, offset0);
	const int lineStart = viewLineMgr()->viewLineStartPosition(vln);
	const int lineStart2 = viewLineMgr()->viewLineStartPosition(vln+1);
	ssize_t sz = lineStart2 - lineStart;
	pos_t first = lineStart;
	ViewTokenizer dt(typeSettings(), buffer(), first, sz, lineStart2);
	dt.setCursorLine();
	int offset = 0;
	qreal wd = 0;
	for(;;) {
		//int ix = dt.ix();
		QString token = dt.nextToken();
		int ix = dt.tokenix();
		if( token.isEmpty() || dt.isTokenNewLine() )
			return ix - lineStart;
		if( token == "\t" ) {
			wd = ((int)wd / tabWidth + 1) * tabWidth;
			if( wd > px ) return ix - lineStart;
			if( wd == px ) return dt.ix() - lineStart;
		} else if( token[0].unicode() < 0x20 ) {
			wd += fm.width(QString(QChar('@' + token[0].unicode())));
			if( wd > px ) return ix - lineStart;
			if( wd == px ) return dt.ix() - lineStart;
		} else {
			QFontMetricsF *pFM;
			QString token0 = token;
			if( dt.ix() > first + sz ) {
				token = token.left(first + sz - dt.tokenix());
			}
			if( !dt.isInLineComment() && !dt.isInBlockComment()
					&& dt.tokenType() == ViewTokenizer::ALNUM
					&& (!bHTML || dt.isInHTMLTag())
					&& (pTypeSettings->boolValue(TypeSettings::KEYWORD1_BOLD)
							&& pTypeSettings->isKeyWord1(token0)
						|| pTypeSettings->boolValue(TypeSettings::KEYWORD2_BOLD)
							&& pTypeSettings->isKeyWord2(token0)) )
			{
				pFM = &fmBold;
			} else
				pFM = &fm;
			int wd2 = wd + pFM->width(token);
			if( wd2 > px ) {
				for(int i = 0; i < token.size(); ++i) {
					if( (wd += pFM->width(token.mid(i, 1))) > px )
						return ix + i - lineStart;
				}
			}
			wd = wd2;
		}
	}
#endif
}
void EditView::insertTextSub(QString text, bool ctrl, bool shift, bool alt)
{
			m_textCursor->insertText(text);		//	文字挿入
}
void EditView::insertTextRaw(pos_t pos, const QString &text)
{
	document()->insertText(pos, text);
	update();
}
void EditView::onBackSpace(bool ctrl, bool shift, bool alt)
{
			m_textCursor->movePosition(TextCursor::LEFT /*, TextCursor::KEEP_ANCHOR*/);
	//if( !editForVar(QString()) )
		m_textCursor->deleteChar(/*BS=*/true);
}
void EditView::onDelete(bool ctrl, bool shift, bool alt)
{
	m_textCursor->deleteChar();
}
void EditView::deleteText(pos_t pos, ssize_t sz, bool BS)
{
	document()->deleteText(pos, sz, BS);
	update();
}
