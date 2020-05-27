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
	if( ix >= 0 ) {
		if( item->childCount() != 0 ) {
			setCurrentItem(item->child(0));
		} else if( ix < cnt - 1) {
			item = topLevelItem(ix+1);
			setCurrentItem(item);
		}
	} else if( ix < 0 ) {		//	トップレベルアイテムでない場合
		auto* pr = item->parent();
		int ix2 = pr->indexOfChild(item);
		int cnt2 = pr->childCount();
		if( ix2 >= 0 && ix2 < cnt2 - 1) {
			setCurrentItem(pr->child(ix2+1));
		} else {
			ix = indexOfTopLevelItem(pr);
			if( ix < cnt - 1 )
				setCurrentItem(topLevelItem(ix+1));
		}
	}
}
void OutlineBar::keyKPressed()
{
	auto* item = currentItem();
	if( item == nullptr ) return;
	int ix = indexOfTopLevelItem(item);
	if( ix > 0 ) {	//	トップレベルアイテムの場合
		item = topLevelItem(ix-1);
		int cnt = item->childCount();
		if( cnt == 0 ) {
			setCurrentItem(item);
		} else {
			setCurrentItem(item->child(cnt-1));
		}
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
