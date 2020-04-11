#include <QKeyEvent>
#include "FindLineEdit.h"

FindLineEdit::FindLineEdit(QWidget *parent)
	: QLineEdit(parent)
{
}
FindLineEdit::~FindLineEdit()
{
}
void FindLineEdit::focusInEvent( QFocusEvent * event )
{
	QLineEdit::focusInEvent(event);
	emit focusIn();
}
void FindLineEdit::keyPressEvent(QKeyEvent *event)
{
	QLineEdit::keyPressEvent(event);
	if( event->key() == Qt::Key_Escape ) {
		emit escPressed();
	}
}
