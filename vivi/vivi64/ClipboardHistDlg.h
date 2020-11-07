#ifndef CLIPBOARDHISTDLG_H
#define CLIPBOARDHISTDLG_H

#include <QDialog>
#include "ui_ClipboardHistDlg.h"

class ClipboardHistDlg : public QDialog
{
	Q_OBJECT

public:
	ClipboardHistDlg(const QStringList &, QWidget *parent = 0);
	~ClipboardHistDlg();

public:
	int		index() const { return m_ix; }

public slots:
	void	accept ();
	
protected:
	//void	closeEvent(QCloseEvent *event);

protected slots:
	void	itemDoubleClicked ( QListWidgetItem * item );

private:
	Ui::ClipboardHistDlg ui;
	int		m_ix;
};

#endif // CLIPBOARDHISTDLG_H
