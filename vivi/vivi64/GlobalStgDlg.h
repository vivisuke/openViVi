#ifndef GLOBALSTGDLG_H
#define GLOBALSTGDLG_H

#include <QDialog>
#include "ui_GlobalStgDlg.h"

class GlobalSettings;

class GlobalStgDlg : public QDialog
{
	Q_OBJECT

public:
	GlobalStgDlg(GlobalSettings *, QWidget *parent = 0);
	~GlobalStgDlg();

public slots:
	void	accept();

public slots:
	void	refPict1FilePath();
	void	refPict2FilePath();
	void	ZenCodingFilePath();
	void	htdocsRootPath();

protected:
	void	setFontFamily(const QString &name);
	
private:
	Ui::GlobalStgDlg ui;
	GlobalSettings	*m_globSettings;
};

#endif // GLOBALSTGDLG_H
