#include <QtGui>
#include <QPainter>
#include <QRect>
#include "CTabBar.h"


CTabBar::CTabBar(QWidget *parent)
	: QTabBar(parent)
{
}
CTabBar::~CTabBar()
{
}
void makePoints(QPoint pnts[], const QRect& rct, int DX, bool bLast)
{
	pnts[0] = rct.topLeft();
	pnts[0].setX(pnts[0].x() + DX);
	pnts[1] = rct.topRight();
	pnts[2] = rct.bottomRight();
	pnts[3] = rct.bottomLeft();
	pnts[3].setX(pnts[3].x() - DX);
	if( !bLast ) {
		pnts[1].setX(pnts[1].x() - DX);
		pnts[2].setX(pnts[2].x() + DX);
	} else {
		pnts[1].setX(pnts[1].x() - DX*2);
		pnts[2].setX(pnts[2].x());
	}
}
void drawTrapezoid(QPainter& pt, const QRect& rct, QSize &isz, QIcon &icon, QString &txt,
					int idx, int idy, bool bLast, bool bCurrent)
{
	const int NPNTS = 4;
	const int DX = 3;
	QPoint pnts[NPNTS];
	QColor col = !bCurrent ? QColor("darkgray") : Qt::white;
	makePoints(pnts, rct, DX, bLast);
	pt.setBrush(col);
	pt.setPen(col);
	pt.drawPolygon(pnts, NPNTS);
	//
	pt.setPen(Qt::white);
	pt.drawLine(pnts[3], pnts[0]);
	pt.setPen(Qt::black);
	pt.drawLine(pnts[2], pnts[1]);
	pt.setPen(!bCurrent ? Qt::white : Qt::black);
	QRect r2(rct.x(), rct.y(), rct.width(), rct.height());
	r2.setX(rct.x() + idx*3);
	pt.drawText(r2, Qt::AlignHCenter|Qt::AlignVCenter, txt);
	auto pxmap = icon.pixmap(isz);
	r2.setX(r2.x() - idx*2);
	r2.setY(r2.y() + idy);
	r2.setSize(isz);
	if( !bCurrent )
		pt.setOpacity(0.333);
	pt.drawPixmap(r2, pxmap);
	pt.setOpacity(1.0);
}
void CTabBar::paintEvent(QPaintEvent *event)
{
	auto brct = rect();
	const auto idy = (brct.height() - iconSize().height()) / 2;
	const auto idx = (brct.height() - iconSize().height()) * 2 / 3;
	QPainter pt(this);
	pt.setRenderHint(QPainter::Antialiasing);
	pt.setPen(Qt::black);
	const int NPNTS = 4;
	const int DX = 3;
	QPoint pnts[NPNTS];
	for (int i = 0; i < count(); ++i) {
		auto rct = tabRect(i);
		drawTrapezoid(pt, rct, iconSize(), tabIcon(i), tabText(i), idx, idy, i == count() - 1, false);
	}
	auto ix = currentIndex();
	if( ix >= 0 ) {
		auto rct = tabRect(ix);
		drawTrapezoid(pt, rct, iconSize(), tabIcon(ix), tabText(ix), idx, idy, ix == count() - 1, true);
	}
}

