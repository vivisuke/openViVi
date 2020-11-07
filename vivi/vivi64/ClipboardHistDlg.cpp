#include <QtGui>
#include "ClipboardHistDlg.h"

ClipboardHistDlg::ClipboardHistDlg(const QStringList &lst, QWidget *parent)
	: QDialog(parent)
	, m_ix(-1)
{
	ui.setupUi(this);
	foreach(QString txt, lst) {
		txt.replace("\r\n", " ");
		txt.replace("\r", " ");
		txt.replace("\n", " ");
		txt.replace("\t", "  ");		//	半角空白*2
		ui.listWidget->addItem(txt);
	}
	ui.listWidget->setCurrentRow(0);
	connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), 
					this, SLOT(itemDoubleClicked(QListWidgetItem *)));
}
ClipboardHistDlg::~ClipboardHistDlg()
{
}
void ClipboardHistDlg::accept ()
{
	m_ix = ui.listWidget->currentRow();
	QDialog::accept();
}
void ClipboardHistDlg::itemDoubleClicked( QListWidgetItem * item )
{
	accept();
}
#if		0
void ClipboardHistDlg::closeEvent(QCloseEvent *event)
{
	m_ix = ui.listWidget->currentRow();
	QDialog::closeEvent(event);
}
#endif
