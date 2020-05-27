#pragma once

#include <QTreeWidget>

class OutlineBar : public QTreeWidget
{
	Q_OBJECT

public:
	OutlineBar(QWidget *parent = nullptr);
	
protected:
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	keyPressEvent(QKeyEvent *);

signals:
	void	doubleClicked(int);
	void	doubleClicked(QTreeWidgetItem*);
	void	enterPressed();
	void	colonPressed();
	void	keyHPressed();
	void	keyJPressed();
	void	keyKPressed();
	void	keyLPressed();
};
