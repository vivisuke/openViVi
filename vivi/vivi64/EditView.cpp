#include <QtGui>
//#include <QPainter>
#include <QDebug>
#include "MainWindow.h"
#include "Document.h"
#include "EditView.h"
#include "ViewTokenizer.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "TextCursor.h"
#include "viewLineMgr.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"
#include "../buffer/UTF16.h"
#include "../buffer/sssearch.h"

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
#if	0
QString getText(const Buffer &buffer, int pos, int sz)
{
	QString text;
	while( --sz >= 0 )
		text += QChar(buffer.charAt(pos++));
	return text;
}
#endif
//----------------------------------------------------------------------
EditView::EditView(MainWindow* mainWindow, Document *doc /*, TypeSettings* typeSettings*/)
	: m_mainWindow(mainWindow)
	, m_document(doc)
	, m_buffer(doc->buffer())
	//, m_buffer(buffer)
	//, m_typeSettings(nullptr)
	, m_lineNumDigits(3)		//	初期値は3桁 1〜999
	,m_scrollX0(0)
	,m_scrollY0(0)
	, m_minMapDragging(false)
	, m_dispCursor(true)
	//, m_viewLineMgr(new ViewLineMgr(this))
{
	Q_ASSERT(doc != nullptr);
	//setFocusPolicy(Qt::ClickFocus);
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
	//	テキストエリア
	//m_textAreaWidget.setParent(this);
	//m_textAreaWidget.setCursor(Qt::IBeamCursor);
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
	auto wd = rct.width();
	//
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	m_minMapWidget.setGeometry(rct);
	//
	rct.setX(0);
	rct.setWidth(m_lineNumAreaWidth);
	m_lineNumAreaWidget.setGeometry(rct);
	//
	//m_textAreaPixmap = QPixmap(wd - m_lineNumAreaWidth - MINMAP_WIDTH, rct.height());
#if	0
	rct.setX(m_lineNumAreaWidth);
	rct.setWidth(wd - m_lineNumAreaWidth - MINMAP_WIDTH);
	m_textAreaWidget.setGeometry(rct);
#endif
}
void EditView::setPlainText(const QString& txt)
{
	buffer()->clear();
	buffer()->insertText(0, (cwchar_t*)txt.data(), txt.size());
	document()->buildMinMap();
	update();
}
void EditView::setFullPathName(const QString &pathName)
{
	document()->setPathName(pathName);
}
TypeSettings *EditView::typeSettings()
{
	return document()->typeSettings();
}
const TypeSettings *EditView::typeSettings() const
{
	return document()->typeSettings();
}
bool EditView::isModified() const		// { return m_modified; }
{
	return document()->isModified();
}
bool EditView::hasSelection() const
{
	return m_textCursor->hasSelection();
}
bool EditView::hasSelectionInALine() const
{
	if( !m_textCursor->hasSelection() ) return false;
	pos_t first = m_textCursor->selectionFirst();
	pos_t last = m_textCursor->selectionLast();
	int ln1 = document()->positionToLine(first);
	int ln2 = document()->positionToLine(last);
	return ln1 == ln2;
}
QString EditView::text(pos_t pos, ssize_t sz) const
{
	//return getText(*buffer(), 0, bufferSize());
	return document()->text(pos, sz);
}
QString EditView::selectedText() const
{
	return m_textCursor->selectedText();
}
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
#if	1
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
	int hpos = m_scrollX0 * m_fontWidth;
	int wd = rect().width() - m_lineNumAreaWidth - MINMAP_WIDTH;
	if( px < hpos ) {
		m_scrollX0 = qMax(0, px - HS_MARGIN) / m_fontWidth;
		scrolled = true;
	} else if( px >= hpos + wd - HS_MARGIN / 2 ) {
		m_scrollX0 = (px - wd + HS_MARGIN) / m_fontWidth;
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
QString EditView::title() const
{
	return document()->title();
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
void EditView::resizeEvent(QResizeEvent *event)
{
	onResized();
}
void EditView::focusInEvent( QFocusEvent * event )
{
}
void EditView::focusOutEvent( QFocusEvent * event )
{
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
	if( pnt.x() < m_lineNumAreaWidth ) {
	} else {
		pnt.setX(pnt.x() - m_lineNumAreaWidth);
		int vln, offset;
		pointToLineOffset(pnt, vln, offset);
		m_textCursor->setPosition(viewLineStartPosition(vln) + offset);
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
	if( !alt && ctrl && !shift ) {
		ssize_t sz = typeSettings()->intValue(TypeSettings::FONT_SIZE);
		if( event->delta() > 0 ) {
			++sz;
		} else {
			--sz;
		}
		if( sz < 1 ) sz = 1;
		typeSettings()->setIntValue(TypeSettings::FONT_SIZE, sz);
		updateFont();
		emit showMessage(tr("fontSize = %1").arg(sz), 5000);
		return;
	}
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
	bool im = isModified();
	QString text = event->text();
	switch( event->key() ) {
	case Qt::Key_Left:
		if( ctrl )
			m_textCursor->movePosition(TextCursor::PREV_WORD, mvmd);
		else {
			if( !shift /*&& !isBoxSelectionMode()*/ && m_textCursor->hasSelection() )
				m_textCursor->setPosition(m_textCursor->selectionFirst());
			else
				m_textCursor->movePosition(TextCursor::LEFT, mvmd);
		}
		break;
	case Qt::Key_Right:
		if( ctrl )
			m_textCursor->movePosition(TextCursor::NEXT_WORD, mvmd);
		else {
			if( !shift /*&& !isBoxSelectionMode()*/ && m_textCursor->hasSelection() )
				m_textCursor->setPosition(m_textCursor->selectionLast());
			else
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
	case Qt::Key_Escape:
		//onEscape(ctrl, shift, alt);
		m_textCursor->clearSelection();
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		switch( m_mainWindow->newLineType() ) {
		default:
		case NEWLINE_CRLF:	text = "\r\n";	break;
		case NEWLINE_LF:	text = "\n";	break;
		case NEWLINE_CR:	text = "\r";	break;
		}
		goto ins;
	case Qt::Key_Tab:
		text = "\t";
		goto ins;
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
		if( text[0] != '\t' && text[0] != '\n' && text[0] != '\r' && text[0].unicode() < 0x20 )
			return;
	ins:
		insertTextSub(text, ctrl, shift, alt);
		if( isModified() != im )
			emit modifiedChanged();
		makeCursorInView();
		resetCursorBlinking();
		update();
		emit updateUndoRedoEnabled();
		return;
	}
	}
	onCursorPosChanged();
	makeCursorInView();
	update();
}
void EditView::doInsertText(const QString &text, bool ctrl, bool shift, bool alt)
{
	bool im = isModified();
	insertTextSub(text, ctrl, shift, alt);
	if( isModified() != im )
		emit modifiedChanged();
	makeCursorInView();
	resetCursorBlinking();
	update();
	emit updateUndoRedoEnabled();
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
		m_preeditString.clear();
		m_textCursor->insertText(text);
	}
	m_preeditString = event->preeditString();
	if( !m_preeditString.isEmpty() ) {		//	■ 変換候補ありの場合
		qDebug() << "  start = " << event->replacementStart () <<
					", len = " << event->replacementLength ();
		qDebug() << "  insertText " << m_preeditString;
		update();
	}
	QWidget::inputMethodEvent( event );
}
void EditView::paintEvent(QPaintEvent *event)
{
	//qDebug() << "lineCount = " << buffer()->lineCount();
	QPainter pt(this);
	//QPainter pt2(&m_textAreaPixmap);
	//QPainter pt2(&m_textAreaWidget);
	auto rct = rect();
	auto wd = rct.width();
	//qDebug() << "rect = " << rct;
	//	全体背景描画
	pt.setPen(Qt::transparent);
	//pt2.setPen(Qt::transparent);
	QColor bg = typeSettings()->color(TypeSettings::BACK_GROUND);
	//pt2.setBrush(bg);
	//pt2.drawRect(m_textAreaPixmap.rect());
#if	1
	//QColor col = typeSettings()->color(TypeSettings::BACK_GROUND);
	pt.setBrush(bg);
	pt.drawRect(rct);
#endif
	//
	drawPreeditString(pt);
	drawSelection(pt);
	drawTextArea(pt);			//	テキストエイア描画
	drawMinMap(pt);				//	ミニマップ描画
	drawCursor(pt);				//	テキストカーソル表示
	drawLineCursor(pt);			//	行カーソル表示
	//	行番号部分背景描画
	pt.setOpacity(1.0);
	QColor col = typeSettings()->color(TypeSettings::LINENUM_BG);
	pt.setPen(Qt::transparent);
	pt.setBrush(col);
	rct.setWidth(m_lineNumAreaWidth);
	pt.drawRect(rct);
	drawLineNumberArea(pt);		//	行番号エリア描画
	//
#if 0
	pt.setOpacity(1.0);
	rct.setX(m_lineNumAreaWidth);
	rct.setWidth(wd - m_lineNumAreaWidth - MINMAP_WIDTH);
	pt.drawPixmap(rct, m_textAreaPixmap, m_textAreaPixmap.rect());
#endif
}
void EditView::drawLineNumberArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(Qt::black);
    int mg = m_fontWidth*2;		//.width("88");
    int mg4 = mg / 4;
	int py = 0 /*DRAW_Y_OFFSET*/;
	int limit = buffer()->lineCount() + (buffer()->isBlankEOFLine() ? 1 : 0);
	for (int ln = 1 + m_scrollY0; ln <= limit && py < rct.height(); ++ln, py+=m_lineHeight) {
		//	行番号、行編集・保存済みフラグ表示
		uint flags = buffer()->lineFlags(ln-1);
		if( (flags & Buffer::LINEFLAG_MODIFIED) != 0 ) {
			if( (flags & Buffer::LINEFLAG_SAVED) != 0 ) {
				pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
									typeSettings()->color(TypeSettings::LINENUM_SAVED));
			} else {
				pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
									typeSettings()->color(TypeSettings::LINENUM_MODIFIED));
			}
		}
		//qDebug() << "line flags = " << flags;
		//
		QString number = QString::number(ln);
		int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
		pt.drawText(px, py+m_fontHeight, number);
	}
}
void EditView::drawSelection(QPainter& pt)
{
	if( !m_textCursor->hasSelection() ) return;
	auto rct = rect();
	const auto nLines = rct.height() / m_lineHeight;
	auto first = m_textCursor->selectionFirst();
	auto last = m_textCursor->selectionLast();
	int firstLn = viewLineMgr()->positionToViewLine(first);
	int lastLn = viewLineMgr()->positionToViewLine(last);
	if( firstLn > m_scrollY0 + nLines || lastLn < m_scrollY0 ) return;		//	画面外の場合
	auto firstLnPos = viewLineMgr()->viewLineStartPosition(firstLn);
	auto lastLnPos = viewLineMgr()->viewLineStartPosition(lastLn);
	int px1 = viewLineOffsetToPx(firstLn, first - firstLnPos) + m_lineNumAreaWidth;
	int px9 = viewLineOffsetToPx(lastLn, last - lastLnPos) + m_lineNumAreaWidth;
	int py1 = (firstLn - m_scrollY0) * m_lineHeight;		//	行上部座標
	pt.setBrush(typeSettings()->color(TypeSettings::SEL_BG));
	pt.setPen(Qt::transparent);
	if( firstLn == lastLn ) {	//	選択が１行内の場合
		QRect r(px1, py1, px9 - px1, m_lineHeight);
		pt.drawRect(r);
		return;
	}
	//	undone: クリッピング処理
	//	選択開始行
	int sz = viewLineMgr()->viewLineSize(firstLn);
	int px2 = viewLineOffsetToPx(firstLn, sz) + m_lineNumAreaWidth;
	QRect r(px1, py1, px2 - px1, m_lineHeight);
	pt.drawRect(r);
	//	途中行
	py1 += m_lineHeight;
	for (int vln = firstLn + 1; vln < lastLn; ++vln, py1+=m_lineHeight) {
		int pos0 = viewLineMgr()->viewLineStartPosition(vln);
		int sz = viewLineMgr()->viewLineSize(vln);
		int px2 = viewLineOffsetToPx(vln, sz) + m_lineNumAreaWidth;
		QRect r(m_lineNumAreaWidth, py1, px2 - m_lineNumAreaWidth, m_lineHeight);
		pt.drawRect(r);
	}
	//	選択修了行
	QRect r2(m_lineNumAreaWidth, py1, px9 - m_lineNumAreaWidth, m_lineHeight);
	pt.drawRect(r2);
}
void EditView::drawTextArea(QPainter& pt)
{
	if( m_preeditString.isEmpty() ) m_preeditWidth = 0;
	auto rct = rect();
	pt.setPen(Qt::black);
	pt.setOpacity(1.0);
	//pt.drawText(100, 100, "Hello");
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
		auto px = m_lineNumAreaWidth;
		if( m_textCursor->viewLine() == EOFLine() && !m_preeditString.isEmpty() )
			px += m_preeditWidth;
		pt.drawText(px, py+m_fontHeight, "[EOF]");
	}
}
//	１行表示
//
//			
void EditView::drawLineText(QPainter &pt,
							int &px,
							int py,			//	ベースライン位置
							int ln,			//	論理行番号, 0 org
							pos_t ls,			//	表示行先頭位置
							int vlnsz,		//	表示行サイズ
							pos_t nxdls,		//	次の論理行先頭位置
							bool &inBlockComment,
							bool &inLineComment,
							QString &quotedText)
{
	int pxLimit = rect().width() - MINMAP_WIDTH + m_scrollX0*m_fontWidth;
	QFontMetrics fm(m_font);
	QFontMetrics fmBold(m_fontBold);
	QFontMetrics fmMB(m_fontMB);
	const auto descent = fm.descent();
	const auto chWidth = m_fontWidth;		//fm.width(QString("8"));
	int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int ix = 0;
	int clmn = 0;
	const int last = ls + vlnsz;
	const QString lineComment = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
	//const QString lineComment = "//";		//	暫定コード
	int curpos = 0;
	if( !m_preeditString.isEmpty() && ln == m_textCursor->viewLine() )
		curpos = m_textCursor->position();
	ViewTokenizer tkn(typeSettings(), buffer(), ls, vlnsz, nxdls /*, curpos*/);
	tkn.setInLineComment(inLineComment);
	tkn.setInBlockComment(inBlockComment);
	QString token = tkn.nextToken();
	int peDX = 0;
	QString nextToken;
	while( !token.isEmpty() ) {
		if( px >= pxLimit )
			break;
		if( !m_preeditString.isEmpty() && ln == m_textCursor->viewLine() ) {
			if( tkn.tokenix() < m_textCursor->position() && tkn.ix() > m_textCursor->position() ) {
				nextToken = token.right(tkn.ix() - m_textCursor->position());
				token = token.left(m_textCursor->position() - tkn.tokenix());
			} else if( tkn.tokenix() >= m_textCursor->position() )
				peDX = m_preeditWidth;
		}
		if( tkn.tokenix() + token.size() > last )
			token = token.left(last - tkn.tokenix());
		//qDebug() << "type = " << tkn.tokenType() << ", token = " << token;
		bool bold = false;
		//pt.setFont(m_font);
		QColor col = typeSettings()->color(inBlockComment || inLineComment ? TypeSettings::COMMENT : TypeSettings::TEXT);
		auto tabwd = 0;	//fm.width(token);
		if( token == "\t" ) {
			token = ">";
			col = typeSettings()->color(TypeSettings::TAB);
			int clmn = (px - m_lineNumAreaWidth) / chWidth;
			tabwd = (nTab - (clmn % nTab)) * chWidth;
		} else {
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
					} else if( typeSettings()->isKeyWord2(token) ) {
						col = typeSettings()->color(TypeSettings::KEYWORD2);
						bold = typeSettings()->boolValue(TypeSettings::KEYWORD2_BOLD);
					}
					break;
				case ViewTokenizer::DIGITS:
					if( !inLineComment && !inBlockComment )
						col = typeSettings()->color(TypeSettings::DIGITS);
					break;
				case ViewTokenizer::QUOTED:
					if( !inLineComment && !inBlockComment )
						col = typeSettings()->color(TypeSettings::STRING);
					break;
				case ViewTokenizer::ZEN_SPACE:
					token = QChar(L'□');
					col = typeSettings()->color(TypeSettings::ZEN_SPACE);
					break;
				case ViewTokenizer::CTRL:
#if	0
					if( token == "\t" ) {
						token = ">";
						col = typeSettings()->color(TypeSettings::TAB);
						int clmn = (px - m_lineNumAreaWidth) / chWidth;
						tabwd = (nTab - (clmn % nTab)) * chWidth;
					}
#endif
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
		}
		if( !token.isEmpty() ) {
			if( bold )
				pt.setFont(m_fontBold);
			else
				pt.setFont(m_font);
			pt.setPen(col);
			px += drawTokenText(pt, token, clmn, px, py, peDX, tabwd, chWidth, descent /*, col*/ /*, bold*/);
		}
		//
		if( !nextToken.isEmpty() ) {
			token = nextToken;
			nextToken.clear();
			tkn.m_tokenix = m_textCursor->position();
		} else
			token = tkn.nextToken();
	}
}
int EditView::drawTokenText(QPainter& pt,
								QString& token,
								int& clmn,
								int& px,
								int py,			//	ベースライン位置
								int peDX,		//	IME変換候補表示のためのX座標差分
								int tabwd,
								const int chWidth,
								const int descent)		//	フォントのベースライン下高さ
								//QColor& col)
								//bool bold)
{
	int wd = 0;	 tabwd;
	int sx = m_scrollX0 * m_fontWidth;
	//pt.setPen(col);
#if	0
	if (bold) {
		pt.setFont(m_fontBold);
		//pt.drawText(px + peDX - sx, py, token);
	} //else
#endif
#if	0
	if( token[0] == '\t' ) {
		int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
		
	} else 
#endif
	if (token[0] < 0x100) {
		//pt.setFont(m_font);
		pt.drawText(px + peDX - sx, py, token);
		wd = chWidth * token.size();
	} else {
		auto x = px + peDX;
		wd = 0;
		for (int i = 0; i != token.size(); ++i) {
			QString txt = token[i];
			if (txt == " ") {
				x += chWidth;
				wd += chWidth;
			}
			else {
				pt.drawText(x - sx, py - m_lineHeight + descent, chWidth * 2, m_lineHeight, Qt::AlignHCenter | Qt::AlignBottom, txt);
				x += chWidth * 2;
				wd += chWidth * 2;
			}
		}
	}
	return !tabwd ? wd : tabwd;
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
	pt.setOpacity(1.0);
}
void EditView::drawPreeditString(QPainter&pt)
{
	if( m_preeditString.isEmpty() ) return;
	QFontMetrics fm(m_font);
	pt.setOpacity(1.0);
	pt.setPen(typeSettings()->color(TypeSettings::TEXT));
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	int py = (vln - m_scrollY0) * lineHeight() + DRAW_Y_OFFSET*2;	//	ベースライン位置
	QRect rct = rect();
	if( py < 0 || py >= rct.height() )
		return;		//	画面外の場合
	int hv = 0;		//horizontalScrollBar()->value();
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) + m_lineNumAreaWidth;
	int ht = fm.ascent();
	const auto descent = fm.descent();
	pt.drawText(px, py+m_fontHeight-descent, m_preeditString);
	m_preeditWidth = fm.width(m_preeditString);
}
//	行カーソル表示
void EditView::drawLineCursor(QPainter &pt)
{
	if( !typeSettings()->boolValue(TypeSettings::LINE_CURSOR) ) return;
	int vln = m_textCursor->viewLine();
	int py = (vln - m_scrollY0) * lineHeight() + DRAW_Y_OFFSET*2;
	py += fontHeight();
	QRect rct = rect();
	if( py >= 0 && py < rct.height() ) {
		//QPixmap wholeMap = document()->wholeMap();
		pt.setPen(typeSettings()->color(TypeSettings::LINE_CURSOR));
		int wd = rct.width();
		//if( globSettings()->boolValue(GlobalSettings::WHOLE_MAP) && py <= wholeMap.height() )
			wd -= MINMAP_WIDTH;
		pt.drawLine(0, py, wd, py);
	}
}
void EditView::drawCursor(QPainter& pt)
{
	if( !m_dispCursor ) return;
	pt.setOpacity(1.0);
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	int py = (vln - m_scrollY0) * lineHeight() + DRAW_Y_OFFSET*2;
	QRect rct = rect();
	if( py < 0 || py >= rct.height() )
		return;		//	画面外の場合
	int hv = m_scrollX0 * m_fontWidth;
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) + m_lineNumAreaWidth;
	if( !m_preeditString.isEmpty() ) px += m_preeditWidth;
	int ht = QFontMetrics(m_font).ascent();
	pt.fillRect(QRect(px - hv, py, CURSOR_WD, ht),
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
	int col = 0;
	while( pos != endpos ) {
		auto ch = pbuffer->operator[](pos);
		if( ch == '\t' ) {
			auto n = (nTab - col % nTab);
			col += n;
			wd += chWidth * n;
		} else if( ch == '\r' || ch == '\n' ) {
			break;
		} else if( ch < 0x100 ) {
			++col;
			wd += chWidth;
		} else {
			col += 2;
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
	int col = 0;
	int offset = 0;
	const Buffer* buf = buffer();
	while( wd < px && pos < nextLinePos ) {
		auto ch = buf->operator[](pos);
		if( ch == '\t' ) {
			int n = (nTab - col % nTab);
			col += n;
			wd += chWidth * n;
		} else if( ch == '\r' || ch == '\n' ) {
			break;
		} else if( ch < 0x100 ) {
			col += 1;
			wd += chWidth;
		} else {
			col += 2;
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
	emit updateUndoRedoEnabled();
}
void EditView::undo()
{
	if( !document()->canUndo() ) return;
	m_delForVarPos.clear();
	bool im = isModified();
	pos_t pos = document()->undo();
	if( isModified() != im )
		emit modifiedChanged();
	//##m_textCursor->setBoxSelectionMode(false);
	m_textCursor->setPosition(pos);
	//##checkAssocParen();
	//##updateScrollBarInfo();
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
void EditView::redo()
{
	if( !document()->canRedo() ) return;
	m_delForVarPos.clear();
	bool im = isModified();
	pos_t pos = document()->redo();
	if( isModified() != im )
		emit modifiedChanged();
	//##m_textCursor->setBoxSelectionMode(false);
	m_textCursor->setPosition(pos);
	//##checkAssocParen();
	//##updateScrollBarInfo();
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
void EditView::cut(bool append)
{
	bool im = isModified();
	copy(true, append);
	if( !m_textCursor->hasSelection() ) {
		m_textCursor->movePosition(TextCursor::BEG_LINE);
		m_textCursor->movePosition(TextCursor::DOWN, TextCursor::KEEP_ANCHOR);
		if( !m_textCursor->hasSelection() )	//	EOF が空行でない場合
			m_textCursor->movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
	}
	if( !m_textCursor->hasSelection() ) return;
	//##setupParabolicChars();
	m_textCursor->deleteChar();
	if( !im )
		emit modifiedChanged();
	//##updateScrollBarInfo();
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
int EditView::copy(bool bCut, bool append)
{
	if( !m_textCursor->hasSelection() ) return 0;
	auto first = m_textCursor->selectionFirst();
	auto last = m_textCursor->selectionLast();
	auto sz = last - first;
	document()->copy(first, sz);
	return sz;
}
void EditView::paste()
{
	QClipboard *cb = qApp->clipboard();
	const QMimeData *md = cb->mimeData();
	if( md != 0 && md->hasFormat("MT_BOX_SELECT") ) {
		qDebug() << md->hasText();
		QString text = md->text();
		boxPaste(text);
	} else {
		QString text = cb->text();
		if( text.isEmpty() ) return;
		paste(text);
	}
}
void EditView::paste(const QString &text)
{
	if( text.isEmpty() ) return;
#if	1
	m_textCursor->insertText(text);
#else
	setupParabolicChars();
	bool im = isModified();
	m_textCursor->insertText(text);
	emit textInserted(text);
	if( !im )
		emit modifiedChanged();
	checkAssocParen();
	updateScrollBarInfo();
#endif
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
void EditView::boxPaste(const QString &)
{
}
bool EditView::saveFile() const
{
	bool im = isModified();
	bool rc = document()->saveFile();
	if( rc && im )
		emit modifiedChanged();
	return rc;
}
bool EditView::findForward(const QString &text, uint opt, bool loop, bool next, bool vi)
{
	pos_t pos0;
	if( !m_textCursor->hasSelection() )
		pos0 = m_textCursor->position();
	else
		pos0 = m_textCursor->selectionFirst();
	if( next )
		++pos0;
	int algorithm = mainWindow()->searchAlgorithm();
	SSSearch &sssrc = mainWindow()->sssrc();
	if( g_globSettings.boolValue(GlobalSettings::REGEXP)
		|| (opt & SSSearch::REGEXP) != 0 )
	{
		algorithm = SSSearch::STD_REGEX;
	}
	int ln = positionToLine(pos0);
	int lineStart = lineStartPosition(ln);
	pos_t pos = buffer()->indexOf(sssrc, (wchar_t *)text.data(), text.size(), pos0, opt, -1, algorithm);
	//pos_t pos = m_doc->indexOf(sssrc, text, pos0, opt, -1, algorithm);
	if( pos < 0 ) {
		if( !loop ) return false;
		pos = buffer()->indexOf(sssrc, (wchar_t *)text.data(), text.size(), 0, opt, pos0+1, algorithm);
		//pos = m_doc->indexOf(sssrc, text, 0, opt, -1, algorithm);
		if( pos < 0 ) return false;
	}
	m_textCursor->setPosition(pos);
	if( !vi ) {
		const int matchLength = sssrc.matchLength();
		m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, matchLength);
	}
	makeCursorInView(true);
	//onCursorPosChanged();
	update();
	return true;
}
bool EditView::findBackward(const QString &text, uint opt, bool loop, bool vi)
{
	pos_t pos0;
	if( !m_textCursor->hasSelection() )
		pos0 = m_textCursor->position();
	else
		pos0 = m_textCursor->selectionFirst();
	int algorithm = mainWindow()->searchAlgorithm();
	if( globSettings()->boolValue(GlobalSettings::REGEXP) )
		algorithm = SSSearch::STD_REGEX;
	SSSearch &sssrc = mainWindow()->sssrc();
	pos_t pos = buffer()->rIndexOf(sssrc, (wchar_t *)text.data(), text.size(), pos0, opt, 0, algorithm);
	//pos_t pos = m_doc->rIndexOf(sssrc, text, pos0, opt, /*last=*/-1, algorithm);
	if( pos < 0 ) {
		if( !loop ) return false;
		pos = buffer()->rIndexOf(sssrc, (wchar_t *)text.data(), text.size(), bufferSize(), opt, 0, algorithm);
		//pos = m_doc->rIndexOf(sssrc, text, bufferSize(), opt, /*last=*/-1, algorithm);
		if( pos < 0 ) return false;
	}
	m_textCursor->setPosition(pos);
	if( !vi ) {
		const int matchLength = sssrc.matchLength();
		m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, matchLength);
	}
	makeCursorInView(true);
	//onCursorPosChanged();
	update();
	return true;
}
void EditView::findNext(const QString &pat, bool vi)
{
	uint opt = 0;
	if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) ) opt |= SSSearch::IGNORE_CASE;
	if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) ) opt |= SSSearch::WHOLE_WORD_ONLY;
	QTime tm;
	tm.start();
	bool rc = findForward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), true, vi);
	int ms = tm.elapsed();
	if( !rc )
		showMessage(tr("'%1' was not found (%2 msec).").arg(pat).arg(ms), 3000);
	else {
		showMessage(tr("(%1 msec).").arg(ms), 3000);
		mainWindow()->setFindString(pat);
	}
	setFocus();
	update();
}
void EditView::findPrev(const QString &pat, bool vi)
{
	uint opt = 0;
	if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) )
		opt |= SSSearch::IGNORE_CASE;
	if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) )
		opt |= SSSearch::WHOLE_WORD_ONLY;
	if( !findBackward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), vi) )
		showMessage(tr("'%1' was not found.").arg(pat), 3000);
	else
		mainWindow()->setFindString(pat);
	setFocus();
	update();
}
bool EditView::focusNextPrevChild(bool next)
{
	return false;
}
