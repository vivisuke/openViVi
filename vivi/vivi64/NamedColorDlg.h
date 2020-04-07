#ifndef NAMEDCOLORDLG_H
#define NAMEDCOLORDLG_H

/*

	Copyright (C) 2012 by Nobuhide Tsuda

	RuviEdit のライセンスは MIT＋嫌GPL なライセンスです。 
	無保証・無サポートですが、無償で利用でき、商用アプリでもソースコードを流用することが可能です。 
	（ソースコードを流用した場合、流用部分の著作権・ライセンスはRuviEditのそれのままです） 
	ただし筆者は、プログラマにとって不自由極まりないのに自由だ自由だと言い張るGPL系が嫌いなので、 
	RuviEdit のソースをGPL系プロジェクトで使用することを禁止します。 
	GPLプロジェクトでは一切の流用を禁止しますが、LGPLプロジェクトでは動的リンクによる流用は許可します。

*/

#include <QDialog>

class NamedColorDlg : public QDialog
{
	Q_OBJECT

public:
	NamedColorDlg(QWidget *parent = 0);
	~NamedColorDlg();

public:
	QString colorName() const { return m_colorName; }

public slots:
	void	accept();

protected slots:
	void	setupColorList();
	void	itemDoubleClicked ( class QListWidgetItem * item );

private:
	QString	m_colorName;
	class QLineEdit *m_lineEdit;
	class QListWidget *m_listWidget;
};

#endif // NAMEDCOLORDLG_H
