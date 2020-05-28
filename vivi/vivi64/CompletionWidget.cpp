/*

	Copyright (C) 2012 by Nobuhide Tsuda


*/
#include <QtGui>
#include <QVBoxLayout>
#include <QToolTip>
#include "CompletionWidget.h"
#include "EditView.h"

typedef const char cchar;

CompletionWidget::CompletionWidget(EditView *editor,
									const QStringList &candidates,	//	補完候補、空ならばキーワード補完
									QString text,		//	カーソル直前テキスト
									QWidget *parent)
	: QDialog(parent, Qt::FramelessWindowHint)
	, m_editor(editor)
	, m_candidates(candidates)
	, m_text(text)
{
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);
#if 0
	m_treeWidget = new QTreeWidget;
	layout->addWidget(m_treeWidget);
	//m_treeWidget->setColumnCount(2);
	//m_treeWidget->setColumnWidth(0, 100);
	m_treeWidget->setHeaderHidden(true);	//	ヘッダ非表示
	setupCandidates();
	m_treeWidget->setCurrentItem(m_treeWidget->itemAt(0, 0));	//	最初のアイテム選択
#else
	m_listWidget = new QListWidget;
	layout->addWidget(m_listWidget);
	int k = setupCandidates();
	m_listWidget->setCurrentRow(k);
	m_listWidget->installEventFilter(this);
	//m_listWidget->setAttribute(Qt::WA_AlwaysShowToolTips);
	//m_listWidget->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum));
	connect(m_listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));
	connect(m_listWidget, SIGNAL(itemDoubleClicked (QListWidgetItem *)),
				this, SLOT(itemDoubleClicked(QListWidgetItem *)));
#endif
	QListWidgetItem *item = m_listWidget->item(k);
	QRect rect = m_listWidget->visualItemRect(item);
	const int nht = rect.height() * (m_listWidget->count() + 1);		///
	if( height() > nht )
		resize(240, nht);
	//setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum));
	setAttribute(Qt::WA_TranslucentBackground);

	//currentRowChanged(k);		12/05/09 何故か表示されない
}

CompletionWidget::~CompletionWidget()
{

}
void CompletionWidget::setCurrentRow(int ix)
{
	m_listWidget->setCurrentRow(ix);
}
void CompletionWidget::currentRowChanged(int ix)
{
	if( ix < 0 ) return;
	QListWidgetItem *item = m_listWidget->item(ix);
	QRect rect = m_listWidget->visualItemRect(item);
	QPoint gp = m_listWidget->mapToGlobal(QPoint(rect.right(), rect.y() - rect.height()));
	QToolTip::showText(gp, item->toolTip());
#if 0
	QHelpEvent *event = new QHelpEvent(QEvent::ToolTip,
                                       QPoint(0, 0),
                                       QPoint(gp.x(), gp.y()));  

    QApplication::postEvent(m_listWidget, event);
#endif
}
void CompletionWidget::itemDoubleClicked(QListWidgetItem *)
{
	accept();
}
#if 0
void CompletionWidget::currentItemChanged(QListWidgetItem *item)
{
}
#endif
//	return: デフォルト候補位置
int CompletionWidget::setupCandidates()
{
	m_listWidget->clear();
	QListWidgetItem *item = 0;
	int i = 0;
	int k = 0;
	if( m_candidates.isEmpty() ) {
#if		0
		for(int ix = 0; keywords[ix].m_text != 0; ++ix) {
			QString kw(keywords[ix].m_text);
			if( !m_text.isEmpty() && !kw.startsWith(m_text, Qt::CaseInsensitive) )
				continue;
			item = new QListWidgetItem(kw);
			item->setToolTip(keywords[ix].m_desc);
			//item->setAttribute(WA_AlwaysShowToolTips);
			m_listWidget->insertItem(i++, item);
		}
#endif
	} else {
		foreach(const QString cText, m_candidates) {
			if( cText.startsWith(m_text, Qt::CaseInsensitive) ) {
				if( cText == m_text ) k = i;
				item = new QListWidgetItem(cText);
				m_listWidget->insertItem(i++, item);
			}
		}
		//if( k >= 0 )
		//	m_listWidget->setCurrentRow(k);
	}
	if( !m_listWidget->count() ) {
		reject();
		return 0;
	}
#if 0	//	正しく動作しないので、とりあえずコメントアウト 12/04/04
	if( item != 0 && m_listWidget->count() < 8 ) {
		//int ht0 = item->sizeHint().height();
		int ht = QFontMetrics(item->font()).height();	//	1アイテム高さ
		//QSize gs = m_listWidget->gridSize();
		QSize sz = size();
		sz.setHeight(ht * m_listWidget->count());
		resize(sz);
	}
#endif
	if( k > 0 ) --k;
	return k;
}
#if 0
void CompletionWidget::accept()
{
}
#endif
int CompletionWidget::count() const
{
	return m_listWidget->count();
}
QString CompletionWidget::text() const
{
#if 0
	QList<QTreeWidgetItem *> lst = m_treeWidget->selectedItems();
	if( lst.isEmpty() ) return QString();
	QTreeWidgetItem *item = lst[0];
	return item->text(0);
#else
	QList<QListWidgetItem *> lst = m_listWidget->selectedItems();
	if( lst.isEmpty() ) return QString();
	QListWidgetItem *item = lst[0];
	return item->text();
#endif
}
bool CompletionWidget::eventFilter ( QObject * obj, QEvent * event )
{
	if( obj == m_listWidget ) {
		if( event->type() == QEvent::KeyPress ) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			//if( keyEvent->key() != Qt::Key_Return ) {
				QString text = keyEvent->text();
				if( !text.isEmpty() && text[0].unicode() >= 0x21 && text[0].unicode() <= 0x7f ) {
					///m_editor->textCursor().insertText(text);
					m_text += text;
					emit textChanged(m_text);
					setupCandidates();
					if( !m_listWidget->count() )
						reject();
					else {
						m_listWidget->setCurrentRow(0);
						if( m_listWidget->count() == 1 )
							accept();
					}
					return true;
				}
			//}
		}
		//else if( event->type() == QEvent::MouseButtonDblClick ) {
		//	accept();
		//	return true;
		//}
	}
	return false;
}
void CompletionWidget::keyPressEvent ( QKeyEvent * event )
{
	const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	const bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	if( event->key() == Qt::Key_Return ) {
		accept();
		return;
	}
	if( event->key() == Qt::Key_Escape ) {
		reject();
		return;
	}
	if( event->key() == Qt::Key_Backspace ) {
		done('\b');
		return;
	}
	if( ctrl && !shift && !alt ) {
		if( event->key() == Qt::Key_K || event->key() == Qt::Key_L ) {
			int ix = m_listWidget->currentRow();
			if( ix != 0 )
				m_listWidget->setCurrentRow(ix - 1);
			else
				m_listWidget->setCurrentRow(m_listWidget->count() - 1);
			return;
		}
		if( event->key() == Qt::Key_J ) {
			int ix = m_listWidget->currentRow();
			if( ++ix < m_listWidget->count() )
				m_listWidget->setCurrentRow(ix);
			else
				m_listWidget->setCurrentRow(0);
			return;
		}
	}
#if 0
	QString text = event->text();
	if( text.isEmpty() ) return;
	m_text += text;
	setupCandidates();
	m_listWidget->setCurrentRow(0);
#endif
}
