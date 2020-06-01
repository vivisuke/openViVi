#include "GreppingDlg.h"

GreppingDlg::GreppingDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowStaysOnTopHint);
}
GreppingDlg::~GreppingDlg()
{
}
void GreppingDlg::setGreppingDir(const QString &dirStr)
{
	ui.dir->setText(dirStr);
}
#if		0
void GreppingDlg::terminate()
{
}
#endif
