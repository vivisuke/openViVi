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
#if	0
	QPixmap pxmp(":/MainWindow/Resources/Trapezoid.png");
	pt.drawPixmap(rct, pxmp, pxmp.rect());
#else
	pt.drawPolygon(pnts, NPNTS);
	//
	pt.setPen(Qt::white);
	pt.drawLine(pnts[3], pnts[0]);
	pt.setPen(Qt::black);
	pt.drawLine(pnts[2], pnts[1]);
#endif
	//auto txt = tabText(i);
	pt.setPen(!bCurrent ? Qt::white : Qt::black);
	//int x = rct.x();
	//int y = rct.y();
	//int wd = rct.width();
	//int ht = rct.height();
	//QRect r2(0, 0, 100, 20);
	//QRect rct2(x, y, wd, ht);
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
#if	0
	if( count() != 0 ) {
		auto rct = tabRect(count() - 1);
		if( brct.right() == rct.right() ) {
			auto g = geometry();
			g.setWidth(g.width() + 100);
			setGeometry(g);
			//update();
			//return;
		}
	}
#endif
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
#if	0
		pt.setBrush(QColor("darkgray"));
		pt.setPen(QColor("darkgray"));
		//pt.drawRect(rct);
		makePoints(pnts, rct, DX, i == count() - 1);
		pt.drawPolygon(pnts, NPNTS);
		pt.setPen(Qt::white);
		pt.drawLine(pnts[3], pnts[0]);
		pt.setPen(Qt::black);
		pt.drawLine(pnts[2], pnts[1]);
		auto txt = tabText(i);
		pt.setPen(Qt::white);
		//pt.drawText(rct.x(), rct.y()+rct.height(), txt);
		rct.setX(rct.x() + idx*3);
		pt.drawText(rct, Qt::AlignHCenter|Qt::AlignVCenter, txt);
		auto icon = tabIcon(i);
		auto pxmap = icon.pixmap(iconSize());
		rct.setX(rct.x() - idx*2);
		rct.setY(rct.y() + idy);
		rct.setSize(iconSize());
		pt.setOpacity(0.333);
		pt.drawPixmap(rct, pxmap);
		pt.setOpacity(1.0);
#endif
	}
	auto ix = currentIndex();
	if( ix >= 0 ) {
		auto rct = tabRect(ix);
		drawTrapezoid(pt, rct, iconSize(), tabIcon(ix), tabText(ix), idx, idy, ix == count() - 1, true);
#if	0
		pt.setBrush(Qt::white);
		pt.setPen(Qt::white);
		//pt.drawRect(rct);
		makePoints(pnts, rct, DX, ix == count() - 1);
		pt.drawPolygon(pnts, NPNTS);
		pt.setPen(Qt::white);
		pt.drawLine(pnts[3], pnts[0]);
		pt.setPen(Qt::black);
		pt.drawLine(pnts[2], pnts[1]);
		auto txt = tabText(ix);
		pt.setPen(Qt::black);
		//pt.drawText(rct.x(), rct.y()+rct.height(), txt);
		rct.setX(rct.x() + idx*3);
		pt.drawText(rct, Qt::AlignHCenter|Qt::AlignVCenter, txt);
		auto icon = tabIcon(ix);
		auto pxmap = icon.pixmap(iconSize());
		rct.setX(rct.x() - idx*2);
		rct.setY(rct.y() + idy);
		rct.setSize(iconSize());
		pt.drawPixmap(rct, pxmap);
#endif
	}
}

