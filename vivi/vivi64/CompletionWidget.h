#ifndef COMPLETIONWIDGET_H
#define COMPLETIONWIDGET_H

/*

	Copyright (C) 2012 by Nobuhide Tsuda


*/

#include <QDialog>
#include <QListWidget>

class QListWidget;
class QTreeWidget;
class EditView;

class CompletionWidget : public QDialog
{
	Q_OBJECT

public:
	CompletionWidget(EditView *, const QStringList &, QString text, QWidget *parent = 0);
	~CompletionWidget();

public:
	int			count() const;		//	候補テキスト数
	QString		text() const;		//	選択テキスト

public:
	void	setCurrentRow(int);

protected:
	int		setupCandidates();		//	補完候補を treeWidget に格納

protected:
	//void accept();
	bool	eventFilter ( QObject * watched, QEvent * event );
	void	keyPressEvent ( QKeyEvent * event );

protected slots:
	void	currentRowChanged(int);
	void	itemDoubleClicked(QListWidgetItem *);
	//void	currentItemChanged(QListWidgetItem *);

signals:
	void	textChanged(const QString &);

private:
	EditView	*m_editor;
	const QStringList	m_candidates;
	QString		m_text;		//	絞り込みテキスト
	QListWidget	*m_listWidget;
	//QTreeWidget	*m_treeWidget;
};

#endif // COMPLETIONWIDGET_H
