#ifndef GREPPINGDLG_H
#define GREPPINGDLG_H

#include <QDialog>
#include "ui_GreppingDlg.h"

class GreppingDlg : public QDialog
{
	Q_OBJECT

public:
	GreppingDlg(QWidget *parent = 0);
	~GreppingDlg();

public slots:
	void	setGreppingDir(const QString &);
	
//public slots:
signals:
	void	terminate();

private:
	Ui::GreppingDlg ui;
};

#endif // GREPPINGDLG_H
