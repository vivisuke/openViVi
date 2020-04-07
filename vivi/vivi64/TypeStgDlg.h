#ifndef TYPESTGDLG_H
#define TYPESTGDLG_H

#include <QDialog>
#include "ui_TypeStgDlg.h"

class TypeSettings;
class EditView;

class TypeStgDlg : public QDialog
{
	Q_OBJECT

public:
	TypeStgDlg(EditView *, TypeSettings *typeStg, QWidget *parent = 0);
	~TypeStgDlg();

protected:
	void	setupColorButtons();
	void	setupCheckButtons();
	void	selectColor(int);
	void	setFontFamily(const QString &);

protected slots:
	void	btnSelectColor();
	void	selectColor();
	void	namedColor();
	void	onCheckToggled(bool);
	void	onReset();
	void	onLoad();
	void	onSave();
	void	onFontNameChanged(const QString &);
	void	onFontSizeChanged(int);
	void	onTab2(bool);
	void	onTab4(bool);
	void	onTab8(bool);
	void	onCommentChanged();
	void	onKeyword1Edited(const QString &);
	void	onKeyword2Edited(const QString &);
	void	refKW1Clicked();
	void	refKW2Clicked();
	void	accept();

private:
	Ui::TypeStgDlg ui;
	EditView	*m_view;
	TypeSettings	*m_typeSettings;
	QHash<int, QToolButton *>	m_ixToButtonHash;
};

#endif // TYPESTGDLG_H
