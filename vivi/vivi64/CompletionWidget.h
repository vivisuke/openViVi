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
	int			count() const;		//	���e�L�X�g��
	QString		text() const;		//	�I���e�L�X�g

public:
	void	setCurrentRow(int);

protected:
	int		setupCandidates();		//	�⊮���� treeWidget �Ɋi�[

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
	QString		m_text;		//	�i�荞�݃e�L�X�g
	QListWidget	*m_listWidget;
	//QTreeWidget	*m_treeWidget;
};

#endif // COMPLETIONWIDGET_H
