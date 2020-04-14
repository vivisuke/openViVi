#pragma once

#include <QTabBar>

class CTabBar : public QTabBar
{
public:
	CTabBar(QWidget *parent = nullptr);
	~CTabBar();
protected:
	void	paintEvent(QPaintEvent *event);
};
