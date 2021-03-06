﻿#include <assert.h>
#include <QtGui>
#include <QTransform>
#include <QMessageBox>
//#include <QPainter>
#include <QDebug>
#include "MainWindow.h"
#include "Document.h"
#include "EditView.h"
#include "tokenizer.h"
#include "ViewTokenizer.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "TextCursor.h"
#include "viewLineMgr.h"
#include "charEncoding.h"
#include "ViEngine.h"
#include "AutoCompletionDlg.h"
#include "assocParen.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"
#include "../buffer/UTF16.h"
#include "../buffer/sssearch.h"

#define		TM_FPS			50		//	50FPS
#define		HS_MARGIN		60
#define		PAI				3.1415926535
#define		MAX_HSCB		(1024*40)


extern QApplication* g_app;

//----------------------------------------------------------------------
void setupCandidates(QStringList &lst, const QString &key, const QString &type);
void setupLibNames(QStringList &lst /*, QString pat = QString()*/);

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isNewLineChar(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
inline bool isSpaceOrNewLine(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}
inline bool isLetterOrNumberOrUnderbar(const QChar &ch)
{
	return ch.isLetterOrNumber() || ch == QChar('_');
}
inline bool isLetterOrUnderbar(const QChar &ch)
{
	return ch.isLetter() || ch == QChar('_');
}
inline bool isDigit(wchar_t ch)
{
	return ch < 0x100 && isdigit(ch);
}
inline bool isAlpha(wchar_t ch)
{
	return ch < 0x100 && isalpha(ch);
}
QString autoIndentText(TypeSettings *typeSettings,
							const Buffer &buffer,
							pos_t pos0,				//	カーソル位置
							cwchar *newLineText,		//	改行文字列
							bool nxln);				//	次に行を挿入
							//bool bCPP);
							//bool &delPrevSpace);	//	カーソル直前の空白類をすべて削除
#if	0
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
#endif
#if	0
QString getText(const Buffer &buffer, int pos, int sz)
{
	QString text;
	while( --sz >= 0 )
		text += QChar(buffer.charAt(pos++));
	return text;
}
#endif
QString escapeRegExpSpecialChars(const QString txt0)
{
	QString txt;
	for(int i = 0; i < txt0.size();) {
		QChar ch = txt0[i++];
		if( ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '|'
			|| ch == '+' || ch == '*' || ch == '?' || ch == '.' || ch == '\\' )
		{
			txt += QChar('\\');
		}
		txt += ch;
	}
	return txt;
}
//----------------------------------------------------------------------
EditView::EditView(MainWindow* mainWindow, Document *doc /*, TypeSettings* typeSettings*/)
	: m_mainWindow(mainWindow)
	, m_document(doc)
	, m_buffer(doc->buffer())
	//, m_buffer(buffer)
	//, m_typeSettings(nullptr)
	, m_lineNumDigits(4)		//	初期値は4桁 1〜9999
	, m_scrollX0(0)
	, m_scrollY0(0)
	, m_autoCmplPos(-1)
	, m_aiCurPos(-1)
	, m_autoCompletionDlg(nullptr)
	, m_mouseDragging(false)
	, m_mouseLineDragging(false)
	, m_mouseDblClkDragging(false)
	, m_minMapDragging(false)
	, m_dispCursor(true)
	, m_autoCmplAtBegWord(false)
	, m_noDeleteAnimation(false)
	, m_noFallingChars(false)
	//, m_viewLineMgr(new ViewLineMgr(this))
{
	Q_ASSERT(doc != nullptr);
	//setFocusPolicy(Qt::ClickFocus);
	setAttribute(Qt::WA_InputMethodEnabled);
	setFocusPolicy(Qt::StrongFocus);
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
	//m_viewLineMgr = new ViewLineMgr(this);
	m_viewLineMgr = unique_ptr<ViewLineMgr>(new ViewLineMgr(this));
	m_viewLineMgr->setLineBreak(typeSettings->boolValue(TypeSettings::LINE_BREAK_WIN_WIDTH));
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
	//
	auto im = g_app->inputMethod();
}
EditView::~EditView()
{
	delete m_buffer;
	//delete m_viewLineMgr;
}
void EditView::doTest()
{
	qDebug() << "*** doTest() ***\n";
	qDebug() << "viewLineMgr:\n";
	qDebug() << "size = " << viewLineMgr()->size();
	for (int i = 0; i != viewLineMgr()->size(); ++i) {
		int dln, offset;
		dln = viewLineMgr()->viewLineToDocLine(i, offset);
		qDebug() << "dln = " << dln << ", offset = " << offset;
	}
}
bool EditView::isKeisenMode() const
{
	return mainWindow()->isKeisenMode();
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
	if( m_viewLineMgr->isLineBreakMode() ) {
		int vln = m_scrollY0;	// verticalScrollBar()->value();		//	画面最上部行
		int dln = viewLineToDocLine(vln);
		//qDebug() << "dln = " << dln;
		pos_t pos = m_textCursor->position();
		m_viewLineMgr->doLineBreakAll();
		//##verticalScrollBar()->setValue(dln);
		//qDebug() << "v = " << verticalScrollBar()->value();
		m_textCursor->setPosition(pos);
		//qDebug() << "v = " << verticalScrollBar()->value();
		//makeCursorInView();
		//qDebug() << "v = " << verticalScrollBar()->value();
		updateScrollBarInfo();
		//qDebug() << "v = " << verticalScrollBar()->value();
		update();
	}
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
QString EditView::newLineText() const
{
	switch( document()->newLineCode() ) {
	case CharEncoding::CR:	return QString("\r");
	case CharEncoding::LF:		return QString("\n");
	default:
	case CharEncoding::CRLF:	return QString("\r\n");
	}
}
int EditView::viewLineOffsetToPx(int vln, int offset) const
{
	auto start = viewLineMgr()->viewLineStartPosition(vln);
	//auto next = viewLineMgr()->viewLineStartPosition(vln + 1);
	return textWidth(start, offset /*, next*/);
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
	QFontMetrics fm(m_font);
	m_fontWidth = fm.width('8');
	m_fontDescent = fm.descent();
	m_fontHeight = QFontInfo(m_font).pixelSize();
	m_lineHeight = (int)(m_fontHeight * 1.2);
	m_baseLineDY = (m_lineHeight - m_fontHeight) / 2 + m_fontHeight - m_fontDescent;
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
int EditView::cursorLine() const		//	[0, EOFLine]
{
	return positionToLine(m_textCursor->position());
}
int EditView::EOFLine() const
{
	return m_viewLineMgr->EOFLine();
}
size_t EditView::bufferSize() const
{
	return buffer()->size();
}
int EditView::lineCount() const
{
	return document()->lineCount();
}
int EditView::viewLineCount() const
{
	if (m_viewLineMgr->isEmpty()) {
		return document()->lineCount();
	} else {
		return m_viewLineMgr->size() - 1;		//	1 for ダミー行
	}
}
void EditView::onCursorPosChanged()
{
	resetCursorBlinking();
	//m_dispCursor = true;
	//m_tmCounter = TM_FPS / 2;
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	emit cursorPosChanged(this, ln+1, pos - lineStartPosition(ln));
}
void EditView::jumpToLine(int ln, bool vi)		//	ln [0, EOFLine) ドキュメント行番号
{
	pos_t pos = document()->lineStartPosition(ln);
	int vln = docLineToViewLine(ln);
	int offset;
	if( viewLineToDocLine(vln, offset) != ln ) {	//	折畳まれている場合
		//##expand(vln);
		//##vln = docLineToViewLine(ln);
	}
	m_textCursor->setLineAndPosition(vln, pos);
	if( vi )
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	scrollCurQuarterOfScreen();
	makeCursorInView();
	//onCursorPosChanged();
	update();
}
void EditView::jumpAssociatedParen()
{
	m_textCursor->movePosition(TextCursor::ASSOC_PAREN);
	checkAssocParen();
	makeCursorInView();
	resetCursorBlinking();
	update();
}
void EditView::updateScrollBarInfo()
{
}
//	return:	スクロールしたかどうか
bool EditView::makeCursorInView(bool bQuarter)
{
	int anchor = m_textCursor->anchor();
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
#if	1
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
			m_scrollY0 = qMax(0, vln - nLine / 4);
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
QByteArray EditView::codecName() const
{
	return document()->codecName();
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
void EditView::setLineBreak(bool b)
{
	pos_t pos = m_textCursor->position();
	m_viewLineMgr->setLineBreak(b);
	//m_textCursor->updateViewLine();
	m_textCursor->setPosition(pos);
	makeCursorInView();
	updateScrollBarInfo();
	update();
}
void EditView::resetCursorBlinking()
{
	m_dispCursor = true;
	m_tmCounter = TM_FPS / 2;
	m_timer.start(1000/TM_FPS);
}
void EditView::setupHeaders(QStringList &lst, pos_t pos2, const QString &text)
{
	QDir dir = QDir::current();
	pos_t pos = m_textCursor->position();
	if( pos2 < pos ) {
		QString path = getText(*buffer(), pos2, pos-pos2) + text;
		while( path.startsWith("../") || path.startsWith("..\\") ) {
			path = path.mid(3);
			dir.cdUp();
		}
		if( !path.isEmpty() )
			dir.cd(path);
	}
	QStringList elst = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
	foreach(const QString fileName, elst) {
		if( fileName.endsWith(".h", Qt::CaseInsensitive) )
			lst += text + fileName + "\"";
	}
	elst = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	foreach(const QString fileName, elst) {
		lst += text + fileName;
	}
}
void EditView::setupFallingChars()
{
	if( m_noFallingChars ) return;
	pos_t first = m_textCursor->selectionFirst();
	int vlnFirst = m_textCursor->selectionFirstLine();
	pos_t last = m_textCursor->selectionLast();
	int vlnLast = m_textCursor->selectionLastLine();
	QRect rct = rect();
	int ln = viewLineToDocLine(vlnFirst);
	int offset = first - viewLineStartPosition(vlnFirst);
	int px = viewLineOffsetToPx(vlnFirst, offset);
	int py = (vlnFirst - m_scrollY0) * lineHeight();
	while( py < 0 ) {
		ln = viewLineToDocLine(++vlnFirst, offset);
		first = viewLineStartPosition(vlnFirst);
		py += lineHeight();
	}
	//pos_t last = first + sz;
	while( py < rct.height() ) {
		if( buffer()->charAt(first) == '\r' || buffer()->charAt(first) == '\n' ) {
			if( buffer()->charAt(first) == '\r'
				&& first + 1 < last && buffer()->charAt(first+1) == '\n' )
			{
				++first;
			}
			py += lineHeight();
			px = 0;
			if( ++first >= last ) break;
		} else {
			QPointF pnt(px+m_lineNumAreaWidth, py+m_fontHeight);
			qreal theta = 3.1415926535 * 2 * (qrand() % 65536) / 65536;
			QPointF v(2*qCos(theta), 2*qSin(theta));
			QString txt = QChar(buffer()->charAt(first));
			if( isSrgtPirFirstChar(txt[0]) ) {
				txt += QChar(buffer()->charAt(++first));
			}
			m_fallingChars.push_back(FallingChar(txt, pnt, v));
			//m_fallingChars.push_back(FallingChar(QChar(buffer()->charAt(first)), pnt, v));
			if( ++first >= last ) break;
			px += textWidth(first, 1 /*, last*/);
		}
	}
}
void EditView::setupFallingCharsBoxSelected()
{
}
void EditView::onTimer()
{
	//qDebug() << "onTimer()";
	if( hasFocus() && --m_tmCounter < 0 ) {
		m_dispCursor = !m_dispCursor;
		m_tmCounter = TM_FPS / 2;
		update();
	}
	int hv = m_scrollX0 * m_fontWidth;
	if( !m_fallingChars.empty() ) {
		QRect rct = rect();
		for(int i = 0; i < m_fallingChars.size(); ) {
			m_fallingChars[i].m_pnt += m_fallingChars[i].m_v;
			m_fallingChars[i].m_v.ry() += 0.1;
			if( m_fallingChars[i].m_pnt.x() - hv < 0
				|| m_fallingChars[i].m_pnt.x() - hv >= rct.width()
				|| m_fallingChars[i].m_pnt.y() >= rct.height() )
			{
				m_fallingChars.erase(m_fallingChars.begin() + i);
			} else
				++i;
		}
		update();
	}
}
void EditView::resizeEvent(QResizeEvent *event)
{
	onResized();
}
void EditView::focusInEvent( QFocusEvent * event )
{
	qDebug() << "EditView::focusInEvent()";
	//QScrollArea::focusInEvent(event);
	resetCursorBlinking();
	update();
	static bool reloading = false;
	if( fullPathName().isEmpty() || reloading )
		return;
	QFileInfo fi(fullPathName());
	if( !fi.exists() ) return;
	if( fi.lastModified() > document()->lastModified() ) {
		qDebug() << "file: " << QFileInfo(fullPathName()).lastModified();
		qDebug() << "doc: " << document()->lastModified();
		reloading = true;
		emit reloadRequest(this);
		reloading = false;
	}
}
void EditView::focusOutEvent( QFocusEvent * event )
{
}
void EditView::mousePressEvent(QMouseEvent *event)
{
	viEngine()->resetRedoRecording();
	if( mainWindow()->mode() == MODE_EX ) {
		mainWindow()->setMode(MODE_VI);
	}
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	setFocus();
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	if( event->button() == Qt::LeftButton ) {
		m_mouseDragging = true;
		if( pnt.x() >= rct.width() - MINMAP_WIDTH ) {		//	ミニマップエリアの場合
			m_minMapDragging = true;
			double scale = qMin(1.0, (double)rct.height() / document()->minMap().height());
			int nLines = rct.height() / m_lineHeight;
			m_scrollY0 = qMax(0, (int)(pnt.y() / scale) - nLines / 2);
	    	m_scrollY0 = qMin(m_scrollY0, buffer()->lineCount());		//	undone: 折返し処理対応
		} else if( pnt.x() < m_lineNumAreaWidth ) {		//	行番号エリアの場合
			m_mouseLineDragging = true;
			pnt.setX(pnt.x() - m_lineNumAreaWidth + m_scrollX0*m_fontWidth);
			int vln, offset;
			pointToLineOffset(pnt, vln, offset);
			m_curLineNumAnchor = vln;
			//qDebug() << vln;
			m_textCursor->setPosition(viewLineStartPosition(vln));
			m_textCursor->setPosition(viewLineStartPosition(vln+1), TextCursor::KEEP_ANCHOR);
			emit cursorPosChanged(this, vln, offset);
		} else {		//	通常テキストエリアの場合
			pnt.setX(pnt.x() - m_lineNumAreaWidth + m_scrollX0*m_fontWidth);
			int vln, offset;
			pointToLineOffset(pnt, vln, offset);
			m_textCursor->setPosition(viewLineStartPosition(vln) + offset, shift ? TextCursor::KEEP_ANCHOR : TextCursor::MOVE_ANCHOR);
			emit cursorPosChanged(this, vln, offset);
			makeCursorInView();
		}
		update();
	}
}
void EditView::mouseMoveEvent(QMouseEvent *event)
{
	//	暫定実装
	auto rct = rect();
	QPoint pnt = event->pos();
	//if( pnt.x() >= rct.width() - MINMAP_WIDTH )
	if( m_mouseDragging ) {
		if( m_minMapDragging )
		{
			double scale = qMin(1.0, (double)rct.height() / document()->minMap().height());
			int nLines = rct.height() / m_lineHeight;
			m_scrollY0 = qMax(0, (int)(pnt.y()/scale) - nLines / 2);
	    	m_scrollY0 = qMin(m_scrollY0, buffer()->lineCount());		//	undone: 折返し処理対応
			update();
		} else if( m_mouseLineDragging ) {		//	行選択
			int vln, offset;
			pointToLineOffset(pnt, vln, offset);
			m_curLineNum = viewLineToDocLine(vln, offset);
			int n;
			if( m_curLineNum >= m_curLineNumAnchor ) {
				m_textCursor->setPosition(lineStartPosition(m_curLineNumAnchor));
				m_textCursor->setPosition(lineStartPosition(m_curLineNum + 1), TextCursor::KEEP_ANCHOR);
				n = m_curLineNum - m_curLineNumAnchor + 1;
			} else {
				m_textCursor->setPosition(lineStartPosition(m_curLineNumAnchor + 1));
				m_textCursor->setPosition(lineStartPosition(m_curLineNum), TextCursor::KEEP_ANCHOR);
				n = m_curLineNumAnchor - m_curLineNum + 1;
			}
			makeCursorInView();
			showMessage(tr("%1 lines selected.").arg(n));
		} else {
			int hv = m_scrollX0 * m_fontWidth;
			pnt.setX(qMax(0, pnt.x() + hv - m_lineNumAreaWidth));
			//qDebug() << pnt;
			int vln, offset;
			pointToLineOffset(pnt, vln, offset);
			int ln = viewLineToDocLine(vln);
			//##if( !alt && !m_textCursor->isBoxSelectMode() )
			{
				m_textCursor->setPosition(viewLineStartPosition(vln) + offset, TextCursor::KEEP_ANCHOR);
				if( m_mouseDblClkDragging ) {
					if( m_textCursor->position() >= m_textCursor->anchor() ) {
						m_textCursor->setAnchorWordBeg();
						m_textCursor->movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
					} else {
						m_textCursor->setAnchorWordEnd();
						m_textCursor->movePosition(TextCursor::BEG_WORD, TextCursor::KEEP_ANCHOR);
					}
				}
				checkAssocParen();
			}
#if	0
			else {
				int px1 = offsetToPx(ln, offset);
				int px2 = offsetToPx(ln, offset+1);
				if( px2 < pnt.x() ) {
					px1 = px2 = pnt.x();
				}
				m_textCursor->setBoxSelPosition(m_doc->lineStartPosition(ln) + offset,
											vln, px1, px2, TextCursor::KEEP_ANCHOR);
			}
#endif
			m_textCursor->setPX(pnt.x());
			resetCursorBlinking();
			makeCursorInView();
			emit cursorPosChanged(this, ln, offset);
		}
		update();
	}
}
void EditView::mouseReleaseEvent(QMouseEvent *)
{
	m_mouseDragging = false;
	m_mouseLineDragging = false;
	m_mouseDblClkDragging = false;
	m_minMapDragging = false;
}
void EditView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QPoint pnt = event->pos();
	if( event->button() == Qt::LeftButton ) {
		//	undone: テキストエリア内チェック
		if( pnt.x() >= m_lineNumAreaWidth && pnt.x() < rect().width() - MINMAP_WIDTH ) {
			pnt.setX(pnt.x() - m_lineNumAreaWidth + m_scrollX0*m_fontWidth);
			int vln, offset;
			pointToLineOffset(pnt, vln, offset);
			m_textCursor->setPosition(viewLineStartPosition(vln) + offset);
			m_textCursor->movePosition(TextCursor::BEG_WORD);
			m_textCursor->setWordBegPos();
			m_textCursor->movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);	//	単語末尾の次
			m_textCursor->setWordEndPos();
			m_textCursor->setPX(pnt.x());
			m_mouseDragging = true;
			m_mouseDblClkDragging = true;
			makeCursorInView();
			resetCursorBlinking();
			update();
		}
	}
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
	//ViEngine *viEngine = viEngine();
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	int mvmd = shift ? TextCursor::KEEP_ANCHOR : TextCursor::MOVE_ANCHOR;
	bool im = isModified();
	QString text = event->text();
	switch( event->key() ) {
	case Qt::Key_Left:
		//viEngine->resetRedoRecording();
		if( mainWindow()->isKeisenMode() && (ctrl || shift)) {
			drawKeisenLeft(shift);
		} else {
			if( ctrl )
				m_textCursor->movePosition(TextCursor::PREV_WORD, mvmd);
			else {
				if( !shift /*&& !isBoxSelectMode()*/ && m_textCursor->hasSelection() )
					m_textCursor->setPosition(m_textCursor->selectionFirst());
				else
					m_textCursor->movePosition(TextCursor::LEFT, mvmd);
			}
		}
		break;
	case Qt::Key_Right:
		//viEngine->resetRedoRecording();
		if( mainWindow()->isKeisenMode() && (ctrl || shift)) {
			drawKeisenRight(shift);
		} else {
			if( ctrl )
				m_textCursor->movePosition(TextCursor::NEXT_WORD, mvmd);
			else {
				if( !shift /*&& !isBoxSelectMode()*/ && m_textCursor->hasSelection() )
					m_textCursor->setPosition(m_textCursor->selectionLast());
				else
					m_textCursor->movePosition(TextCursor::RIGHT, mvmd);
			}
		}
		break;
	case Qt::Key_Up:
		//viEngine->resetRedoRecording();
		if( mainWindow()->isKeisenMode() && (ctrl || shift)) {
			drawKeisenUp(shift);
		} else {
			m_textCursor->movePosition(TextCursor::UP, mvmd);
		}
		break;
	case Qt::Key_Down:
		//viEngine->resetRedoRecording();
		if( mainWindow()->isKeisenMode() && (ctrl || shift)) {
			drawKeisenDown(shift);
		} else {
			m_textCursor->movePosition(TextCursor::DOWN, mvmd);
		}
		break;
	case Qt::Key_Home:
		//viEngine->resetRedoRecording();
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
		onEscape(ctrl, shift, alt);
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		text = "\n";
#if	0
		switch( m_mainWindow->newLineType() ) {
		default:
		case NEWLINE_CRLF:	text = "\r\n";	break;
		case NEWLINE_LF:	text = "\n";	break;
		case NEWLINE_CR:	text = "\r";	break;
		}
#endif
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
		if( viEngine()->mode() != Mode::COMMAND ) {
			viEngine()->onBackSpace();
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
	viEngine()->resetRedoRecording();
	onCursorPosChanged();
	makeCursorInView();
	checkAssocParen();
	update();
}
void EditView::onEscape(bool ctrl, bool shift, bool alt)
{
	m_textCursor->clearSelection();
	m_fallingChars.clear();
	mainWindow()->resetBoxKeisenMode();
	//
	mainWindow()->clearMatchedString();
	auto md = mainWindow()->mode();
	if( globSettings()->boolValue(GlobalSettings::VI_COMMAND) && (md == MODE_INS || md == MODE_REP) ) {
		viEngine()->processCommand(0x1b);
		//mainWindow()->setMode(MODE_VI);
		//update();
	} else if( md == MODE_VI ) {
		//mainWindow()->setFindString("");
		//mainWindow()->setShowMatchedBG(false);		//	検索マッチ強調OFF
		update();
	}
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
	qDebug() << "inputMethodQuery()";
	if( query == Qt::ImMicroFocus ) {
			pos_t pos = /*m_toDeleteIMEPreeditText ? m_preeditPos :*/ m_textCursor->position();
			int vln = m_textCursor->viewLine();
			int x = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) - m_scrollX0 * m_fontWidth;
						/*- horizontalScrollBar()->value()*/;
			int y = (vln - m_scrollY0) * lineHeight();
			return QVariant(QRect(x + m_lineNumAreaWidth, y, 2, lineHeight() + 4));
	}
	return QWidget::inputMethodQuery(query);
}
void EditView::inputMethodEvent(QInputMethodEvent * event)
{
	qDebug() << "inputMethodEvent()";
	if( mainWindow()->mode() == MODE_VI )
		mainWindow()->setMode(MODE_INS);
	const QString &text = event->commitString();
	if( !text.isEmpty() ) {		//	■ IME入力が確定した場合
		m_preeditString.clear();
		m_textCursor->insertText(text);
		makeCursorInView();
		checkAssocParen();
		emit textInserted(text);
	}
	m_preeditString = event->preeditString();
	if( !m_preeditString.isEmpty() ) {		//	■ 変換候補ありの場合
		qDebug() << "  start = " << event->replacementStart () <<
					", len = " << event->replacementLength ();
		qDebug() << "  preeditString " << m_preeditString;
		if( m_textCursor->hasSelection() )
			m_textCursor->deleteChar();
		resetCursorBlinking();
		makeCursorInView();
		update();
	}
	QWidget::inputMethodEvent( event );
}
int EditView::textWidth(const QString &text) const
{
	if( text.isEmpty() ) return 0;
	Buffer b;
	b.basicInsertText(0, (wchar_t *)text.data(), text.size());
	return textWidth(0, b.size(), /*b.size(),*/ &b);
}
int EditView::textWidth(pos_t first, ssize_t sz, /*pos_t last,*/ const Buffer* pbuffer) const
{
	if( !sz ) return 0;
	if( pbuffer == 0 ) pbuffer = buffer();
	QFontMetrics fm = fontMetrics();
	//QFontMetrics fmBold(m_fontBold);
	int nTab = document()->typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	//int tabWidth = fm.width(QString(nTab, QChar(' ')));
	int wd = 0;
#if	1
	auto chWidth = m_fontWidth;		//fm.width(QString("8"));
	const auto endpos = first + sz;
	auto pos = first;
	int col = 0;
	while( pos != endpos ) {
		if (pos >= pbuffer->size())
			break;
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
const TextCursor* EditView::textCursor() const
{
	return m_textCursor;
}
int EditView::offsetToPx(int dln, int offset) const
{
	return textWidth(lineStartPosition(dln), offset /*, lineStartPosition(dln+1)*/);
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
	int offset = 0, lastOffset;
	const Buffer* buf = buffer();
	while( wd < px && pos < nextLinePos ) {
		lastOffset = offset;
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
		} else if( isSrgtPirFirstChar(ch) && isSrgtPirSecondChar(buf->operator[](pos+1)) ) {
			col += 4;
			wd += chWidth * 4;
			++offset;
			++pos;
		} else {
			col += 2;
			wd += chWidth * 2;
		}
		++offset;
		++pos;
	}
	return px < wd ? lastOffset : offset;
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
//	clmn：0 オリジン
int EditView::columnToPos(int vln, int clmn) const
{
	int dln = viewLineToDocLine(vln);
	pos_t pos = lineStartPosition(dln);
	if( !clmn ) return pos;
	pos_t last = posEOS(dln);
	const int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int c = 0;
	while( c < clmn && pos < last ) {
		if( charAt(pos++) == '\t' ) {
			c += nTab - c % nTab;
			if( c > clmn )
				return pos - 1;		//	TAB 位置を返す
		} else
			++c;
	}
	return pos;
}
//	dln 行の改行位置を返す
int EditView::posEOS(int dln) const
{
	pos_t first = lineStartPosition(dln);
	pos_t last = lineStartPosition(dln+1);
	while( last > first && isNewLineChar(charAt(last-1)) )
		--last;
	return last;
}
void EditView::setMark(pos_t pos, char ch)
{
	//##document()->setMark(pos, ch);
}
//	カーソル位置をマーク or マーク解除
void EditView::markSetUnset()
{
	//##document()->markSetUnset(m_textCursor->position());
	update();
}
void EditView::clearMark(pos_t pos)
{
	//##document()->markSetUnset(pos);
	update();
}
//	次のマーク位置に移動（非ループ）
bool EditView::nextMark(bool fromBeinDoc)
{
#if 0		//##
	std::vector<MarkItem> lst;
	document()->getMarks(lst);
	if( lst.empty() ) return false;
	const pos_t pos = fromBeinDoc ? -1 : m_textCursor->position();
	for(int i = 0; i < lst.size(); ++i) {
		if( lst[i].m_pos > pos ) {
			m_textCursor->setPosition(lst[i].m_pos);
			makeCursorInView(true);
			update();
			return true;
		}
	}
#endif
	return false;
}
bool EditView::prevMark(bool fromEndDoc)
{
#if 0		//##
	std::vector<MarkItem> lst;
	document()->getMarks(lst);
	if( lst.empty() ) return false;
	const pos_t pos = fromEndDoc ? bufferSize() + 1 : m_textCursor->position();
	for(int i = lst.size(); --i >= 0; ) {
		if( lst[i].m_pos < pos ) {
			m_textCursor->setPosition(lst[i].m_pos);
			makeCursorInView();
			update();
			return true;
		}
	}
#endif // 0

	return false;
}
bool EditView::jumpMarkPos(char ch)
{
	pos_t pos = buffer()->markPos(ch);
	if( pos < 0 ) return false;
	m_textCursor->setPosition(pos);
	return true;
}
void EditView::curTopOfScreen(bool vi, int n)
{
	int vln = m_scrollY0;
	m_textCursor->setPosition(viewLineStartPosition(vln + n - 1));
	if( vi )
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	makeCursorInView();
	resetCursorBlinking();
	update();
}
void EditView::curMiddleOfScreen(bool vi)
{
	QRect rct = rect();
	int vlnTop = m_scrollY0;
	int vln = vlnTop + rct.height() / lineHeight() / 2;
	m_textCursor->setPosition(viewLineStartPosition(vln));
	if( vi )
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	makeCursorInView();
	resetCursorBlinking();
	update();
}
void EditView::curBottomOfScreen(bool vi, int n)
{
	QRect rct = rect();
	int vlnTop = m_scrollY0;
	int vln = vlnTop + rct.height() / lineHeight() - n;
	m_textCursor->setPosition(viewLineStartPosition(vln));
	if( vi )
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	makeCursorInView();
	resetCursorBlinking();
	update();
}
bool  EditView::getSelectedLineRange(int &ln1, int &ln2) const
{
	if( !m_textCursor->hasSelection() )
		return false;
	else
		return m_textCursor->getSelectedLineRange(ln1, ln2);
}
QStringList merge(const QStringList &sl, const QStringList &sl2)
{
	QSet<QString> set;
	QStringList lst = sl;
	for (int i = 0; i < sl.size(); ++i) {
		set += sl[i];
	}
	for (int i = 0; i < sl2.size(); ++i) {
		if( !set.contains(sl2[i]) )
			lst += sl2[i];
	}
	return lst;
}
void EditView::insertTextSub(QString text, bool ctrl, bool shift, bool alt)
{
#if	1
	if( text.isEmpty() )
		return;
	if( text == "\t" && shift ) {
		int ln1, ln2;
		getSelectedLineRange(ln1, ln2);
		revIndent(ln1, ln2);
		return;
	}
	//qDebug() << text;
	if( viEngine()->mode() == Mode::COMMAND ) {
		viEngine()->processCommandText(text, m_textCursor->hasSelection());	//##
		return;
	}
	//const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	//const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	//const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	const bool stmntCmpl = typeSettings()->boolValue(TypeSettings::STATEMENT_COMPLETION);
	const bool wordCmpl = typeSettings()->boolValue(TypeSettings::WORD_COMPLETION);
	//const bool stmntCmpl = globSettings()->boolValue(GlobalSettings::STATEMENT_COMPLETION);
	//const bool wordCmpl = globSettings()->boolValue(GlobalSettings::WORD_COMPLETION);
	pos_t pos = m_textCursor->position();
	pos_t lastTokenPos, pos2;
	//pos_t ls = viewLineStartPosition(m_textCursor->viewLine());
	QString text0 = text;
	TypeSettings *ts = typeSettings();
	const QString typeName = ts->name();
	int ln = positionToLine(m_textCursor->position());
	uint flags = buffer()->lineFlags(ln);
	if( (flags & Buffer::LINEFLAG_IN_SCRIPT) != 0 )
		ts = mainWindow()->typeSettingsForType("JS");
	else if( (flags & Buffer::LINEFLAG_IN_PHP) != 0 )
		ts = mainWindow()->typeSettingsForType("PHP");
	const bool isCPPType = typeName == "CPP"
										|| typeName == "C#"
										|| typeName == "HLSL"
										|| typeName == "JAVA"
										|| typeName == "JS"
										|| typeName == "PHP";
	bool ai = false;		//	auto indent
	if( m_autoCompletionDlg != 0 ) {
		if( text == "\r" || text == "\n" ) {
			text = m_autoCompletionDlg->text();
			closeAutoCompletionDlg();
			int d = m_textCursor->position() - m_autoCmplPos;
			text = text.mid(d);
		} else {
			m_autoCompletionDlg->appendFilterText(text);
			//if( !m_autoCompletionDlg->count() )
			//	closeAutoCompletionDlg();
		}
	} else if( text == "\r" || text == "\n" ) {
		//	auto-indent テキスト取得
		ai = true;
		text = autoIndentText(/*isCPPType*/);
		if( !m_textCursor->hasSelection() && lineStartPosition(ln) != m_textCursor->position() ) {
			//	カーソル以降の空白類削除
			while( isSpaceChar(charAt(m_textCursor->position())) )
				m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR);
		}
	} else if( text == "\t" ) {
		if( m_textCursor->hasSelection() ) {
			int ln1, ln2;
			getSelectedLineRange(ln1, ln2);
			if( !shift )
				indent(ln1, ln2);
			else
				revIndent(ln1, ln2);
			//onCursorPosChanged();
			return;
		}
	} else if( isCPPType && text == "#" ) {
		//##insertSharp();
	} else if( isCPPType && text == "}" ) {
		insertCloseCurl(text);
	} else if( stmntCmpl && text == "{" && typeName == "CPP"
					&& isAfter(lastTokenPos, "do") )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "do", "CPP");
		//setupDo(lst);
		showAutoCompletionDlg(lst, "do {");
	} else if( isCPPType && text == "{" ) {
		//##insertOpenCurl(text);
	} else if( typeName == "CSS" && text == "{" ) {
		m_autoCmplPos = m_textCursor->position();
		QStringList lst;
		setupCandidates(lst, "{", "CSS");
		showAutoCompletionDlg(lst, "{");
	} else 
#if	0	//##
	if( !isBoxSelectionMode() && isMultilineSelected() && text == " " ) {
		insertSpaces(shift);
		return;
	} else
#endif
	if( isCPPType && text == " " ) {
		//##insertCaseSpace(text);
	//} else if( text[0].unicode() == 0x08 && ctrl ) {	//	Ctrl + H
	//	ctrl = false;	//	ちょっと反則行為
	//	goto BackSpace;
	} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "for")
					&& isCPPType )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "for", typeName);
		//setupFor(lst);
		showAutoCompletionDlg(lst, "for (");
	} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "foreach")
					&& typeName == "C#" )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "foreach", typeName);
		//setupFor(lst);
		showAutoCompletionDlg(lst, "foreach (");
	} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "if")
					&& isCPPType )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "if", "CPP");
		//setupIf(lst);
		showAutoCompletionDlg(lst, "if (");
	} else if( stmntCmpl && text == "("
					&& isCPPType && isAfter(lastTokenPos, "while") )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "while", "CPP");
		//setupWhile(lst);
		showAutoCompletionDlg(lst, "while (");
	} else if( stmntCmpl && text == "("
					&& isCPPType && isAfter(lastTokenPos, "main") )
	{
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "main", "CPP");
		//setupMain(lst);
		showAutoCompletionDlg(lst, "main(");
	} else if( stmntCmpl && text == "<" && isAfterInclude() ) {
		m_autoCmplPos = pos;
		QStringList lst;
		setupLibNames(lst);
		showAutoCompletionDlg(lst, "<");
	} else if( stmntCmpl && text == "\"" && isAfterInclude() ) {
		m_autoCmplPos = pos;
		QStringList lst;
		setupHeaders(lst, pos, text);
		if( !lst.isEmpty() )
			showAutoCompletionDlg(lst, "\"", false, /*CaseSensitive:*/false);
	} else if( stmntCmpl && text == "/" && isAfterIncludeDQ(pos2) ) {
		m_autoCmplPos = pos;
		QStringList lst;
		setupHeaders(lst, pos2, text);
		if( !lst.isEmpty() )
			showAutoCompletionDlg(lst, "\"");
	} else if( stmntCmpl && typeName == "HTML"
		&& text == "/" &&  charAt(pos-1) == '<' )
	{
		m_autoCmplPos = pos - 1;
		QStringList lst;
		setupCloseTags(lst);
		if( !lst.isEmpty() )
			showAutoCompletionDlg(lst, "</", true);
	} else if( text[0].unicode() < 0x20 || ctrl ) {
		return;		//	コントロールコードは挿入しない
	} else if( m_textCursor->hasSelection() /*##&& !isBoxSelectionMode()*/ ) {
		if( text == "(" ) {
			insertText("(", ")");
			return;
		} else if( text == "[" ) {
			insertText("[", "]");
			return;
		} else if( text == "{" ) {
			insertText("{", "}");
			return;
		}
	}
	//##if( !m_noDeleteAnimation && !isBoxSelectionMode() )
	//##	setupParabolicChars();
	m_noDeleteAnimation = false;
	//##if( !isBoxSelectionMode() && mainWindow()->insMode() == MODE_REP ) {
	//##	m_textCursor->selectOverWriteText(text);
	//##	//m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, text.size());
	//##}
	if( !editForVar(text) ) {
		if( globSettings()->boolValue(GlobalSettings::VI_COMMAND) )
			viEngine()->processCommandText(text, m_textCursor->hasSelection());
		else
			m_textCursor->insertText(text);		//	文字挿入
	}
	//emit textInserted(text0);
	QStringList lst, klst;
	QString key;
	if( wordCmpl && m_autoCompletionDlg == 0
		&& text.size() == 1 &&
			(isLetterOrNumberOrUnderbar(text[0]) || typeName == "CSS" && text == "-") )
	{
		const bool kwCompletion = typeSettings()->boolValue(TypeSettings::KEYWORD_COMPLETION);
		//const bool kwCompletion = globSettings()->boolValue(GlobalSettings::KEYWORD_COMPLETION);
		if( setupWord(lst, key, lastTokenPos) && isLetterOrUnderbar(key[0]) ) {
			m_autoCmplPos = lastTokenPos;
			if (kwCompletion) {
				if( setupKeywordsCandidates(klst, key) ) {
					if( klst.indexOf("for") >= 0 )
						setupCandidates(klst, "for", typeName);
					if( klst.indexOf("foreach") >= 0 )
						setupCandidates(klst, "foreach", typeName);
					else if( klst.indexOf("if") >= 0 )
						setupCandidates(klst, "if", typeName);
					else if( klst.indexOf("switch") >= 0 )
						setupCandidates(klst, "switch", typeName);
					else if( klst.indexOf("case") >= 0 )
						setupCandidates(klst, "case", typeName);
					else if( klst.indexOf("while") >= 0 )
						setupCandidates(klst, "while", typeName);
					else if( klst.indexOf("do") >= 0 )
						setupCandidates(klst, "do", typeName);
					else if( klst.indexOf("program") >= 0 )
						setupCandidates(klst, "program", typeName);
					else if( klst.indexOf("repeat") >= 0 )
						setupCandidates(klst, "repeat", typeName);
					else if( klst.indexOf("begin") >= 0 )
						setupCandidates(klst, "begin", typeName);
					lst = merge(klst, lst);		//	重複を削除しつつマージ
				}
			}
			showAutoCompletionDlg(lst, key);
			m_autoCmplAtBegWord = key == text;
		} else if( !key.isEmpty() && kwCompletion && setupKeywordsCandidates(klst, key) ) {
			if( klst.indexOf("for") >= 0 )
				setupCandidates(klst, "for", typeName);
			if( klst.indexOf("foreach") >= 0 )
				setupCandidates(klst, "foreach", typeName);
			else if( klst.indexOf("if") >= 0 )
				setupCandidates(klst, "if", typeName);
			else if( klst.indexOf("switch") >= 0 )
				setupCandidates(klst, "switch", typeName);
			else if( klst.indexOf("case") >= 0 )
				setupCandidates(klst, "case", typeName);
			else if( klst.indexOf("while") >= 0 )
				setupCandidates(klst, "while", typeName);
			else if( klst.indexOf("do") >= 0 )
				setupCandidates(klst, "do", typeName);
			else if( klst.indexOf("program") >= 0 )
				setupCandidates(klst, "program", typeName);
			else if( klst.indexOf("repeat") >= 0 )
				setupCandidates(klst, "repeat", typeName);
			else if( klst.indexOf("begin") >= 0 )
				setupCandidates(klst, "begin", typeName);
			pos_t pos = m_textCursor->position();
			m_autoCmplPos = pos - key.size();
			qDebug() << m_autoCmplPos;
			showAutoCompletionDlg(klst, key);
			m_autoCmplAtBegWord = key == text;
		}
	}
#if	0
	if( m_autoCompletionDlg != 0 && m_autoCompletionDlg->count() == 1 ) {
		QString t = getText(*buffer(), m_autoCmplPos, m_textCursor->position() + 1);
		QString t2 = m_autoCompletionDlg->text();
		if( t == m_autoCompletionDlg->text() )
			closeAutoCompletionDlg();
	}
#endif
	if( ai ) {
		m_aiCurPos = m_textCursor->position();
		m_aiSpaces = text.mid(newLineText().size());
	} else
		m_aiCurPos = -1;
	QString openParen;
	if( text == ")" )
		openParen = "(";
	else if( text == "}" )
		openParen = "{";
	else if( text == "]" )
		openParen = "[";
	m_unbalancedAssocParen = false;
	if( !openParen.isEmpty() ) {
		m_closeParenPos = m_textCursor->position() - 1;
		m_closeParenViewLine = m_textCursor->viewLine();
		m_openParenPos = assocParenPositionBackward(typeSettings(), *buffer(), m_closeParenPos, text[0].unicode(), openParen[0].unicode());
		m_openParenViewLine = docLineToViewLine(positionToLine(m_openParenPos));
	} else {
		clearOpenCloseParenPos();
		//m_openParenPos = m_closeParenPos = -1;
		checkAssocParen();
	}
	updateScrollBarInfo();
#else
	if( text.isEmpty() ) return;
	if( text != "\t" && mainWindow()->mode() == MODE_VI ) {
		viEngine()->processCommandText(text, m_textCursor->hasSelection());
		return;
	}
	bool ai = false;
	const bool stmntCmpl = typeSettings()->boolValue(TypeSettings::STATEMENT_COMPLETION);
	const bool wordCmpl = typeSettings()->boolValue(TypeSettings::WORD_COMPLETION);
	const QString typeName = typeSettings()->name();
	const bool isCPPType = typeName == "CPP"
										|| typeName == "C#"
										|| typeName == "HLSL"
										|| typeName == "JAVA"
										|| typeName == "JS"
										|| typeName == "PHP";
	int ln = positionToLine(m_textCursor->position());
	pos_t pos = m_textCursor->position();
	pos_t lastTokenPos, pos2;
	if( m_autoCompletionDlg != 0 ) {	//	補完ダイアログ表示時
	} else if( text == "\t" ) {
		if( m_textCursor->hasSelection() ) {
			int ln1, ln2;
			getSelectedLineRange(ln1, ln2);
			if( !shift )
				indent(ln1, ln2);
			else
				revIndent(ln1, ln2);
			return;
		}
	} else if( text[0] == '\n' || text[0] == '\r' ) {
		//	auto-indent テキスト取得
		ai = true;
		text = autoIndentText(/*isCPPType*/);
		if( !m_textCursor->hasSelection() && lineStartPosition(ln) != m_textCursor->position() ) {
			//	カーソル以降の空白類削除
			while( isSpaceChar(charAt(m_textCursor->position())) )
				m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR);
		}
	} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "for") && isCPPType ) {
		m_autoCmplPos = lastTokenPos;
		QStringList lst;
		setupCandidates(lst, "for", typeName);
		//setupFor(lst);
		showAutoCompletionDlg(lst, "for (");
	}
		if( globSettings()->boolValue(GlobalSettings::VI_COMMAND) )
			viEngine()->processCommandText(text, m_textCursor->hasSelection());
		else
			m_textCursor->insertText(text);		//	文字挿入
#endif
}
void EditView::clearOpenCloseParenPos()
{
	m_openParenPos = m_closeParenPos = -1;		//	対応括弧強調OFF
	m_openParenViewLine = m_closeParenViewLine = -1;
}
bool EditView::isSpaceText(pos_t first, pos_t last)
{
	while( first < last ) {
		if( !isSpaceChar(charAt(first)) )
			return false;
		++first;
	}
	return true;
}
//	} が押された場合の処理
void EditView::insertCloseCurl(QString &text)
{
	if( m_textCursor->hasSelection() ) return;
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	pos_t ls = lineStartPosition(ln);
	if( !isSpaceText(ls, pos) ) return;		//	} より左に空白類以外があればリターン
	const Buffer &buf = *buffer();
	int apos = assocParenPositionBackward(typeSettings(), buf, pos, '}', '{');
	if( apos < 0 ) return;		//	対応する括弧が無い
	ln = document()->positionToLine(apos);
	ls = lineStartPosition(ln);
	int i = ls;
	while( isSpaceChar(buf[i]) ) ++i;
	if( i > ls )
		text = getText(buf, ls, i - ls) + text;		//	参照行のインデントテキストを前に付加
	m_textCursor->movePosition(TextCursor::BEG_LINE, TextCursor::KEEP_ANCHOR);		//	行頭から選択し、置換
}
QString EditView::indentText(int ln)
{
	return "";
}
QString EditView::autoIndentText(/*bool,*/ bool nxline)
{
	QString text = ::autoIndentText(typeSettings(),
							*buffer(),
							m_textCursor->position(),
							(cwchar*)L"\n",
							//(cwchar *)newLineText().data(),
							nxline /*, bCPP*/);
	return text;
}
void EditView::indent(int ln1, int ln2, bool vi)
{
	//##if( !viEngine()->isUndoBlockOpened() )
		openUndoBlock();
	for(int ln = ln1; ln <= ln2; ++ln) {
		pos_t pos = lineStartPosition(ln);
		document()->insertText(pos, QString((QChar *)L"\t"));
	}
	//##if( !viEngine()->isUndoBlockOpened() )
		closeUndoBlock();
	if( !vi ) {
		//##m_textCursor->setMode(TextCursor::NOMAL_MODE);
		m_textCursor->setPosition(lineStartPosition(ln1));
		m_textCursor->setPosition(lineStartPosition(ln2+1), TextCursor::KEEP_ANCHOR);
	}
	update();
}
void EditView::revIndent(int ln1, int ln2, bool vi)
{
	int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	//##if( !viEngine()->isUndoBlockOpened() )
		openUndoBlock();
	for(int ln = ln1; ln <= ln2; ++ln) {
		const pos_t ls = lineStartPosition(ln);
		pos_t pos = ls;
		if( charAt(pos) == '\t' )
			document()->deleteText(pos);
		else {
			int nsp = 0;	//	行頭空白数
			while( nsp < nTab && charAt(pos+nsp) == ' ' )
				++nsp;
			if( nsp != 0 )
				document()->deleteText(ls, nsp);
		}
	}
	//##if( !viEngine()->isUndoBlockOpened() )
		closeUndoBlock();
	if( !vi ) {
		//##m_textCursor->setMode(TextCursor::NOMAL_MODE);
		m_textCursor->setPosition(lineStartPosition(ln1));
		m_textCursor->setPosition(lineStartPosition(ln2+1), TextCursor::KEEP_ANCHOR);
	}
	update();
}
void EditView::replaceText(const QString &text)
{
	if( text.isEmpty() ) return;
	m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, text.size(), /*vi:*/true);
	insertText(text);
}
void EditView::insertText(const QString &text0)
{
	QString text = text0;
#if	0
	//	done: 選択されている部分と text の先頭が一致している場合対応
	if( m_textCursor->hasSelection() ) {
		QString t = m_textCursor->selectedText();
		if( text.startsWith(t) ) {
			m_textCursor->setPosition(m_textCursor->selectionLast());		//	選択末尾に移動
			text = text.mid(t.size());
		}
	}
#endif
	m_textCursor->insertText(text);
	emit textInserted(text);
	updateScrollBarInfo();
	makeCursorInView();
	resetCursorBlinking();
	update();
}
//	選択領域の前後に文字挿入
void EditView::insertText(const QString &text1, const QString &text2)
{
	pos_t first = m_textCursor->selectionFirst();
	pos_t last = m_textCursor->selectionLast();
	openUndoBlock();
	m_textCursor->setPosition(last);
	m_textCursor->insertText(text2);
	m_textCursor->setPosition(first);
	m_textCursor->insertText(text1);
	closeUndoBlock();
	m_textCursor->setPosition(first);
	m_textCursor->setPosition(last + text1.size() + text2.size(), TextCursor::KEEP_ANCHOR);
	updateScrollBarInfo();
	makeCursorInView();
	resetCursorBlinking();
	update();
}
void EditView::insertTextRaw(pos_t pos, const QString &text)
{
	document()->insertText(pos, text);
	update();
}
void EditView::onBackSpace(bool ctrl, bool shift, bool alt)
{
	viEngine()->onBackSpace();
#if	1
	if( isBoxSelectMode() ) {
	} else if( !m_textCursor->hasSelection() ) {
		if( ctrl ) {
			m_textCursor->movePosition(TextCursor::PREV_WORD, TextCursor::KEEP_ANCHOR);
		} else if( shift ) {
			m_textCursor->movePosition(TextCursor::BEG_LINE, TextCursor::KEEP_ANCHOR);
		} else
			m_textCursor->movePosition(TextCursor::LEFT, TextCursor::KEEP_ANCHOR);
	}
	if( !m_textCursor->hasSelection() ) return;
	//##if( !isBoxSelectMode() )
		//setupFallingChars();
	if( !editForVar(QString()) )
		m_textCursor->deleteChar(/*BS=*/true, /*vi:*/false, /*fall=*/true);
#if	1
	if( m_autoCompletionDlg != 0 ) {
		if( m_textCursor->position() <= m_autoCmplPos )
			closeAutoCompletionDlg();
		else {
			QString ft = getText(*buffer(), m_autoCmplPos, m_textCursor->position() - m_autoCmplPos);
			m_autoCompletionDlg->setFilterText(ft);
		}
	}
#endif
	emit textBackSpaced();
#else
	if( !m_textCursor->hasSelection() ) {
			m_textCursor->movePosition(TextCursor::LEFT /*, TextCursor::KEEP_ANCHOR*/);
	}
	if( !editForVar(QString()) )
		m_textCursor->deleteChar(/*BS=*/true);
#endif
}
void EditView::onDelete(bool ctrl, bool shift, bool alt)
{
	//##if( m_autoCompletionDlg != 0 )
	//##	closeAutoCompletionDlg();
	if( !m_textCursor->hasSelection() ) {
		if( ctrl ) {
			if( shift )
				m_textCursor->movePosition(TextCursor::NEXT_CAP_WORD, TextCursor::KEEP_ANCHOR);
			else
				m_textCursor->movePosition(TextCursor::NEXT_WORD, TextCursor::KEEP_ANCHOR);
		} else {
			if( shift ) {
				m_textCursor->movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
			} else
				m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR);
		}
	}
	if( !m_textCursor->hasSelection() ) return;
	//setupFallingChars();
	if( !editForVar(QString()) )
		m_textCursor->deleteChar(/*BS=*/false, /*vi:*/false, /*fall=*/true);
	//##emit boxSelectModeChanged(m_textCursor->isBoxSelectMode());
}
void EditView::deleteText(pos_t pos, ssize_t sz, bool BS, bool fall)
{
	if( fall )
		setupFallingChars();
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
	//##m_textCursor->setBoxSelectMode(false);
	m_textCursor->setPosition(pos);
	checkAssocParen();
	updateScrollBarInfo();
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
	//##m_textCursor->setBoxSelectMode(false);
	m_textCursor->setPosition(pos);
	checkAssocParen();
	updateScrollBarInfo();
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
void EditView::appendCut()
{
	cut(true);
}
int EditView::appendCopy()
{
	return copy(false, true);
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
	setupFallingChars();
	m_textCursor->deleteChar(/*BS=*/false, /*vi:*/false, /*fall=*/true);
	if( !im )
		emit modifiedChanged();
	updateScrollBarInfo();
	makeCursorInView();
	update();
	emit updateUndoRedoEnabled();
}
int EditView::copy(bool bCut, bool append)
{
#if	0
	qDebug() << "m_lineHeight = " << m_lineHeight;
	qDebug() << "m_fontHeight = " << m_fontHeight;
	QFontMetrics fm(m_font);
	const auto descent = fm.descent();
	qDebug() << "descent = " << descent;
#endif
	//
	if( !m_textCursor->hasSelection() ) return 0;
	auto first = m_textCursor->selectionFirst();
	auto last = m_textCursor->selectionLast();
	auto sz = last - first;
	document()->copy(first, sz, append);
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
	setupFallingChars();
#if	1
	bool im = isModified();
	m_textCursor->insertText(text);
	if( !im )
		emit modifiedChanged();
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
void EditView::openUndoBlock()
{
	buffer()->openUndoBlock();
}
void EditView::closeUndoBlock()
{
	buffer()->closeUndoBlock();
}
void EditView::closeAllUndoBlock()
{
	buffer()->closeAllUndoBlock();
}
void EditView::selectAll()
{
	m_textCursor->setPosition(0);
	m_textCursor->setPosition(bufferSize(), TextCursor::KEEP_ANCHOR);
	makeCursorInView();
	update();
}
bool EditView::saveFile() const
{
	bool im = isModified();
	bool rc = document()->saveFile();
	if( rc && im )
		emit modifiedChanged();
	return rc;
}
bool EditView::searchCurWord(QString &txt, bool vi)
{
	const bool hadSelection = m_textCursor->hasSelection();
	if( hadSelection ) {
		int ln1 = positionToLine(m_textCursor->selectionFirst());
		int ln2 = positionToLine(m_textCursor->selectionLast());
		if( ln1 != ln2 ) m_textCursor->clearSelection();
	}
	if( !m_textCursor->hasSelection() ) {
		m_textCursor->movePosition(TextCursor::BEG_WORD);
		m_textCursor->movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
	}
	if( !m_textCursor->hasSelection() ) return false;
	pos_t first, last;
	int ln1 = positionToLine(first = m_textCursor->selectionFirst());
	int ln2 = positionToLine(last = m_textCursor->selectionLast());
	if( ln1 != ln2 ) return false;
	txt = document()->text(first, last - first);
	//mainWindow()->setFindString(txt);
	//mainWindow()->setMatchedString(txt);
	if( txt.isEmpty() ) return false;
	//uint opt = 0;
#if	1	//##
	if( vi || globSettings()->boolValue(GlobalSettings::REGEXP) ) {
		txt = escapeRegExpSpecialChars(txt);
	}
#endif
	if (!hadSelection && vi && !txt.isEmpty() ) {
		//txt = "\\b" + txt + "\\b";
		if( txt[0].unicode() < 0x100 ) txt = "\\b" + txt;
		if( txt.back().unicode() < 0x100 ) txt += "\\b";
	}
	mainWindow()->setFindString(txt);
	mainWindow()->setMatchedString(txt);
	mainWindow()->setSearchWordOpt(!hadSelection);		//	非選択状態ならば、単語単位検索オプションON
	auto opt = mainWindow()->getSearchOpt();
	//opt |= SSSearch::WHOLE_WORD_ONLY;
	bool rc = findForward(txt, opt, false, true, vi);
	//emit textSearched(txt, !vi);
	resetCursorBlinking();
	update();
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
	sssrc.setup((wchar_t *)text.data(), text.size());
	update();
	qDebug() << "findForward() pos = " << pos;
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
bool EditView::focusNextPrevChild(bool next)
{
	return false;
}
void EditView::setModified(bool b)
{
	if( b == isModified() ) return;
	document()->setModified(b);
	emit modifiedChanged();
}
void EditView::scrollDown()
{
	int v = m_scrollY0;
	m_scrollY0 = (++v);
	update();
}
void EditView::scrollUp()
{
	int v = m_scrollY0;
	if( v == 0 ) return;
	m_scrollY0 = (--v);
	update();
}
void EditView::scrollDownPage(bool shift)
{
	const int nLines = rect().height() / lineHeight();
	int v = m_scrollY0;
	m_scrollY0 = (v + nLines);
	int vln = qMin((size_t)m_textCursor->viewLine() + nLines, (size_t)viewLineCount());
	pos_t pos = viewLineStartPosition(vln);
	int mode = shift ? TextCursor::KEEP_ANCHOR : TextCursor::MOVE_ANCHOR;
	m_textCursor->setPosition(pos, mode);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mode);
	update();
}
void EditView::scrollUpPage(bool shift)
{
	const int nLines = rect().height() / lineHeight();
	int v = m_scrollY0;
	m_scrollY0 = (v - nLines);
	int vln = qMax(m_textCursor->viewLine() - nLines, 0);
	pos_t pos = viewLineStartPosition(vln);
	int mode = shift ? TextCursor::KEEP_ANCHOR : TextCursor::MOVE_ANCHOR;
	m_textCursor->setPosition(pos, mode);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mode);
	update();
}
void EditView::scrollDownHalfPage()
{
	const int nLines = rect().height() / lineHeight();
	int v = m_scrollY0;
	m_scrollY0 = (v + nLines / 2);
	int vln = qMin(m_textCursor->viewLine() + nLines / 2, viewLineCount());
	pos_t pos = viewLineStartPosition(vln);
	m_textCursor->setPosition(pos);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	update();
}
void EditView::scrollUpHalfPage()
{
	const int nLines = rect().height() / lineHeight();
	int v = m_scrollY0;
	m_scrollY0 = (v - nLines / 2);
	int vln = qMax(m_textCursor->viewLine() - nLines / 2, 0);
	pos_t pos = viewLineStartPosition(vln);
	m_textCursor->setPosition(pos);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	update();
}
void EditView::exposeBottomOfScreen()
{
	int v = m_scrollY0;
	m_scrollY0 = (v + 1);
	int vln = qMin(m_textCursor->viewLine() + 1, viewLineCount());
	pos_t pos = viewLineStartPosition(vln);
	m_textCursor->setPosition(pos);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	update();
}
void EditView::exposeTopOfScreen()
{
	int v = m_scrollY0;
	m_scrollY0 = (v - 1);
	int vln = qMax(m_textCursor->viewLine() - 1, 0);
	pos_t pos = viewLineStartPosition(vln);
	m_textCursor->setPosition(pos);
	m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	update();
}
void EditView::scrollCurTopOfScreen()
{
	int ln = document()->positionToLine(m_textCursor->position());
	if( ln == m_scrollY0 ) {
		scrollCurCenterOfScreen();
	} else {
		int ln2 = document()->positionToLine(m_textCursor->position())
					- rect().height() / lineHeight() / 2;
		if( ln2 < 0 ) ln2 = 0;
		if( ln2 == m_scrollY0 ) {
			scrollCurBottomOfScreen();
		} else {
			m_scrollY0 = (ln);
			update();
		}
	}
}
void EditView::scrollCurQuarterOfScreen()
{
	int ln = document()->positionToLine(m_textCursor->position())
				- rect().height() / lineHeight() / 4;
	if( ln < 0 ) ln = 0;
	m_scrollY0 = (ln);
	update();
}
void EditView::scrollCurCenterOfScreen()
{
	int ln = document()->positionToLine(m_textCursor->position())
				- rect().height() / lineHeight() / 2;
	if( ln < 0 ) ln = 0;
	m_scrollY0 = (ln);
	update();
}
void EditView::scrollCurBottomOfScreen()
{
	int ln = document()->positionToLine(m_textCursor->position())
				- rect().height() / lineHeight() + 2;
	if( ln < 0 ) ln = 0;
	m_scrollY0 = (ln);
	update();
}
void EditView::joinLines(int nLines, bool vi)
{
	pos_t firstPos;
	int firstLine, lastLine;
	int newPos;	//	for vi
	if( m_textCursor->hasSelection() ) {
		firstPos = m_textCursor->selectionFirst();
		firstLine = positionToLine(firstPos) ;
		pos_t last = m_textCursor->selectionLast();
		lastLine = positionToLine(last);
		if( last == lineStartPosition(lastLine) ) --lastLine;		//	行頭の場合は、直前行までとする
	} else {
		firstLine = positionToLine(firstPos = m_textCursor->position());
		lastLine = firstLine + qMax(nLines - 1, 1);
	}
	if( vi )
		newPos = endOfLinePosition(firstLine);
	QTime tm;
	tm.start();
	openUndoBlock();
	int dln = viewLineToDocLine(firstLine);
	while( firstLine <= --lastLine ) {
		pos_t pos2 = lineStartPosition(lastLine+1);
		pos_t pos1 = pos2;
		while( pos1 > 0 && isNewLineChar(charAt(pos1 - 1)) ) --pos1;
		while( isSpaceChar(charAt(pos2)) ) ++pos2;
#if	0
		//	TextCursor::insertText() を繰り返し呼ぶと時間がかかる場合があるので、
		//	ドキュメント or バッファの挿入関数を直接呼ぶ方がよい
		buffer()->replaceText(pos1, pos2 - pos1, L" ", 1, dln);
#else
		m_textCursor->setPosition(pos1);
		m_textCursor->setPosition(pos2, TextCursor::KEEP_ANCHOR);
		m_textCursor->insertText(" ");
		//m_textCursor->deleteChar();
#endif
	}
	closeUndoBlock();
	updateScrollBarInfo();
	if( vi )
		m_textCursor->setPosition(newPos);
	else {
		m_textCursor->setPosition(firstPos);
		m_textCursor->movePosition(TextCursor::HOME_LINE);
	}
	makeCursorInView();
	update();
	int ms = tm.elapsed();
	showMessage(QString("%1 elapsed.").arg(ms/1000.0), 5000);

}
int EditView::endOfLinePosition(int dln) const		//	行の改行位置を返す
{
	pos_t ls = lineStartPosition(dln);
	pos_t pos = lineStartPosition(dln+1);
	while( pos > ls && isNewLineChar(charAt(pos - 1)) )
		--pos;
	return pos;
}
//	インクリメント・デクリメント
void EditView::incDec(bool bInc, int d)
{
	pos_t pos = m_textCursor->position();
	if( m_textCursor->hasSelection() ) {
		pos = m_textCursor->selectionFirst();
		m_textCursor->clearSelection();
	}
	wchar_t ch = charAt(pos);
	if( isDigit(ch) ) {
		while( pos > 0 && isDigit(charAt(pos-1)) ) --pos;
		pos_t first = pos;
		pos = m_textCursor->position();;
		while( pos < bufferSize() && isDigit(charAt(pos)) ) ++pos;
		pos_t last = pos;
		//QString txt = getText(*buffer(), first, last);
		m_textCursor->setPosition(first);
		m_textCursor->setPosition(last, TextCursor::KEEP_ANCHOR);
		QString txt = m_textCursor->selectedText();
		int n = txt.toInt() + (bInc ? d : -d);
		if( txt[0] == '0' )
			txt = QString("%1").arg(n, txt.size(), 10, QChar('0'));
		else
			txt = QString::number(n);
		m_noFallingChars = true;
		m_textCursor->insertText(txt);
		m_noFallingChars = false;
		m_textCursor->movePosition(TextCursor::LEFT, TextCursor::MOVE_ANCHOR, txt.size());
		makeCursorInView();
		update();
	}
}
void EditView::openPrevLine()
{
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	m_textCursor->movePosition(TextCursor::BEG_LINE);
	m_textCursor->movePosition(TextCursor::LEFT);
	//qDebug() << "viewLine = " << m_textCursor->viewLine() << ", pos = " << m_textCursor->position();
	//QString name = typeSettings()->name();
	//bool bCPP = name == "CPP" || name == "C#" || name == "JAVA" || name == "JS" || name == "PHP";
	QString itext = autoIndentText(/*bCPP,*/ false);
	m_textCursor->insertText(itext);
	if( !vln )
		m_textCursor->movePosition(TextCursor::LEFT);
	checkAssocParen();
	makeCursorInView();
	update();
}
void EditView::openNextLine()
{
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	m_textCursor->movePosition(TextCursor::END_LINE);
	//QString name = typeSettings()->name();
	//bool bCPP = name == "CPP" || name == "C#" || name == "JAVA" || name == "JS" || name == "PHP";
	QString itext = autoIndentText(/*bCPP*/);
	m_textCursor->insertText(itext);
	checkAssocParen();
	makeCursorInView();
	update();
}
void EditView::checkAssocParen()
{
	clearOpenCloseParenPos();
	checkAssocParen(m_textCursor->viewLine(), m_textCursor->position());
}
void EditView::checkAssocParen(int vln, pos_t pos)
{
	//m_openParenPos = m_closeParenPos = -1;
	wchar_t dch;
	wchar_t ch = charAt(pos);
	bool forward = true;
	switch( ch ) {
		case ')':	dch = '(';	forward = false; break;
		case '(':	dch = ')';	break;
		case '}':	dch = '{';	forward = false; break;
		case '{':	dch = '}';	break;
		case ']':	dch = '[';	forward = false; break;
		case '[':	dch = ']';	break;
		case '#': {
			int ln = positionToLine(pos);
			if( buffer()->isSpaces(lineStartPosition(ln), pos) ) {
				checkAssocSharpTag();
			}
			return;
		}
		case '<': {
			//pos_t pos = m_textCursor->position();
			if( typeSettings()->name() == "HTML" &&
				(isAlpha(charAt(pos+1)) || charAt(pos+1) == '/' && isAlpha(charAt(pos+2))) )
			{
				checkAssocSharpTag();
			}
			return;
		}
		default:
			return;
	}
	m_unbalancedAssocParen = false;
	if( forward ) {
		m_openParenPos = pos;
		m_openParenViewLine = vln;
		m_closeParenPos = assocParenPositionForward(typeSettings(), *buffer(), m_openParenPos, ch, dch);
		m_closeParenViewLine = docLineToViewLine(positionToLine(m_closeParenPos));
		m_unbalancedAssocParen = ch == '{' && m_closeParenPos < 0;
	} else {
		m_closeParenPos = pos;
		m_closeParenViewLine = vln;
		m_openParenPos = assocParenPositionBackward(typeSettings(), *buffer(), m_closeParenPos, ch, dch);
		m_openParenViewLine = docLineToViewLine(positionToLine(m_openParenPos));
		m_unbalancedAssocParen = ch == '}' && m_openParenPos < 0;
	}
	if( ch != '{' && ch != '}' || m_unbalancedAssocParen ) return;
#if		1
	pos_t pos1 = lineStartPosition(positionToLine(m_openParenPos));
	pos_t pos2 = lineStartPosition(positionToLine(m_closeParenPos));
	for(;;) {
		if( !isSpaceChar(charAt(pos1)) ) {
			m_unbalancedAssocParen = isSpaceChar(charAt(pos2));
			break;
		}
		if( charAt(pos1++) != charAt(pos2++) ) {
			m_unbalancedAssocParen = true;
			break;
		}
	}
#else
	int ln = positionToLine(m_openParenPos);
	pos_t ls = lineStartPosition(ln);
	while( isSpaceChar(charAt(ls)) ) ++ls;
	int indent1 = ls - lineStartPosition(ln);
	ln = positionToLine(m_closeParenPos);
	ls = lineStartPosition(ln);
	while( isSpaceChar(charAt(ls)) ) ++ls;
	int indent2 = ls - lineStartPosition(ln);
	m_unbalancedAssocParen = indent1 != indent2;
#endif
}
void EditView::checkAssocSharpTag()
{
	TextCursor cur(*m_textCursor);
	cur.movePosition(TextCursor::ASSOC_PAREN);
	if( cur.position() == m_textCursor->position() ) {
		clearOpenCloseParenPos();
		//m_openParenPos = m_closeParenPos = -1;
		return;
	}
	m_openParenPos = m_textCursor->position();
	m_openParenViewLine = m_textCursor->viewLine();
	m_closeParenPos = cur.position();
	m_closeParenViewLine = docLineToViewLine(positionToLine(m_closeParenPos));
	m_unbalancedAssocParen = false;
}
void EditView::toggleUpperLowerCase()
{
	//	undone: BOX選択モード対応
	if( m_textCursor->hasSelection() ) {
		QString t = m_textCursor->selectedText();
		bool toggled = false;
		for(int i = 0; i < t.size(); ++i) {
			wchar_t ch = t[i].unicode();
			if( ch >= 'A' && ch <= 'Z' )
				ch += 'a' - 'A';
			else if( ch >= 'a' && ch <= 'z' )
				ch += 'A' - 'a';
			else
				continue;
			t[i] = QChar(ch);
			toggled = true;
		}
		if( toggled )
			m_textCursor->insertText(t);
		//m_textCursor->clearSelection();
	} else {
		wchar_t ch = m_textCursor->charAt();
		if( ch >= 'A' && ch <= 'Z' )
			ch += 'a' - 'A';
		else if( ch >= 'a' && ch <= 'z' )
			ch += 'A' - 'a';
		else {
			m_textCursor->movePosition(TextCursor::RIGHT);
			update();
			return;
		}
		m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR);
		m_textCursor->insertText(QString(QChar(ch)));
	}
	update();
}
void EditView::toggleTrueFalse()
{
	TextCursor cur(*m_textCursor);
	if( !cur.hasSelection() ) {
		cur.movePosition(TextCursor::BEG_WORD);
		cur.movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
	}
	QString text = cur.selectedText();
	if( text == "true" )
		text = "false";
	else if( text == "false" )
		text = "true";
	else
		return;
	*m_textCursor = cur;
	m_textCursor->insertText(text);
	update();
}
QString EditView::getLineText(int dln) const
{
	return ::getLineText(*buffer(), dln);
	//assert(0);
	//return "";
}
uint EditView::lineFlags(int dln) const
{
	return buffer()->lineFlags(dln);
	//assert(0);
	//return 0;
}
void EditView::clearLineFlags()
{
	buffer()->clearLineFlags();
}
void EditView::substitute(int dln1, int dln2, const QString &before, const QString &after, const QString &optstr)
{
	//assert(0);
	int opt = 0;
	bool global = optstr.indexOf('g') >= 0;
	pos_t first = lineStartPosition(dln1);
	pos_t last = lineStartPosition(dln2+1);
	//	done: 折り返しモードの場合は、いったん折り返し情報をクリアし、置換後に再構築
	const bool lineBreaked = viewLineMgr()->isLineBreakMode();
	viewLineMgr()->clear();		//	＊折り返し情報クリア＊
	
	bool im = isModified();
	byte_t algorithm = mainWindow()->searchAlgorithm();
	//if( mainWindow()->globSettings()->boolValue(GlobalSettings::REGEXP) )
		algorithm = SSSearch::STD_REGEX;
	pos_t pos;
	int cnt = buffer()->replaceAll((const wchar_t *)before.data(), before.size(),
									(const wchar_t *)after.data(), after.size(),
									opt, algorithm, first, last, pos, global);
	showMessage(tr("%1 replaced.").arg(cnt), 5000);
	if( pos >= 0 )
		m_textCursor->setPosition(pos);
	if( isModified() != im )
		emit modifiedChanged();
	if( lineBreaked )
		viewLineMgr()->setLineBreak(true);
	updateScrollBarInfo();
	makeCursorInView();
	update();
}
void EditView::setCursorPosition(pos_t pos, int mode)
{
	//assert(0);
	m_textCursor->setPosition(pos, mode);
	makeCursorInView();
	update();
}
//	挿入・置換処理・削除
bool EditView::editForVar(const QString &text)
{
	if( !text.isEmpty() && !isLetterOrNumberOrUnderbar(text[0]) )
		return false;
	QString name = typeSettings()->name();
	if( name != "CPP" && name != "JAVA" && name != "JS" && name != "C#" && name != "PHP" )
		return false;
	if( !m_delForVarPos.empty()
		&& !m_textCursor->hasSelection()
		&& m_textCursor->position() == m_delForVarPos[0] )
	{
		openUndoBlock();
		for (int i = m_delForVarPos.size(); --i >= 0; ) {
			m_textCursor->setPosition(m_delForVarPos[i]);
			m_textCursor->insertText(text);
		}
		closeUndoBlock();
		m_delForVarPos.clear();
		return true;
	}
	m_delForVarPos.clear();
	pos_t first = m_textCursor->selectionFirst();
	//int selLast = m_textCursor->selectionLast;
	int vln = m_textCursor->selectionFirstLine();
	if( vln != m_textCursor->selectionLastLine() ) 
		return false;		//	複数行選択されている場合
	int ln = viewLineToDocLine(vln);
	pos_t ls = lineStartPosition(ln);
	pos_t nxls = lineStartPosition(ln+1);
	Tokenizer tkn(buffer(), ls, nxls, /*bStr*/true);
	QString token = tkn.tokenText();
	if( token != "for" || tkn.nextTokenText() != "(" ||
		tkn.nextTokenText() != "int" && tkn.tokenText() != "auto" && tkn.tokenText() != "var" )
	{
		return false;
	}
	token = tkn.nextTokenText();
	if( name == "PHP" && token == "$" )
		token = tkn.nextTokenText();
	pos_t pos = tkn.tokenPosition();
	if( first < pos ) return false;		//	変数より前から選択されていた場合
	int endpos = pos + token.size();
	if( m_textCursor->hasSelection() ) {		//	選択状態の場合
		//int first = m_textCursor->selectionFirst();
		pos_t last = m_textCursor->selectionLast();
		if( first >= endpos || last <= pos )
			return false;
		if( last > endpos )			//	変数名以降も選択されていた場合
			return false;
		std::vector<int> v;		//	マッチするトークン位置リスト
		v.push_back(pos);
		QString t;
		while (!(t = tkn.nextTokenText()).isEmpty()) {
			if( t == token && tkn.tokenPosition() >= last ) {		//	選択範囲外で等しい場合
				v.push_back(tkn.tokenPosition());
			}
		}
		openUndoBlock();
		for (int i = v.size(); --i >= 0; ) {
			m_textCursor->setPosition(v[i] + first - pos);
			m_textCursor->setPosition(v[i] + last - pos, TextCursor::KEEP_ANCHOR);
			if( text.isEmpty() )
				m_textCursor->deleteChar();
			else
				m_textCursor->insertText(text);
		}
		closeUndoBlock();
		m_delForVarPos.clear();
		if( first == pos && last == endpos && text.isEmpty() ) {		//	変数全削除の場合
			for (int i = 0; i < v.size(); ++i) {
				m_delForVarPos.push_back(v[i] - token.size() * i);
			}
		}
	} else {
		int cp = m_textCursor->position();
		if( cp < pos || cp > endpos )
			return false;
		std::vector<int> v;		//	マッチするトークン位置リスト
		v.push_back(pos);
		QString t;
		while (!(t = tkn.nextTokenText()).isEmpty()) {
			if( t == token ) {
				v.push_back(tkn.tokenPosition());
			}
		}
		openUndoBlock();
		for (int i = v.size(); --i >= 0; ) {
			m_textCursor->setPosition(v[i] + cp - pos);
			m_textCursor->insertText(text);
		}
		closeUndoBlock();
	}
	return true;
}
void setupCandidates(QStringList &lst, const QString &key, const QString &type)
{
	//lst.clear();
#ifdef		_DEBUG
	QString dir("G:/bin/sse64");
#else
	QString dir(qApp->applicationDirPath());
#endif
	QString fileName = dir + "/autoCmpl/" + type + ".txt";
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	while( !file.atEnd() ) {
		QString buf = codec->toUnicode(file.readLine()).trimmed();
		if( buf.size() > key.size() && buf.startsWith(key)
			&& !isLetterOrNumberOrUnderbar(buf[key.size()]) )
		{
			lst << buf;
		}
	}
}
bool EditView::isCppType() const
{
	const QString typeName = typeSettings()->name();
	return	typeName == "CPP"
				|| typeName == "C#"
				|| typeName == "HLSL"
				|| typeName == "JAVA"
				|| typeName == "JS"
				|| typeName == "PHP";
}
void EditView::jumpToLastModifiedPos()
{
	m_textCursor->setPosition(buffer()->lastModifiedPos());
	makeCursorInView(true);
	resetCursorBlinking();
	update();
}
//	C++ メンバ関数宣言から、実装コードスケルトン生成
void EditView::copyImplCode()
{
	pos_t last;
	pos_t pos;
	if( !m_textCursor->hasSelection() ) {
		last = (pos = m_textCursor->position()) + 1;
	} else {
		pos = m_textCursor->selectionFirst();
		last = m_textCursor->selectionLast();
	}
	QString implText;		//	実装テキスト
	while( pos < last ) {
		QString t = getImplText(pos);
		if( t.isEmpty() ) return;
		implText += t;
	}
	QClipboard *cb = qApp->clipboard();
	cb->setText(implText);
	emit textCopied(implText);
}
QString EditView::getImplText(pos_t &pos)
{	
	const Buffer &buf = *buffer();
	const int ln0 = positionToLine(pos);
	int ln = ln0;
	const int ls0 = lineStartPosition(ln);
	int ls = ls0;
	QString className;
	while( --ln >= 0 ) {
		pos_t nxls = ls;
		ls = lineStartPosition(ln);
		if( buf[ls] == 'c' ) {	
			Tokenizer tn(buf, ls, nxls);
			if( tn.tokenText() == "class"
				&& tn.nextToken() == Tokenizer::IDENT
				&& tn.skipSpace() != ';' )
			{
				className = tn.tokenText();
				break;
			}
		}
	}
	if( className.isEmpty() ) {
		QMessageBox::warning(this, "ViVi64", tr("class Name was not found."));
		return QString();
	}
	qDebug() << className;
	//	~<IDENT>"(); "	<COMMENT>
	//	<TYPE> <IDENT>"(" ... "); "	<COMMENT>
	//	<TYPE> "opetaor"<opr>"(" ... ")" "; "	<COMMENT>
	//	")" はバランスする括弧。30行先まで可
	QString implText;		//	実装テキスト
	pos_t nxls = lineStartPosition(ln0+1);
	Tokenizer tn(buf, ls0, nxls);
#if		1
	//	undone: operator[]()
	int start = tn.tokenPosition();
	for(;;) {
		if( tn.tokenText() == "~" || tn.tokenText() == "operator")
			break;
		QChar ch = tn.skipSpace();
		if( ch == '(' ) break;		//	<IDENT>"(" を発見した場合
		if( tn.nextToken() == Tokenizer::END_OF_FILE ) {
			QMessageBox::warning(this, "SakuSakuEditor",
											tr("seems not member-function decl."));
			return QString();
		}
	}
	implText = getText(buf, start, tn.tokenPosition() - start);
	implText.replace(QRegExp("\\t+"), " ");
#else
	for(;;) {
		QChar ch = tn.skipSpace();
		if( ch == '(' ) break;
		implText += tn.tokenText();
		if( tn.tokenType() == Tokenizer::IDENT && isLetterOrNumberOrUnderbar(ch) )
			implText += " ";
		if( tn.nextToken() == Tokenizer::END_OF_FILE ) {
			QMessageBox::warning(this, "SakuSakuEditor",
											tr("seems not member-function decl."));
			return QString();
		}
	}
#endif
	implText += className;
	implText += "::";
	implText += tn.tokenText();
	//	とりあえず ) まで追加
	pos = tn.nextPosition();
	bool bSharp;
	int dst = assocParenPosition(typeSettings(), buf, pos, bSharp);
	if( dst < 0 || (ln = positionToLine(dst)) - ln0 > 32 )
	{
		QMessageBox::warning(this, "SakuSakuEditor",
										tr("seems not member-function decl."));
		return QString();
	}
	while( pos < dst )
		implText += QChar(buf[pos++]);
	QChar ch;
	while( pos < buf.size() ) {
		if( (ch = buf[pos]) == ';' ) break;
		if( ch == '\r' || ch == '\n' ) {
			pos = buf.size();
			break;
		}
		implText += ch;
		++pos;
	}
	if( pos >= buf.size() ) {
		QMessageBox::warning(this, "SakuSakuEditor",
										tr("seems not member-function decl."));
		return QString();
	}
	++pos;		//	skip ';'
	while( pos < buf.size() ) {
		if( (ch = buf[pos++]) == '\r' || ch == '\n' ) break;
		implText += ch;
	}
	while( pos < buf.size() && (buf[pos] == '\r' || buf[pos] == '\n') ) ++pos;
	
	implText += newLineText() + "{" + newLineText() + "}" + newLineText();
	return implText;
}
void EditView::tagsJump()
{
	TextCursor cur(*m_textCursor);
	cur.movePosition(TextCursor::BEG_WORD);
	cur.movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
	if( !cur.hasSelection() ) return;
	QString text = cur.selectedText();
	if( !isLetterOrUnderbar(text[0]) ) return;		//	英字で始まる単語でない場合
	QString dirStr = QDir::currentPath();;
	QString path = fullPathName();
	if( !path.isEmpty() ) {
		QDir dir(path);
		dir.cdUp();
		dirStr = dir.absolutePath();
	}
	mainWindow()->tagsJump(text, dirStr);
}
void EditView::tagJump()
{
	int dln = viewLineToDocLine(m_textCursor->viewLine());
#if 0	//##
	if( m_grepView || m_isOutputView ) {
		QString text = getLineText(dln);
		//qDebug() << text;
		if( text.isEmpty() ) return;
		//if( text[0].isNumber() ) {
		//	int ix = 1;
		//	while( text[ix].isNumber() ) ++ix;
		//	int lineNum = text.left(ix).toInt();
		//	emit jump(lineNum-1);
		//	return;
		//}
		QString t = text;
		if( text.startsWith("tag: ") ) {
			text = text.mid(5);	//	skip "tag: "
			int ix = text.indexOf('\t');
			if( ix < 0 ) return;
			QString sym = text.left(ix);
			text = text.mid(ix + 1);
			if( sym[0] == '\"' ) {		//	"ファイル名" がある場合
				if( (ix = text.indexOf('\t')) < 0 ) return;
				sym = text.left(ix);
				text = text.mid(ix + 1);
			}
			if( (ix = text.indexOf('\t')) < 0 ) return;
			QString fileName = text.left(ix);
			text = text.mid(ix + 1);
			emit tagsJump(sym, fileName, text);
			return;
		} else if( text[0] == '\"' ) {
			text.clear();
		} else {
			do {
				if( --dln < 0 ) return;
				t = getLineText(dln);
			} while( t[0] != '\"' );
		}
		int ixDQ = t.indexOf('\"', 1);
		if( ixDQ < 0 )return;
		QString filePath = t.mid(1, ixDQ - 1);
		int lineNum = -1;
		if( t[ixDQ + 1] == '(' ) {		//	"fileName"(lineNum): の場合
			int ix = t.indexOf(')', ixDQ);
			if( ix > 0 )
				lineNum = t.mid(ixDQ+2, ix - ixDQ - 2).toInt();
		} else {
			int ix = text.indexOf(':');
			if( ix > 0 )
				lineNum = text.left(ix).toInt();
		}
		emit tagJump(filePath, lineNum);
		return;
	}
#endif
	//pos_t pos = m_textCursor->position();
	//int ln = m_doc->positionToLine(pos);
	Tokenizer tkn(*buffer(), lineStartPosition(dln), lineStartPosition(dln+1));
	if( typeSettings()->name() == "HTML" ) {
		while( tkn.tokenType() != Tokenizer::END_OF_FILE ) {
			if( tkn.tokenText() == "href"
				&& tkn.nextToken() == Tokenizer::SYMBOL && tkn.tokenText() == "="
				&& tkn.nextToken() == Tokenizer::STRING )
			{
				QString fileName = tkn.tokenText().mid(1, tkn.tokenText().size() - 2);
				//qDebug() << fileName;
				int ix = fileName.indexOf("#");
				if( ix > 0 )
					fileName = fileName.left(ix);
				if( (ix = fileName.indexOf("?")) > 0 )
					fileName = fileName.left(ix);
				QString htdocsRoot = globSettings()->textValue(GlobalSettings::HTDOCS_ROOT);
				if( !fileName.isEmpty() && fileName[0] == '/' && !htdocsRoot.isEmpty() ) {
					if( htdocsRoot[htdocsRoot.size() - 1] != '/' )
						htdocsRoot += '/';
					fileName = htdocsRoot + fileName;
				}
				emit openFile(fileName);
				return;
			}
			tkn.nextToken();
		}
	} else if( typeSettings()->name() == "CPP" ) {
		if( tkn.tokenText() == "#"
			&& tkn.nextToken() == Tokenizer::IDENT && tkn.tokenText() == "include"
			&& tkn.nextToken() == Tokenizer::STRING )
		{
			QString fileName = tkn.tokenText().mid(1, tkn.tokenText().size() - 2);
			emit openFile(fileName);
		}
	}
}
#if	0
void EditView::tagJump(const QString &, int)
{
}
#endif
void EditView::imeOpenStatusChanged()
{
	closeAutoCompletionDlg();
}
void EditView::moveLineCmtToPrev()	//	現在行に行コメントがあれば、全行に移動
{
	QString lineComment = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
	if( lineComment.isEmpty() )
		return;
	int vln = m_textCursor->viewLine();
	int ln = viewLineToDocLine(vln);
#if	1
	int lineStart = lineStartPosition(ln);
	int lineStart2 = lineStartPosition(ln+1);
	ssize_t sz = lineStart2 - lineStart;
	ViewTokenizer dt(typeSettings(), buffer(), lineStart, sz, lineStart2);
	QString token;
	pos_t pos = 0;
	for(;;) {
		token = dt.nextToken();
		if( token.isEmpty() ) break;
		if( token == lineComment ) {
			pos = dt.tokenix();
			break;
		}
		//qDebug() << token;
	}
	while( pos > lineStart && isSpaceChar(charAt(pos-1)) )
		--pos;
	if( !pos || pos == lineStart )
		return;		//	行コメントが行頭 or 無かった場合
	int p = lineStart;
	while( isSpaceChar(charAt(p)) ) ++p;
	QString indent = getText(*buffer(), lineStart, p-lineStart);
	//QString text = getText(buffer(), pos, );
	openUndoBlock();
	m_textCursor->setPosition(pos);
	m_textCursor->movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
	QString text = m_textCursor->selectedText().trimmed() + newLineText();
	m_noFallingChars = true;
	m_textCursor->deleteChar();
	m_noFallingChars = false;
	m_textCursor->setPosition(lineStart);
	m_textCursor->insertText(indent + text);
	closeUndoBlock();
	updateScrollBarInfo();
	update();
#else
	Tokenizer tkn(*buffer(), lineStartPosition(ln), lineStartPosition(ln+1), true, typeSettings());
	while( tkn.tokenType() != Tokenizer::END_OF_FILE ) {
		qDebug() << tkn.tokenText();
		tkn.nextToken();
	}
#endif
}
void EditView::sharpIfCommentOut()
{
	sharpIfCommentOut(false);
}
void EditView::sharpIfElseCommentOut()
{
	sharpIfCommentOut(true);
}
void EditView::sharpIfCommentOut(bool bElse)
{
	int vFirstLine;		//	選択開始行
	int vLastLine;		//	選択範囲＋１の行
	if( m_textCursor->hasSelection() ) {
		//	undone: box 選択状態
		vFirstLine = m_textCursor->selectionFirstLine();
		vLastLine = m_textCursor->selectionLastLine();
		pos_t pos = m_textCursor->selectionLast();
		if( pos != viewLineStartPosition(vLastLine) )		//	行頭位置でない場合
			++vLastLine;
	} else {
		vFirstLine = m_textCursor->viewLine();
		vLastLine = vFirstLine + 1;
	}
	openUndoBlock();
	m_textCursor->setPosition(viewLineStartPosition(vLastLine));
	m_textCursor->insertText("#endif" + newLineText());
	m_textCursor->setPosition(viewLineStartPosition(vFirstLine));
	if( bElse ) {
		m_textCursor->insertText("#if\t1" + newLineText());
		m_textCursor->insertText("#else" + newLineText());
	} else {
		m_textCursor->insertText("#if\t0" + newLineText());
	}
	closeUndoBlock();
}
ViEngine *EditView::viEngine()
{
	return mainWindow()->viEngine();
}
void EditView::encomment()
{
	QString cmtText = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
	if( cmtText.isEmpty() ) return;
	//if( m_typeSettings->name() != "CPP"
	//	&& m_typeSettings->name() != "C#"
	//	&& m_typeSettings->name() != "F#"
	//	&& m_typeSettings->name() != "JS"
	//	&& m_typeSettings->name() != "PHP"
	//	&& m_typeSettings->name() != "JAVA" )
	//{
	//	return;
	//}
	pos_t first = m_textCursor->selectionFirst();
	int ln = document()->positionToLine(first);
	pos_t last = m_textCursor->selectionLast();
	int lnLast = document()->positionToLine(last);
	if( last > lineStartPosition(lnLast) )		//	行頭まで選択されていた場合
		++lnLast;
	const int tw = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int nmax = INT_MAX;
	for(int l = ln; l < lnLast; ++l) {
		pos_t pos = lineStartPosition(l);
		while( isSpaceChar(charAt(pos)) ) ++pos;
		int i = pos - lineStartPosition(l);
		if( (nmax = qMin(nmax, i)) == 0 )
			break;
	}
	openUndoBlock();
	for(; ln < lnLast; ++ln) {
		pos_t pos = lineStartPosition(ln);
		m_textCursor->setPosition(pos);
		if( nmax != 0 )
			m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::MOVE_ANCHOR, nmax);
		m_textCursor->insertText(cmtText);
	}
	closeUndoBlock();
}
void EditView::decomment()
{
	QString cmtText = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
	if( cmtText.isEmpty() ) return;
	//if( m_typeSettings->name() != "CPP"
	//	&& m_typeSettings->name() != "C#"
	//	&& m_typeSettings->name() != "F#"
	//	&& m_typeSettings->name() != "JS"
	//	&& m_typeSettings->name() != "PHP"
	//	&& m_typeSettings->name() != "JAVA" )
	//{
	//	return;
	//}
	pos_t first = m_textCursor->selectionFirst();
	int ln = document()->positionToLine(first);
	pos_t last = m_textCursor->selectionLast();
	int lnLast = document()->positionToLine(last);
	if( last > lineStartPosition(lnLast) )		//	行頭まで選択されていた場合
		++lnLast;
	openUndoBlock();
	for(; ln < lnLast; ++ln) {
		pos_t pos = lineStartPosition(ln);
		while( isSpaceChar(charAt(pos)) ) ++pos;
		//if( charAt(pos) == '/' && charAt(pos+1) == '/' )
		if( startsWith(*buffer(), pos, cmtText) )
		{
			m_textCursor->setPosition(pos);
			m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, cmtText.size());
			setupFallingChars();
			m_textCursor->deleteChar();
		}
	}
	closeUndoBlock();
}
void EditView::blockComment()
{
	if( !m_textCursor->hasSelection() ) return;
	TypeSettings *typeStg = typeSettings();
	int ln = positionToLine(m_textCursor->selectionFirst());
	uint flags = buffer()->lineFlags(ln);
#if 0		//##
	if( typeStg->name() == "HTML" ) {
		if( (flags & Buffer::LINEFLAG_IN_SCRIPT) != 0 && m_jsTypeSettings )
			typeStg = m_jsTypeSettings;
		else if( (flags & Buffer::LINEFLAG_IN_PHP) != 0 && m_phpTypeSettings )
			typeStg = m_phpTypeSettings;
	}
#endif
	insertText(typeStg->textValue(TypeSettings::BLOCK_COMMENT_BEG),
					typeStg->textValue(TypeSettings::BLOCK_COMMENT_END));
}
#if	0
void EditView::doKeisenLeft(bool erase)
{
}
void EditView::doKeisenRight(bool erase)
{
}
void EditView::doKeisenUp(bool erase)
{
}
void EditView::doKeisenDown(bool erase)
{
}
#endif
void EditView::renumber()
{
	//##if( isBoxSelectionMode() ) return;		//	undone: 矩形選択未対応
	if( !hasSelection() ) return;
	//bool fillZero = false;
	int fillWidth = 0;
	if( hasSelectionInALine() ) {
		pos_t first = m_textCursor->selectionFirst();
		pos_t last = m_textCursor->selectionLast();
#if		1
		bool init = true;
		int value = 0;
		openUndoBlock();
		while( first < last ) {
			if( isDigit(charAt(first)) ) {
				QString text;
				pos_t pos = first;
				wchar_t ch;
				while( first < last && isDigit(ch = charAt(first)) ) {
					text += QChar(ch);
					++first;
				}
				if( init ) {
					init = false;
					value = text.toInt();
				} else {
					QString ntext;
					if( !fillWidth )
						ntext = QString::number(++value);
					else
						ntext = QString("%1").arg(++value, fillWidth, 10, QChar('0'));
					m_textCursor->setPosition(pos);
					m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, text.size());
					m_textCursor->insertText(ntext);
					first += ntext.size() - text.size();
					last += ntext.size() - text.size();
				}
			} else
				++first;
		}
#else
		Tokenizer tkn(buffer(), first, last, /*bStr*/false);
		bool init = true;
		int value = 0;
		openUndoBlock();
		while( tkn.tokenType() != Tokenizer::END_OF_FILE ) {
			if( tkn.tokenType() == Tokenizer::NUMBER ) {
				QString text = tkn.tokenText();
				int len = 0;
				while( len < text.size() && text[len].isNumber() ) ++len;
				if( init ) {
					init = false;
					value = text.left(len).toInt();
				} else {
					QString ntext;
					if( !fillWidth )
						ntext = QString::number(++value);
					else
						ntext = QString("%1").arg(++value, fillWidth, 10, QChar('0'));
					m_textCursor->setPosition(tkn.tokenPosition());
					m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, len);
					m_textCursor->insertText(ntext);
					tkn.setPosition(m_textCursor->position());
					tkn.setLast(last += ntext.size() - len);
				}
			}
			tkn.nextToken();
		}
#endif
		closeUndoBlock();
		update();
		return;
	} 
	//	行単位のりナンバー
	int ln1, ln2;
	getSelectedLineRange(ln1, ln2);
	//pos_t ls2 = lineStartPosition(ln1);
	bool init = true;
	int value = 0;
	openUndoBlock();
	for(int ln = ln1; ln <= ln2; ++ln) {
#if		1
		pos_t first = lineStartPosition(ln);
		pos_t last = lineStartPosition(ln + 1);
		while( first < last ) {
			if( isDigit(charAt(first)) ) {
				QString text;
				pos_t pos = first;
				wchar_t ch;
				while( first < last && isDigit(ch = charAt(first)) ) {
					text += QChar(ch);
					++first;
				}
				if( init ) {
					init = false;
					value = text.toInt();
				} else {
					QString ntext;
					if( !fillWidth )
						ntext = QString::number(++value);
					else
						ntext = QString("%1").arg(++value, fillWidth, 10, QChar('0'));
					m_textCursor->setPosition(pos);
					m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, text.size());
					m_textCursor->insertText(ntext);
					//last += ntext.size() - text.size();
				}
				break;
			} else
				++first;
		}
#else
		pos_t ls = lineStartPosition(ln);
		pos_t ls2 = lineStartPosition(ln + 1);
		Tokenizer tkn(buffer(), ls, ls2, /*bStr*/false);
		while( tkn.tokenType() != Tokenizer::END_OF_FILE ) {
			if( tkn.tokenType() == Tokenizer::NUMBER ) {
				QString text = tkn.tokenText();
				int len = 0;
				while( len < text.size() && text[len].isNumber() )
					++len;
				if( init ) {
					init = false;
					if( text[0] == '0' ) fillWidth = len;		//	桁数
					value = text.left(len).toInt();
				} else {
					QString ntext;
					if( !fillWidth )
						ntext = QString::number(++value);
					else
						ntext = QString("%1").arg(++value, fillWidth, 10, QChar('0'));
					m_textCursor->setPosition(tkn.tokenPosition());
					m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, len);
					m_textCursor->insertText(ntext);
				}
				break;
			}
			tkn.nextToken();
		}
#endif
	}
	closeUndoBlock();
}
