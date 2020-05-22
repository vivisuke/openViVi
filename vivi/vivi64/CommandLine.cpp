#include <QKeyEvent>
#include "CommandLine.h"

CommandLine::CommandLine(QWidget *parent)
	: QLineEdit(parent)
{
}

CommandLine::~CommandLine()
{
}
void CommandLine::focusOutEvent( QFocusEvent * event )
{
	QLineEdit::focusOutEvent(event);
	emit focusOut();
}
void CommandLine::keyPressEvent(QKeyEvent *event)
{
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	if( event->key() == Qt::Key_Backspace
		&& !hasSelectedText() && cursorPosition() == 1 )
	{
		emit escPressed();
		return;
	}
	if( event->key() == Qt::Key_Home ) {
		setCursorPosition(1);
		return;
	}
	if( event->key() == Qt::Key_Up ) {
		emit upPressed();
		return;
	}
	if( event->key() == Qt::Key_Down ) {
		emit downPressed();
		return;
	}
	if( event->key() == Qt::Key_Left &&  cursorPosition() == 1 ) {
		return;
	}
	if( event->key() == Qt::Key_A && ctrl ) {
		setSelection(1, text().size());
		return;
	}
	QLineEdit::keyPressEvent(event);		//	基底クラスの処理
	if( event->key() == Qt::Key_Escape ) {
		emit escPressed();
	}
	if( event->key() == Qt::Key_Space ) {
		emit spacePressed();
	}
	if( event->key() == Qt::Key_Slash || event->key() == Qt::Key_Backslash ) {
		emit slashPressed();		//	ディレクトリ区切り文字が入力された
	}
	if( event->key() == Qt::Key_Colon) {
		emit colonPressed();
	}
	if( event->key() == Qt::Key_Tab) {
		emit tabPressed();
	}
}
bool CommandLine::focusNextPrevChild(bool next)
{
	return false;
}
