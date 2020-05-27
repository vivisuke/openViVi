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
	case Qt::Key_Q:
		emit enterPressed();
		return;
	case Qt::Key_Colon:
		emit colonPressed();
		return;
	case Qt::Key_H:
		keyHPressed();
		return;
	case Qt::Key_J:
		keyJPressed();
		return;
	case Qt::Key_K:
		keyKPressed();
		return;
	case Qt::Key_L:
		keyLPressed();
		return;
	}
	QTreeWidget::keyPressEvent(event);
}
void OutlineBar::keyHPressed()
{
	auto* item = currentItem();
	if( item == nullptr ) return;
	auto* pr = item->parent();
	if( pr != nullptr )
		setCurrentItem(pr);
}
void OutlineBar::keyJPressed()
{
	auto* item = currentItem();
	if( item == nullptr ) return;
	int cnt = topLevelItemCount();
	int ix = indexOfTopLevelItem(item);
	if( ix >= 0 && ix < cnt - 1) {
		item = topLevelItem(ix+1);
		setCurrentItem(item);
	} else if( ix < 0 ) {		//	トップレベルアイテムでない場合
		auto* pr = item->parent();
		ix = pr->indexOfChild(item);
		cnt = pr->childCount();
		if( ix >= 0 && ix < cnt - 1) {
			setCurrentItem(pr->child(ix+1));
		}
	}
}
void OutlineBar::keyKPressed()
{
	auto* item = currentItem();
	if( item == nullptr ) return;
	int ix = indexOfTopLevelItem(item);
	if( ix > 0 ) {
		item = topLevelItem(ix-1);
		setCurrentItem(item);
	} else if( ix < 0 ) {		//	トップレベルアイテムでない場合
		auto* pr = item->parent();
		ix = pr->indexOfChild(item);
		if( ix > 0 ) {
			setCurrentItem(pr->child(ix-1));
		} else
			keyHPressed();
	}
}
void OutlineBar::keyLPressed()
{
}
