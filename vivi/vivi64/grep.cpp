//----------------------------------------------------------------------
//
//			File:			"grep.cpp"
//			Created:		02-10-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include <QMessageBox>
#include <QSettings>
#include "mainwindow.h"
#include "EditView.h"
#include "TextCursor.h"
#include "globalSettings.h"
#include "GrepDlg.h"
#include "GreppingDlg.h"
//###include "OutputView.h"
#include "charEncoding.h"
#include "typeSettings.h"
#include "../buffer/Buffer.h"
#include "../buffer/sssearch.h"
#include "GrepEngine.h"

bool isEditViewFocused(QWidget* w);
bool isEditView(const QWidget* w);

void MainWindow::on_action_GrepCurWord_triggered()
{
	grep(true);
}
void MainWindow::on_action_Grep_triggered()
{
	grep(false);
}
void MainWindow::grep(bool curWord)
{
	if( m_grepEngine != 0 ) return;
#if 0
	if( !certified() && !m_possibleGrepCount ) {
		QMessageBox::information(this, "SakuSakuEditor",
										tr("You can't Grep, beause of Grep Count is 0\n")
										+ tr("Grep Count will increse every 10 minits."));
		return;
	}
#endif
	GrepDlg aDlg(globSettings(), m_grepDirHist, m_possibleGrepCount);
	//EditView *view = (EditView *)ui.tabWidget->currentWidget();
	EditView *view = currentWidget();
	if( isEditViewFocused(view) ) {
		if( curWord ) {		//	カーソル位置単語を検索パターンに設定
			TextCursor cur(*view->textCursor());
			cur.movePosition(TextCursor::BEG_WORD);
			cur.movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
			aDlg.setFindString(cur.selectedText());
		} else if( view->hasSelectionInALine() ) {
			aDlg.setFindString(view->selectedText());
		}
	}
	if( isEditView(view) ) {
		aDlg.setTypeName(view->typeSettings()->name());
	}
	if( aDlg.exec() != QDialog::Accepted ) return;
	QSettings settings;
	settings.setValue("grepExclude", aDlg.exclude());
	//if( !certified() && !view->isSakuSakuGame() )
	//	--m_possibleGrepCount;
	globSettings()->setBoolValue(GlobalSettings::IGNORE_CASE, aDlg.ignoreCase());
	globSettings()->setBoolValue(GlobalSettings::WHOLE_WORD_ONLY, aDlg.wholeWordOnly());
	globSettings()->setBoolValue(GlobalSettings::REGEXP, aDlg.regExp());
	globSettings()->setBoolValue(GlobalSettings::GREP_SUB_DIR, aDlg.grepSubDir());
	const bool bGrepView = aDlg.grepView();
	globSettings()->setBoolValue(GlobalSettings::GREP_VIEW, bGrepView);
	QString dirStr = aDlg.dir();
	int ix = m_grepDirHist.indexOf(dirStr);
	if( ix >= 0 ) m_grepDirHist.removeAt(ix);
	m_grepDirHist.push_front(dirStr);
	setFindString(aDlg.findString());
	updateFindStringCB();
	
	m_grepEngine = new GrepEngine(globSettings());
	m_grepEngine->moveToThread(&m_thread);
	connect(this, SIGNAL(doGrep(QString, QString, QString, QString)), m_grepEngine, SLOT(doGrep(QString, QString, QString, QString)));
	connect(m_grepEngine, SIGNAL(finished(int)), this, SLOT(grepFinished(int)));
	const QString mess = tr("* grepping '%1' at %2, %3 ...\n")
					.arg(aDlg.findString()).arg(dirStr).arg(aDlg.extentions());
#if 0
	if( bGrepView ) {
		addNewView(m_grepView = createView(), QString("Grep%1").arg(++m_seqGrepView));
		m_grepView->setGrepView();
		connect(m_grepEngine, SIGNAL(doOutput(const QString &)), this, SLOT(doOutputToGrepView(const QString &)));		//##
		doOutputToGrepView(mess);
	} else 
#endif
	{
		connect(m_grepEngine, SIGNAL(doOutput(const QString &)), this, SLOT(doOutputToBar(const QString &)));		//##
		m_outputWidget->clear();
		doOutput(mess);
	}
	GreppingDlg dlg;		//	グレップ中ダイアログ
	//connect(&dlg, SIGNAL(terminate()), m_grepEngine, SLOT(terminate()));
	connect(m_grepEngine, SIGNAL(greppingDir(const QString &)), &dlg, SLOT(setGreppingDir(const QString &)));
	connect(m_grepEngine, SIGNAL(finished(int)), &dlg, SLOT(reject()));
	
	///QMessageBox mb();
	
	//m_grepEngine->doGrep(aDlg.findString(), aDlg.extentions(), dirStr);
	emit doGrep(aDlg.findString(), aDlg.findString(), dirStr, aDlg.exclude());
#if	0
	const bool b = QMetaObject::invokeMethod(m_grepEngine, "doGrep",
								//Qt::QueuedConnection,
								//Q_RETURN_ARG(),
								Q_ARG(QString, aDlg.findString()),
								Q_ARG(QString, aDlg.extentions()),
								Q_ARG(QString, dirStr),
								Q_ARG(QString, aDlg.exclude()));
#endif

	dlg.exec();
	if( m_grepEngine != 0 )
		m_grepEngine->terminate();
	if( !bGrepView )
		m_outputWidget->setFocus();
}
void MainWindow::grepFinished(int cnt)
{
	delete m_grepEngine;
	m_grepEngine = 0;
	const QString mess = tr("\n* grep finished, %1 line(s) matched.\n").arg(cnt);
	if( true /*m_grepView == 0*/ ) {
		doOutput(mess);
	} else {
		doOutputToGrepView(mess);
		m_grepView->setModified(false);
		m_grepView->clearLineFlags();
		m_grepView->update();
		m_grepView->setFocus();
		m_grepView = 0;
	}
}

