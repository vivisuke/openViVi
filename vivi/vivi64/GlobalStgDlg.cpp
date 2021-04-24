#include <QtGui>
#include <QFileDialog>
#include "GlobalStgDlg.h"
#include "globalSettings.h"

GlobalStgDlg::GlobalStgDlg(GlobalSettings *globSettings, QWidget *parent)
	: QDialog(parent)
	, m_globSettings(globSettings)
{
	ui.setupUi(this);
	QStringList lst;
	lst << "UTF-8" << "UTF-16LE" << "UTF-16BE" << "Sjift_JIS" << "EUC";
	ui.charEncodingCB->addItems(lst);
	ui.charEncodingCB->setCurrentIndex(m_globSettings->enumValue(GlobalSettings::CHAR_ENCODING));
	ui.withBOM->setChecked(m_globSettings->boolValue(GlobalSettings::WITH_BOM));
	lst.clear();
	lst << "Default" << "CPP" << "C#" << "CSS" << "F#" << "HLSL" << "HTML"
			<< "JAVA" << "JS" << "TS" << "LOG" << "MARKDN" << "PASCAL" << "PERL" << "PHP"
			<< "PYTHON" << "GDSCRIPT" << "RUBY" << "SQL" << "TXT";
	ui.docTypeCB->addItems(lst);
	ui.docTypeCB->setCurrentIndex(m_globSettings->enumValue(GlobalSettings::DOC_TYPE));
	ui.docTypeCB->setMaxVisibleItems(ui.docTypeCB->count());
	//ui.pict1FilePath->setText(m_globSettings->textValue(GlobalSettings::PICTURE1_PATH));
	//connect(ui.pict1Ref, SIGNAL(clicked()), this, SLOT(refPict1FilePath()));
	//ui.pict2FilePath->setText(m_globSettings->textValue(GlobalSettings::PICTURE2_PATH));
	//connect(ui.pict2Ref, SIGNAL(clicked()), this, SLOT(refPict2FilePath()));
	//ui.alphaSB->setValue(m_globSettings->intValue(GlobalSettings::PICTURE_OPACITY)/100.0);
	//ui.scaleSB->setValue(m_globSettings->intValue(GlobalSettings::PICTURE_SCALE)/100.0);
	//ui.ZenCodingFilePath->setText(m_globSettings->textValue(GlobalSettings::ZEN_CODING_PATH));
	//connect(ui.ZCFRef, SIGNAL(clicked()), this, SLOT(ZenCodingFilePath()));
	ui.htdocsRoot->setText(m_globSettings->textValue(GlobalSettings::HTDOCS_ROOT));
	connect(ui.htdocsRef, SIGNAL(clicked()), this, SLOT(htdocsRootPath()));
	//ui.statementCompletion->setChecked(m_globSettings->boolValue(GlobalSettings::STATEMENT_COMPLETION));
	//ui.wordCompletion->setChecked(m_globSettings->boolValue(GlobalSettings::WORD_COMPLETION));
	//ui.keywordCompletion->setChecked(m_globSettings->boolValue(GlobalSettings::KEYWORD_COMPLETION));
	ui.MiniMap->setChecked(m_globSettings->boolValue(GlobalSettings::MINI_MAP));
	ui.OpenOpenedDocs->setChecked(m_globSettings->boolValue(GlobalSettings::OPEN_OPENED_DOCS));

	QFontDatabase db;
	ui.fontFamilyCB->addItems(db.families());
	setFontFamily(m_globSettings->textValue(GlobalSettings::OUTPUT_FONT_NAME));
	ui.fontSizeSB->setValue(m_globSettings->intValue(GlobalSettings::OUTPUT_FONT_SIZE));
	ui.outputBar->setChecked(!m_globSettings->boolValue(GlobalSettings::OUTPUT_VIEW));
	ui.outputView->setChecked(m_globSettings->boolValue(GlobalSettings::OUTPUT_VIEW));
}
GlobalStgDlg::~GlobalStgDlg()
{
}
void GlobalStgDlg::setFontFamily(const QString &name)
{
	int ix = ui.fontFamilyCB->findText(name);
	if( ix >= 0 )
		ui.fontFamilyCB->setCurrentIndex(ix);
}
void GlobalStgDlg::accept()
{
	m_globSettings->setEnumValue(GlobalSettings::CHAR_ENCODING, ui.charEncodingCB->currentIndex());
	m_globSettings->setBoolValue(GlobalSettings::WITH_BOM, ui.withBOM->isChecked());
	//m_globSettings->setBoolValue(GlobalSettings::STATEMENT_COMPLETION, ui.statementCompletion->isChecked());
	//m_globSettings->setBoolValue(GlobalSettings::WORD_COMPLETION, ui.wordCompletion->isChecked());
	//m_globSettings->setBoolValue(GlobalSettings::KEYWORD_COMPLETION, ui.keywordCompletion->isChecked());
	m_globSettings->setEnumValue(GlobalSettings::DOC_TYPE, ui.docTypeCB->currentIndex());
	m_globSettings->setTextValue(GlobalSettings::OUTPUT_FONT_NAME, ui.fontFamilyCB->currentText());
	m_globSettings->setIntValue(GlobalSettings::OUTPUT_FONT_SIZE, ui.fontSizeSB->value());
	m_globSettings->setBoolValue(GlobalSettings::OUTPUT_VIEW, ui.outputView->isChecked());
	//m_globSettings->setTextValue(GlobalSettings::PICTURE1_PATH, ui.pict1FilePath->text());
	//m_globSettings->setTextValue(GlobalSettings::PICTURE2_PATH, ui.pict2FilePath->text());
	m_globSettings->setTextValue(GlobalSettings::ZEN_CODING_PATH, ui.ZenCodingFilePath->text());
	m_globSettings->setTextValue(GlobalSettings::HTDOCS_ROOT, ui.htdocsRoot->text());
	//qDebug() << m_globSettings->textValue(GlobalSettings::PICTURE1_PATH);
	//m_globSettings->setIntValue(GlobalSettings::PICTURE_OPACITY, (int)(ui.alphaSB->value() * 100));
	//m_globSettings->setIntValue(GlobalSettings::PICTURE_SCALE, (int)(ui.scaleSB->value() * 100));
	m_globSettings->setBoolValue(GlobalSettings::MINI_MAP, ui.MiniMap->isChecked());
	m_globSettings->setBoolValue(GlobalSettings::OPEN_OPENED_DOCS, ui.OpenOpenedDocs->isChecked());
	QDialog::accept();
}
#if 0
void GlobalStgDlg::refPict1FilePath()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Pictue File Name");
	if( !fileName.isEmpty() )
		ui.pict1FilePath->setText(fileName);
}
void GlobalStgDlg::refPict2FilePath()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Pictue File Name");
	if( !fileName.isEmpty() )
		ui.pict2FilePath->setText(fileName);
}
#endif
void GlobalStgDlg::ZenCodingFilePath()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Zen-Coding File Name");
	if( !fileName.isEmpty() )
		ui.ZenCodingFilePath->setText(fileName);
}
void GlobalStgDlg::htdocsRootPath()
{
	QString dir = QFileDialog::getExistingDirectory(this, "HTML Docs Root Dir");
	if( !dir.isEmpty() ) {
		dir.replace("\\", "/");
		ui.htdocsRoot->setText(dir);
	}
}
