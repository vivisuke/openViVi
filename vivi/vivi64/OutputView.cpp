#include <QtGui>
#include "OutputView.h"

OutputView::OutputView(QWidget *parent)
	: QPlainTextEdit(parent)
{
}
OutputView::~OutputView()
{
}
void OutputView::mouseDoubleClickEvent(QMouseEvent *event)
{
	tagJump();
}
void OutputView::tagJump()
{
	QTextCursor cur = textCursor();
	QTextBlock block = cur.block();
	QString text = block.text();
	//qDebug() << text;
	if( text.isEmpty() ) return;
	if( text[0].isNumber() ) {
		int ix = 1;
		while( text[ix].isNumber() ) ++ix;
		int lineNum = text.left(ix).toInt();
		emit jump(lineNum-1);
		return;
	}
	QString t = text;
	if( text.startsWith("tag: ") ) {
		text = text.mid(5);	//	skip "tag: "
		int ix = text.indexOf('\t');
		if( ix < 0 ) return;
		QString sym = text.left(ix);
		text = text.mid(ix + 1);
		if( sym[0] == '\"' ) {		//	"ƒtƒ@ƒCƒ‹–¼" ‚ª‚ ‚éê‡
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
		//	"path"(lineNum): text
		int ix = t.indexOf("\"(", 1);
		if( ix < 0 ) return;
		int ix2 = t.indexOf("):", ix);
		if( ix2 < 0 ) return;
		int lineNum = t.mid(ix+2, ix2 - ix - 2).toInt();
		QString filePath = t.mid(1, ix - 1);
		emit tagJump(filePath, lineNum);
		return;
	} else {
		do {
			block = block.previous();
			if( !block.isValid() ) return;
			t = block.text();
		} while( t[0] != '\"' );
	}
	QString filePath = t.mid(1);
	filePath = filePath.left(filePath.indexOf('\"'));
	int lineNum = -1;
	int ix = text.indexOf(':');
	if( ix > 0 ) lineNum = text.left(ix).toInt();
	emit tagJump(filePath, lineNum);
}
void OutputView::clear()
{
	QTextCursor cur = textCursor();
	cur.movePosition(QTextCursor::Start);
	cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cur.deleteChar();
}
void OutputView::keyPressEvent( QKeyEvent * event )
{
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	switch( event->key() ) {
		case Qt::Key_Escape:
			emit escPressed();
			return;
		case Qt::Key_Enter:
		case Qt::Key_Return:
			tagJump();
			return;
	}
	QPlainTextEdit::keyPressEvent(event);
}

