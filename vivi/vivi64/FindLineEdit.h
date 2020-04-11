#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class FindLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	FindLineEdit(QWidget *parent = 0);
	~FindLineEdit();

protected:
	void	keyPressEvent(QKeyEvent *);
	void	focusInEvent ( QFocusEvent * event );

signals:
	void focusIn();
	void	escPressed();

private:
};

#endif // FINDLINEEDIT_H
