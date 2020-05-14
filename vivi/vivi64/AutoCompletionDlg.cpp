#include <QtGui>
#include <QBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include "AutoCompletionDlg.h"

//	\n, \n → 半角空白
//	\c → 削除
//	\文字 → 文字
QString escape(const QString &text)
{
	QString dst;
	for (int i = 0; i < text.size(); ) {
		QChar ch = text[i++];
		if( ch == '\\' && i < text.size() ) {
			switch( (ch = text[i++]).unicode() ) {
				case 'n':
				case 't':
					ch = ' ';
					break;
				case 'c':
					continue;
			}
		}
		dst += ch;
	}
	return dst;
}

AutoCompletionDlg::AutoCompletionDlg(const QStringList &lst, QString txt, bool selTail, QWidget *parent)
#if	BASE_QWIDGET
	: QWidget(parent, Qt::FramelessWindowHint)
#else
	: QDialog(parent, Qt::FramelessWindowHint)
#endif
	, m_filterText(txt)
	, m_filterCaseSensitive(true)
	, m_candidates(lst)
{
	setSizeGripEnabled(true);
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
	m_listWidget = new QListWidget;
	m_listWidget->installEventFilter(this);
	layout->addWidget(m_listWidget);
	//foreach(QString text, m_candidates)
	for (int i = 0; i < m_candidates.size(); ++i) {
		QString text = escape(m_candidates[i]);
		QListWidgetItem *item = new QListWidgetItem(text);
		item->setData(Qt::UserRole, i);
		m_listWidget->addItem(item);
		//m_listWidget->addItem(text);
	}
	m_listWidget->setCurrentRow(!selTail ? 0 : m_listWidget->count() - 1);
	connect(m_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
					this, SLOT(itemDoubleClicked(QListWidgetItem *)));
}
AutoCompletionDlg::~AutoCompletionDlg()
{
}
void AutoCompletionDlg::setResizable(bool b)
{
	setSizeGripEnabled(b);
}
QString AutoCompletionDlg::text() const
{
	const QListWidgetItem *ptr = m_listWidget->currentItem();
	if( ptr == 0 ) return QString();
	//return ptr->text();
	return m_candidates[ptr->data(Qt::UserRole).toInt()];
}
int AutoCompletionDlg::count() const
{
	return m_listWidget->count();
}
void AutoCompletionDlg::nextRow()
{
	int ix = m_listWidget->currentRow();
	if( ++ix >= m_listWidget->count() )
		ix = 0;
	m_listWidget->setCurrentRow(ix);
}
void AutoCompletionDlg::prevRow()
{
	int ix = m_listWidget->currentRow();
	if( --ix < 0 )
		ix = m_listWidget->count() - 1;
	m_listWidget->setCurrentRow(ix);
}
void AutoCompletionDlg::setFilterText(const QString &ft)
{
	m_filterText = ft;
	m_listWidget->clear();
	//foreach(QString text, m_candidates)
	const Qt::CaseSensitivity cs = m_filterCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
	for (int i = 0; i < m_candidates.size(); ++i) {
		QString text = escape(m_candidates[i]);
		if( text.startsWith(ft, cs) ) {
			QListWidgetItem *item = new QListWidgetItem(text);
			item->setData(Qt::UserRole, i);
			m_listWidget->addItem(item);
			//m_listWidget->addItem(text);
		}
	}
	m_listWidget->setCurrentRow(0);
}
//	絞込テキストに追加
bool AutoCompletionDlg::appendFilterText(const QString &text)
{
	const Qt::CaseSensitivity cs = m_filterCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
	m_filterText += text;
	for(int i = m_listWidget->count(); --i >= 0;) {
		if( !m_listWidget->item(i)->text().startsWith(m_filterText, cs) ) {
			QListWidgetItem *ptr = m_listWidget->takeItem(i);
			delete ptr;
		}
	}
	if( m_listWidget->count() == 0 )
		emit rejected();
	if( m_listWidget->count() == 1 ) {
		QListWidgetItem *item = m_listWidget->item(0);
		QString txt = m_candidates[item->data(Qt::UserRole).toInt()];
		if( txt == m_filterText ) {
			emit decided(txt, true);
			return true;
		}
	}
	return false;
}
void AutoCompletionDlg::itemDoubleClicked( QListWidgetItem * item )
{
	emit decided(m_candidates[item->data(Qt::UserRole).toInt()]);
}
//void AutoCompletionDlg::focusOutEvent( QFocusEvent * event )
//{
//	QDialog::focusOutEvent(event);
//	emit focusOut();
//}
void AutoCompletionDlg::keyPressEvent(QKeyEvent *event)
{
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	QString text = event->text();
	if( text.isEmpty() ) return;
	if( text == "\r" || text == "\n" ) {
		if( ctrl ) {		//	Ctrl が押された場合は、入力文字だけで確定
			emit rejected();
			return;
		}
		const QListWidgetItem *item = m_listWidget->currentItem();
		if( item != 0 )
			emit decided(m_candidates[item->data(Qt::UserRole).toInt()]);
		return;
	}
	if( text == "\x1b" ) {		//	Esc
		emit rejected();
		return;
	}
	if( text[0].unicode() >= 0x20 && text[0].unicode() < 0x7f || text[0] == '\t' ) {
		if( !appendFilterText(text) )
			emit keyPressed(text);
	}
}
bool AutoCompletionDlg::eventFilter ( QObject * obj, QEvent * event )
{
	if( obj == m_listWidget ) {
		if( event->type() == QEvent::FocusOut ) {
			emit rejected();
			return true;
		}
		if( event->type() == QEvent::KeyPress ) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			const bool ctrl = (keyEvent->modifiers() & Qt::ControlModifier) != 0;
			const bool shift = (keyEvent->modifiers() & Qt::ShiftModifier) != 0;
			const bool alt = (keyEvent->modifiers() & Qt::AltModifier) != 0;
			QString text = keyEvent->text();
			int key = keyEvent->key();
			if( key == Qt::Key_Backspace ) {
				emit backSpace();
				return true;
			}
			if( key == Qt::Key_Delete ) {
				emit delPressed(ctrl, shift);
				return true;
			}
			if( key == Qt::Key_Left) {
				emit leftPressed(ctrl, shift);
				return true;
			}
			if( key == Qt::Key_Right) {
				emit rightPressed(ctrl, shift);
				return true;
			}
			if( text.isEmpty() ) return false;
			if( !ctrl ) {
				if( text[0].unicode() >= 0x20 && text[0].unicode() <= 0x7f || text[0] == '\t' ) {
					if( !appendFilterText(text) )
						emit keyPressed(text);
					return true;
				}
			} else {
				if( key == Qt::Key_J ) {
					nextRow();
					return true;
				}
				if( key == Qt::Key_K ) {
					prevRow();
					return true;
				}
				if( key == Qt::Key_Comma ) {
					emit zenCoding();
					return true;
				}
				if( key == Qt::Key_V ) {
					emit pasted();
					return true;
				}
			}
		}
	}
	return false;
}
