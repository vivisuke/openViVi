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
