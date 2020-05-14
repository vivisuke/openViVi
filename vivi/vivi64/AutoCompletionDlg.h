#ifndef AUTOCompletionDLG_H
#define AUTOCompletionDLG_H

#define	BASE_QWIDGET		0

#include <QDialog>

class EditView;
class QListWidget;
class QListWidgetItem;

class AutoCompletionDlg
#if	BASE_QWIDGET
	: public QWidget
#else
	: public QDialog
#endif
{
	Q_OBJECT

public:
	AutoCompletionDlg(const QStringList &, QString = QString(), bool = false, QWidget *parent = 0);
	~AutoCompletionDlg();

public:
	QString		text() const;		//	カレントテキストを返す
	int	count() const;

public:
	void	nextRow();
	void	prevRow();
	void	setFilterText(const QString &);
	bool	appendFilterText(const QString &);		//	絞込テキストに追加
	void	setFilterCaseSensitive(bool b) { m_filterCaseSensitive = b; }
	void	setResizable(bool b);

protected:
	bool	eventFilter ( QObject * watched, QEvent * event );
	void	keyPressEvent(QKeyEvent *event);
	//void	focusOutEvent ( QFocusEvent * event );		//	フォーカスは QListWidget が元々持っているので効かない

protected slots:
	void	itemDoubleClicked ( QListWidgetItem * item );

signals:
	//void	focusOut();
	void	decided(QString, bool = false);
	void	keyPressed(QString);
	void	backSpace();
	void	delPressed(bool ctrl, bool shift);
	void	leftPressed(bool ctrl, bool shift);
	void	rightPressed(bool ctrl, bool shift);
	void	zenCoding();
	void	pasted();

private:
	const QStringList	m_candidates;	//	絞込前のリスト
	bool		m_filterCaseSensitive;		//	絞込時：大文字小文字区別
	QString		m_filterText;		//	絞り込みテキスト
	QListWidget	*m_listWidget;
};

#endif // AUTOCompletionDLG_H
