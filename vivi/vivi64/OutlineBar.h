#pragma once

#include <QTreeWidget>

class OutlineBar : public QTreeWidget
{
	Q_OBJECT

public:
	OutlineBar(QWidget *parent = nullptr);
	
protected:
	void	mouseDoubleClickEvent(QMouseEvent *);

signals:
	void	doubleClicked(int);
	void	doubleClicked(QTreeWidgetItem*);
};
