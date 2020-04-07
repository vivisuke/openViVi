#include <QtGui>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qmenu.h>
#include <QDebug>
#include "TypeStgDlg.h"
#include "typeSettings.h"
#include "EditView.h"
#include "NamedColorDlg.h"

TypeStgDlg::TypeStgDlg(EditView *view, TypeSettings *typeStg, QWidget *parent)
	: QDialog(parent)
	, m_view(view)
	, m_typeSettings(typeStg)
{
	ui.setupUi(this);
	setWindowTitle(m_typeSettings->name() + tr(" - TypeStgDlg"));
	QFontDatabase db;
	ui.fontFamilyCB->addItems(db.families());
	setFontFamily(m_typeSettings->textValue(TypeSettings::FONT_NAME));
	ui.fontSizeSB->setValue(m_typeSettings->intValue(TypeSettings::FONT_SIZE));
	ui.tab2->setChecked(m_typeSettings->intValue(TypeSettings::TAB_WIDTH) == 2);
	ui.tab4->setChecked(m_typeSettings->intValue(TypeSettings::TAB_WIDTH) == 4);
	ui.tab8->setChecked(m_typeSettings->intValue(TypeSettings::TAB_WIDTH) == 8);
	connect(ui.tab2, SIGNAL(toggled(bool)), this, SLOT(onTab2(bool)));
	connect(ui.tab4, SIGNAL(toggled(bool)), this, SLOT(onTab4(bool)));
	connect(ui.tab8, SIGNAL(toggled(bool)), this, SLOT(onTab8(bool)));
	ui.lineComment->setText(m_typeSettings->textValue(TypeSettings::LINE_COMMENT));
	ui.blockCommentBeg->setText(m_typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG));
	ui.blockCommentEnd->setText(m_typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END));
	connect(ui.lineComment, SIGNAL(editingFinished()), this, SLOT(onCommentChanged()));
	connect(ui.blockCommentBeg, SIGNAL(editingFinished()), this, SLOT(onCommentChanged()));
	connect(ui.blockCommentEnd, SIGNAL(editingFinished()), this, SLOT(onCommentChanged()));
	connect(ui.reset, SIGNAL(clicked()), this, SLOT(onReset()));
	connect(ui.load, SIGNAL(clicked()), this, SLOT(onLoad()));
	connect(ui.save, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(ui.fontFamilyCB, SIGNAL(currentIndexChanged(const QString &)),
			this, SLOT(onFontNameChanged(const QString &)));
	connect(ui.fontSizeSB, SIGNAL(valueChanged(int)),
			this, SLOT(onFontSizeChanged(int)));
	ui.keyWord1->setText(m_typeSettings->textValue(TypeSettings::KEYWORD1_FILE));
	ui.keyWord2->setText(m_typeSettings->textValue(TypeSettings::KEYWORD2_FILE));
	connect(ui.keyWord1, SIGNAL(textEdited(const QString &)),
			this, SLOT(onKeyword1Edited(const QString &)));
	connect(ui.keyWord2, SIGNAL(textEdited(const QString &)),
			this, SLOT(onKeyword2Edited(const QString &)));
	//QList<QToolButton *> lst = findChildren<QToolButton *>("tbText");
	//qDebug() << lst;
	connect(ui.refKW1, SIGNAL(clicked()), this, SLOT(refKW1Clicked()));
	connect(ui.refKW2, SIGNAL(clicked()), this, SLOT(refKW2Clicked()));
	setupColorButtons();
	setupCheckButtons();
}

TypeStgDlg::~TypeStgDlg()
{
}
void TypeStgDlg::accept()
{
	m_typeSettings->setBoolValue(TypeSettings::STATEMENT_COMPLETION, ui.statementCompletion->isChecked());
	m_typeSettings->setBoolValue(TypeSettings::WORD_COMPLETION, ui.wordCompletion->isChecked());
	m_typeSettings->setBoolValue(TypeSettings::KEYWORD_COMPLETION, ui.keywordCompletion->isChecked());
	QDialog::accept();
}
void TypeStgDlg::setFontFamily(const QString &name)
{
	int ix = ui.fontFamilyCB->findText(name);
	if( ix >= 0 )
		ui.fontFamilyCB->setCurrentIndex(ix);
}
void TypeStgDlg::onReset()
{
	m_typeSettings->reset();
	setupColorButtons();
	setupCheckButtons();
}
void TypeStgDlg::onLoad()
{
	QString fileName = "*.stg";
	fileName = QFileDialog::getOpenFileName(this, tr("Load Color File"), fileName, "*.stg");
	if( fileName.isEmpty() ) return;
	if( m_typeSettings->load(fileName) ) {
		setupColorButtons();
		setupCheckButtons();
	}
}
void TypeStgDlg::onSave()
{
	QString fileName = "*.stg";
	fileName = QFileDialog::getSaveFileName(this, tr("Save Color File"), fileName, "*.stg");
	if( !fileName.isEmpty() )
		m_typeSettings->save(fileName);
}
void TypeStgDlg::onCheckToggled(bool vi)
{
	QCheckBox *chk = qobject_cast<QCheckBox *>(sender());
#if		1
	QString name = chk->objectName().mid(3);		//	remove head "chk"
	qDebug() << name;
	int ix = m_typeSettings->indexOfBool(name);
#else
	QString txt = chk->text();
	if( txt.right(1) == QChar(':') || txt.right(1) == QChar(L'ÅF') )
		txt = txt.left(txt.size() - 1);
	qDebug() << txt;
	int ix = m_typeSettings->indexOfBool(txt);
#endif
	if( ix >= 0 ) {
		m_typeSettings->setBoolValue(ix, vi);
		if( ix == TypeSettings::VIEW_LINENUM )
			m_view->setLineNumberVisible(vi);
	}
}
void setButtonColor(QToolButton *btn, const QColor &col)
{
	QPixmap pixmap(48, 48);
	pixmap.fill(col);
	btn->setIcon(QIcon(pixmap));
}
void TypeStgDlg::setupColorButtons()
{
	for(int cix = 0; cix < m_typeSettings->nColor(); ++cix) {
		QList<QToolButton *> lst = findChildren<QToolButton *>(QString("tb")
																+ m_typeSettings->colorKey(cix));
		if( lst.isEmpty() ) continue;
		QToolButton *btn = lst[0];
		m_ixToButtonHash.insert(cix, btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(btnSelectColor()));
		QMenu *menu = new QMenu();
		QAction *btnSelectColorAct = new QAction(this);
		btnSelectColorAct->setData(cix);
		btnSelectColorAct->setIcon(btn->icon());
		btn->setDefaultAction(btnSelectColorAct);
		QAction *selectColorAct = new QAction(tr("SelectColor"), this);
		selectColorAct->setData(cix);
		menu->addAction(selectColorAct);
		connect(selectColorAct, SIGNAL(triggered()), this, SLOT(selectColor()));
		QAction *namedColorAct = new QAction(tr("NamedColor"), this);
		namedColorAct->setData(cix);
		connect(namedColorAct, SIGNAL(triggered()), this, SLOT(namedColor()));
		menu->addAction(namedColorAct);
		btn->setMenu(menu);
		setButtonColor(btn, m_typeSettings->color(cix));
	}
}
void TypeStgDlg::setupCheckButtons()
{
	for(int ix = 0; ix < m_typeSettings->nBool(); ++ix) {
		QString key = QString("chk") + m_typeSettings->viewItemKey(ix);
		//qDebug() << key;
		QList<QCheckBox *> lst = findChildren<QCheckBox *>(key);
		if( lst.isEmpty() ) continue;
		QCheckBox *chk = lst[0];
		chk->setChecked(m_typeSettings->boolValue(ix));
		connect(chk, SIGNAL(toggled(bool)), SLOT(onCheckToggled(bool)));
	}
	ui.statementCompletion->setChecked(m_typeSettings->boolValue(TypeSettings::STATEMENT_COMPLETION));
	ui.wordCompletion->setChecked(m_typeSettings->boolValue(TypeSettings::WORD_COMPLETION));
	ui.keywordCompletion->setChecked(m_typeSettings->boolValue(TypeSettings::KEYWORD_COMPLETION));
}
void TypeStgDlg::btnSelectColor()
{
	QToolButton *btn = qobject_cast<QToolButton *>(sender());
	int colorIX = btn->defaultAction()->data().toInt();
	selectColor(colorIX);
}
void TypeStgDlg::selectColor()
{
	QAction *act = qobject_cast<QAction *>(sender());
	int colorIX = act->data().toInt();
	selectColor(colorIX);
}
void TypeStgDlg::selectColor(int colorIX)
{
	QColorDialog aDlg;
	aDlg.setCurrentColor(m_typeSettings->color(colorIX));
	if( QDialog::Accepted == aDlg.exec() ) {
		QColor col = aDlg.currentColor();
		QToolButton *btn = m_ixToButtonHash.value(colorIX);
		setButtonColor(btn, col);
		m_typeSettings->setColor(colorIX, col);
		m_view->update();
	}
}
void TypeStgDlg::namedColor()
{
	QAction *act = qobject_cast<QAction *>(sender());
	int colorIX = act->data().toInt();
	NamedColorDlg aDlg;
	if( QDialog::Accepted == aDlg.exec() ) {
		QToolButton *btn = m_ixToButtonHash.value(colorIX);
		QColor col(aDlg.colorName());
		setButtonColor(btn, col);
		m_typeSettings->setColor(colorIX, col);
		m_view->update();
	}
}
void TypeStgDlg::onFontNameChanged(const QString &fontName)
{
	m_typeSettings->setTextValue(TypeSettings::FONT_NAME, fontName);
	m_view->updateFont();
}
void TypeStgDlg::onFontSizeChanged(int v)
{
	m_typeSettings->setIntValue(TypeSettings::FONT_SIZE, v);
	m_view->updateFont();
}
void TypeStgDlg::onTab2(bool b)
{
	if( b ) {
		m_typeSettings->setIntValue(TypeSettings::TAB_WIDTH, 2);
		m_view->update();
	}
}
void TypeStgDlg::onTab4(bool b)
{
	if( b ) {
		m_typeSettings->setIntValue(TypeSettings::TAB_WIDTH, 4);
		m_view->update();
	}
}
void TypeStgDlg::onTab8(bool b)
{
	if( b ) {
		m_typeSettings->setIntValue(TypeSettings::TAB_WIDTH, 8);
		m_view->update();
	}
}
void TypeStgDlg::onCommentChanged()
{
	m_typeSettings->setTextValue(TypeSettings::LINE_COMMENT, ui.lineComment->text());
	m_typeSettings->setTextValue(TypeSettings::BLOCK_COMMENT_BEG, ui.blockCommentBeg->text());
	m_typeSettings->setTextValue(TypeSettings::BLOCK_COMMENT_END, ui.blockCommentEnd->text());
	m_view->update();
}
void TypeStgDlg::onKeyword1Edited(const QString &text)
{
	m_typeSettings->setTextValue(TypeSettings::KEYWORD1_FILE, text);
	m_view->update();
}
void TypeStgDlg::onKeyword2Edited(const QString &text)
{
	m_typeSettings->setTextValue(TypeSettings::KEYWORD2_FILE, text);
	m_view->update();
}
void TypeStgDlg::refKW1Clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("keyword-1 file"));
	if( fileName.isEmpty() ) return;
	ui.keyWord1->setText(fileName);
	m_typeSettings->setTextValue(TypeSettings::KEYWORD1_FILE, fileName);
	m_view->update();
}
void TypeStgDlg::refKW2Clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("keyword-2 file"));
	if( fileName.isEmpty() ) return;
	ui.keyWord2->setText(fileName);
	m_typeSettings->setTextValue(TypeSettings::KEYWORD2_FILE, fileName);
	m_view->update();
}
