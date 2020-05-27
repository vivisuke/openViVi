#include <QKeyEvent>
#include "OutlineBar.h"

OutlineBar::OutlineBar(QWidget *parent)
	: QTreeWidget(parent)
{
}
void OutlineBar::mouseDoubleClickEvent(QMouseEvent *)
{
	auto* item = currentItem();
	//auto* view = (EditView*)item->data(1, 0).toULongLong();
	emit doubleClicked(item);
}
void OutlineBar::keyPressEvent(QKeyEvent *event)
{
	switch( event->key() ) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		emit enterPressed();
		return;
	case Qt::Key_Colon:
		emit colonPressed();
		return;
	case Qt::Key_H:
		emit keyHPressed();
		return;
	case Qt::Key_J:
		emit keyJPressed();
		return;
	case Qt::Key_K:
		emit keyKPressed();
		return;
	case Qt::Key_L:
		emit keyLPressed();
		return;
	}
	QTreeWidget::keyPressEvent(event);
}
