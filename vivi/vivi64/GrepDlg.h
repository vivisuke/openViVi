#ifndef GREPDLG_H
#define GREPDLG_H

#include <QDialog>
#include <QFileSystemModel>
#include "ui_GrepDlg.h"

class GlobalSettings;

class GrepDlg : public QDialog
{
	Q_OBJECT

public:
	GrepDlg(GlobalSettings *globSettings, const QStringList &, int, QWidget *parent = 0);
	~GrepDlg();

public:
	QString	findString() const;
	QString	extentions() const;
	QString	dir() const;
	QString	exclude() const;
	bool	ignoreCase() const;
	bool	wholeWordOnly() const;
	bool	regExp() const;
	bool	grepSubDir() const;
	bool	grepView() const;

public:
	void	setFindString(const QString &);
	void	setTypeName(const QString &);

protected:
	void	keyPressEvent(QKeyEvent *);

protected slots:
	void	dirViewDoubleClicked ( QModelIndex index );
	void	dirViewClicked ( QModelIndex index );
	void	regexpHelp();
	
private:
	Ui::GrepDlg ui;
	GlobalSettings	*m_globSettings;
	QFileSystemModel m_fileSystemModel;
};

#endif // GREPDLG_H
