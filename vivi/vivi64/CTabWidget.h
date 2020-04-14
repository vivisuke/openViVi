#pragma once

#include <QTabWidget>

class CTabWidget : public QTabWidget
{
public:
	CTabWidget(QWidget *parent = nullptr);
	~CTabWidget();
public:
	//int		addTab(QWidget *page, const QIcon &icon, const QString &label);
protected:
	void	paintEvent(QPaintEvent *event);
};
