#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QLineEdit>

class CommandLine : public QLineEdit
{
	Q_OBJECT

public:
	CommandLine(QWidget *parent = 0);
	~CommandLine();

protected:
	void	keyPressEvent(QKeyEvent *);
	void	focusOutEvent ( QFocusEvent * event );
	bool	focusNextPrevChild(bool next);

signals:
	void	focusOut();
	void	escPressed();
	void	spacePressed();
	void	slashPressed();			//	ディレクトリセパレータ押下
	void	colonPressed();
	void	upPressed();
	void	downPressed();

private:
	
};

#endif // COMMANDLINE_H
