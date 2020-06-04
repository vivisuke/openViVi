#ifndef OUTPUTVIEW_H
#define OUTPUTVIEW_H

#include <QPlainTextEdit>

class OutputView : public QPlainTextEdit
{
	Q_OBJECT

public:
	OutputView(QWidget *parent = 0);
	~OutputView();

public:
	void	clear();
	void	tagJump();

signals:
	void	jump(int);
	void	tagJump(const QString &, int);
	void	tagsJump(const QString &, const QString &, const QString &);
	void	mouseReleased();
	void	escPressed();

private:
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	keyPressEvent ( QKeyEvent * event );
};

#endif // OUTPUTVIEW_H
