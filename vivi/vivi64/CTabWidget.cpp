#include <QtGui>
#include <QPainter>
#include "CTabWidget.h"
#include "CTabBar.h"

CTabWidget::CTabWidget(QWidget *parent)
	: QTabWidget(parent)
{
	setTabBar(new CTabBar());
}
CTabWidget::~CTabWidget()
{
}
#if	0
int CTabWidget::addTab(QWidget *page, const QIcon &icon, const QString &label)
{
	int rc = QTabWidget::addTab(page, icon, label);
	auto tb = tabBar();
	auto g = tb->geometry();
	g.setWidth(g.width() + 100);
	tb->setGeometry(g);
	//auto sz = tb->size();
	//tb->resize(sz.width()+10, sz.height());
	return rc;
}
#endif

void CTabWidget::paintEvent(QPaintEvent *event)
{
	auto rct = rect();
	QPainter pt(this);
	//pt.setBrush(QColor("skyblue"));
	pt.setBrush(QColor("#c0c0c0"));
	pt.setPen(Qt::transparent);
	pt.drawRect(rct);
#if	0
	pt.setPen(Qt::black);
	pt.drawLine(0, 0, rct.width(), rct.height());
	pt.drawLine(0, rct.height(), rct.width(), 0);
#endif
}
