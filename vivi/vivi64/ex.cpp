#include "mainwindow.h"
#include "EditView.h"
#include "CommandLine.h"
#include "ViEngine.h"
#include "AutoCompletionDlg.h"
#include "globalSettings.h"
#include "settingsMgr.h"
#include "typeSettings.h"
//#include "TextCursor.h"
#include "TestViCommands.h"
#include "../buffer/Buffer.h"
#include "../buffer/sssearch.h"

#define		N_CMD_HIST		64

typedef unsigned char byte;

bool isEditView(QWidget *w);

inline bool isAlnum(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && isalnum(v);
}
inline bool isAlpha(wchar_t ch)
{
	return ch < 0x100 && isalpha(ch);
}
inline bool isDigit(wchar_t ch)
{
	return ch < 0x100 && isdigit(ch);
}
QString unEscape(const QString &src);

void MainWindow::on_action_ExCommand_triggered()
{
	//commandLineMode(':');
	m_viEngine->setCmdLineChar(':');
	m_viEngine->setMode(Mode::CMDLINE);
}
void MainWindow::commandLineMode(QChar qch)
{
	if( m_cmdLineEdit == 0 ) {
		m_cmdLineEdit = new CommandLine();
		//connect(m_cmdLineEdit, SIGNAL(focusOut()), this, SLOT(onFocusOutCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onCmdLineTextChanged(const QString &)));
		connect(m_cmdLineEdit, SIGNAL(returnPressed()), this, SLOT(onEnterCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(escPressed()), this, SLOT(onEscCmdLineEdit()));
		//connect(m_cmdLineEdit, SIGNAL(spacePressed()), this, SLOT(onSpaceCmdLineEdit()));
		//connect(m_cmdLineEdit, SIGNAL(slashPressed()), this, SLOT(onSlashCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(colonPressed()), this, SLOT(onColonCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(upPressed()), this, SLOT(onUpCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(downPressed()), this, SLOT(onDownCmdLineEdit()));
		connect(m_cmdLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onEditedCmdLineEdit(QString)));
	} else
		m_cmdLineEdit->setText(QString());
	//if( m_cmdLineEdit->parentWidget() == 0 )
	if (!m_cmdLineEdit->isVisible() )	
	{
#if	1
	m_cmdLineEdit->setParent(statusBar());
	auto g = m_curCharCode->geometry();
	auto rct = statusBar()->rect();
	auto ht = rct.height();
	rct.setRight(g.left());
	const int SPC = 2;
	rct.setLeft(SPC);
	rct.setTop(SPC);
	rct.setBottom(ht - SPC);
	m_cmdLineEdit->setGeometry(rct);
#else
		statusBar()->insertPermanentWidget(0, m_cmdLineEdit);
#endif
		//m_cmdLineEdit->setText(qch);
		m_cmdLineEdit->show();
	}
	m_cmdLineEdit->setText(m_cmdLineText = QString(qch));
	//qDebug() << m_cmdLineEdit->text();
	if( qch == '/' || qch == '?' ) {
		ui.action_RegExp->setChecked(true);		//	正規表現ON
		globSettings()->setBoolValue(GlobalSettings::REGEXP, true);
		EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
		if( view == 0 || !isEditView(view) )
			m_viEngine->setIncSearchViewPos(0, 0);
		else
			m_viEngine->setIncSearchViewPos(view, view->cursorPosition());
	}
	m_cmdLineEdit->setFocus();
	m_exCmdHistIndex = m_exCmdHist.size();
	m_findStringHistIndex = -1;
	m_viEngine->setMode(Mode::CMDLINE);
}
#if	0
void MainWindow::onFocusOutCmdLineEdit()
{
	closeAutoCompletionDlg();
}
#endif
void MainWindow::onEditedCmdLineEdit(QString text)
{
	if( text.isEmpty() ) return;
	m_cmdLineText = m_cmdLineEdit->text();
	if( text[0] == '/' ) {
		//##m_findStringCB->lineEdit()->setText(text.mid(1));
	} else if( text[0] == ':' ) {
		if( m_autoCompletionDlg != 0 ) return;
		if( m_autoCmplDlgClosed ) {
			m_autoCmplDlgClosed = false;
			return;
		}
		QChar ch = text[text.size() - 1];
		//if( ch == '/' || ch == '\\' || ch == ' ' ) return;
		QString arg;
		if( !isEditCommand(arg) ) return;
#if	0
		QDir dir(arg);
		QString dirName = dir.dirName();
		qDebug() << dirName;
		qDebug() << dir.absolutePath();
		dir.cdUp();
		qDebug() << dir.absolutePath();
#endif
		QFileInfo fi(arg);
		QString fn = fi.fileName();
		QDir dir = fi.dir();
		//qDebug() << dir.absolutePath();
		m_autoCmplIndex = m_cmdLineEdit->cursorPosition() - fn.size();
		fileNameCompletion(dir, fn);
	}
}
void MainWindow::hideCmdLineEdit()
{
	m_cmdLineEdit->hide();
}
void MainWindow::onCmdLineTextChanged(const QString &text)
{
	if( text.isEmpty() || (text[0] != '/' && text[0] != '?') ) return;
	auto pat = text.mid(1);
	m_findString = pat;
	EditView *view = currentWidget();
	if (isEditView(view)) {
		if( pat.isEmpty() ) {
			//	undone: インクリメンタルサーチ終了
			view->update();
			return;
		}
		uint opt = getSearchOpt();
		if (!view->findForward(m_findString = pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), false))
			statusBar()->showMessage(tr("'%1' was not found.").arg(pat), 3000);
		else {
			onCursorPosChanged();
		}
	}
}
//	Enter が押された場合の処理
void MainWindow::onEnterCmdLineEdit()
{
	statusBar()->removeWidget(m_cmdLineEdit);
	m_cmdLineEdit->hide();
	QString text = m_cmdLineEdit->text();
	if( text.isEmpty() ) return;
	EditView* view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if (view != 0) {
		view->setFocus();
	}
	if( text[0] == ':' )
		doExCommand(text.mid(1));		//	先頭の:を削除
	else if( view != 0 && (text[0] == '/' || text[0] == '?') ) {
		if( !isEditView(view) ) return;
		doSearchCommand(view, text);
	}
	//if( m_viEngine->cdy() != 0 )
	//	m_viEngine->setMode(Mode::COMMAND);
	//else
		m_viEngine->popMode();			//	モードを元に戻す
}
void MainWindow::doSearchCommand(EditView* view, QString& text)
{
	if (view == m_viEngine->incSearchView())
		view->setCursorPosition(m_viEngine->incSearchPos());
	int first = view->cursorPosition();
	view->doFindText(text);
	//int last = view->cursorPosition();
#if	0
	if (m_viEngine->cdy() == 'c' || m_viEngine->cdy() == 'd') {
		view->setCursorPosition(first, TextCursor::KEEP_ANCHOR);
		m_viEngine->setCmd(ViCmd::DELETE_CHAR);
		view->doViCommand();
	}
#else
	if (m_viEngine->cdy() != 0) {
		byte mode = Mode::COMMAND;
		view->setCursorPosition(first, TextCursor::KEEP_ANCHOR);
		switch (m_viEngine->cdy()) {
		case 'c':
			m_viEngine->setToInsertMode(true);
			mode = Mode::INSERT;
			//qDebug() << "c";
			//break;
		case 'd':
			//qDebug() << "d";
			m_viEngine->setCmd(ViCmd::DELETE_CHAR);
			break;
		case 'y':
			//qDebug() << "y";
			m_viEngine->setCmd(ViCmd::YANK_TEXT);
			break;
		}
		//view->doViCommand();
		m_viEngine->doCmd();
		m_viEngine->setPrevMode(mode);
	}
#endif
	m_viEngine->resetStatus();
}
//	Esc が押された場合の処理
void MainWindow::onEscCmdLineEdit()
{
	statusBar()->removeWidget(m_cmdLineEdit);
	m_cmdLineEdit->hide();
	EditView* view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if (view != 0) {
		view->setFocus();
	}
	m_viEngine->popMode();			//	モードを元に戻す
}
//	ディレクトリ区切り文字が押された場合
//		:e[dit] Space であれば、ファイル名補完ダイアログを表示する
void MainWindow::onSlashCmdLineEdit()
{
	QString arg;
	if( !isEditCommand(arg) ) return;
	fileNameCompletion(QDir(arg));
}
bool MainWindow::isEditCommand(QString &arg)
{
	QString text = m_cmdLineEdit->text();
	if( m_cmdLineEdit->cursorPosition() != text.size() )		//	末尾にカーソルが無い場合
		return false;
	int ix = text.indexOf(' ');
	if( ix < 0 ) return false;
	QString cmd = text.left(ix);
	if( cmd != ":e" && cmd != ":edit" && cmd != ":test" && cmd != ":cd" )
		return false;
	arg = text.mid(ix+1);
	return true;
}
//	Space が押された場合の処理：
//		:e[dit] Space であれば、ファイル名補完ダイアログを表示する
void MainWindow::onSpaceCmdLineEdit()
{
	QString text = m_cmdLineEdit->text();
	if( m_cmdLineEdit->cursorPosition() != text.size() )		//	末尾にカーソルが無い場合
		return;
	if( text != ":e " && text != ":edit " )
		return;
	fileNameCompletion(QDir::current());
}
//	先頭に : が挿入された場合、選択範囲行番号を挿入する
void MainWindow::onColonCmdLineEdit()
{
	EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
	if( !isEditView(view) ) return;
	const QString text = m_cmdLineEdit->text();
	if( m_cmdLineEdit->cursorPosition() != 2 || text.left(2) != "::" )		//	"::" でない場合
		return;
	//const TextCursor *cur = view->textCursor();
	int dln1, dln2;
	if( !view->getSelectedLineRange(dln1, dln2) ) return;
	m_cmdLineEdit->backspace();
	m_cmdLineEdit->insert(QString("%1,%2").arg(dln1+1).arg(dln2+1));
}
void MainWindow::fileNameCompletion(QDir &dir, QString filter)
{
	QStringList lst;
	QStringList elst = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	foreach(const QString fileName, elst) {
		if( filter.isEmpty() || fileName.startsWith(filter, Qt::CaseInsensitive) )
			lst += fileName;
	}
	showAutoCompletionDlg(lst, QString());
	m_autoCmplIndex = m_cmdLineEdit->cursorPosition() - filter.size();
}
void MainWindow::showAutoCompletionDlg(const QStringList &lst, QString ft /*, bool selTail*/)
{
	if( lst.isEmpty()
		|| lst.size() == 1 && lst[0] == ft )	//	候補がひとつで、キー文字列と等しい場合
	{
		return;
	}
	m_autoCmplIndex = m_cmdLineEdit->cursorPosition();
	m_autoCompletionDlg = new AutoCompletionDlg(lst, ft, /*selTail:*/false, this);
	m_autoCompletionDlg->setFilterCaseSensitive(false);		//	大文字小文字同一視
	m_autoCompletionDlg->setResizable(false);		//	リサイズ不可
	connect(m_autoCompletionDlg, SIGNAL(keyPressed(QString)),
					this, SLOT(autoCmplKeyPressed(QString)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(backSpace()),
					this, SLOT(autoCmplBackSpace()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(delPressed(bool, bool)),
					this, SLOT(autoCmplDelete(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(leftPressed(bool, bool)),
					this, SLOT(autoCmplLeft(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(rightPressed(bool, bool)),
					this, SLOT(autoCmplRight(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(zenCoding()),
					this, SLOT(autoCmplZenCoding()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(pasted()),
					this, SLOT(autoCmplPasted()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(decided(QString, bool)),
					this, SLOT(autoCmplDecided(QString, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(rejected()),
					this, SLOT(autoCmplRejected()),
					Qt::QueuedConnection);
	//connect(m_autoCompletionDlg, SIGNAL(focusOut()),
	//				this, SLOT(autoCmplFocusOut()),
	//				Qt::QueuedConnection);
	
	m_autoCompletionDlg->show();

	QPoint clePos = m_cmdLineEdit->pos();
	int x = clePos.x();
	QRect rct = rect();
	const int ht = m_autoCompletionDlg->height();
	int y = rct.height() - ht - statusBar()->height();
	QPoint p(x, y);
	m_autoCompletionDlg->move(mapToGlobal(p));
	
}
void MainWindow::autoCmplKeyPressed(QString text)
{
	m_cmdLineEdit->insert(text);
}
void MainWindow::autoCmplBackSpace()
{
	//m_cmdLineEdit->backspace();
	if( m_cmdLineEdit->cursorPosition() <= m_autoCmplIndex ) {
		closeAutoCompletionDlg();
		m_cmdLineEdit->backspace();
	} else {
		m_cmdLineEdit->backspace();
		QString text = m_cmdLineEdit->text();
		text = text.mid(m_autoCmplIndex, m_cmdLineEdit->cursorPosition() - m_autoCmplIndex);
		m_autoCompletionDlg->setFilterText(text);
	}
}
void MainWindow::autoCmplDelete(bool, bool)
{
}
void MainWindow::autoCmplLeft(bool, bool)
{
	closeAutoCompletionDlg();
}
void MainWindow::autoCmplRight(bool, bool)
{
	closeAutoCompletionDlg();
}
void MainWindow::autoCmplZenCoding()
{
	closeAutoCompletionDlg();
}
void MainWindow::autoCmplPasted()
{
	closeAutoCompletionDlg();
}
void MainWindow::autoCmplDecided(QString text, bool)
{
	closeAutoCompletionDlg();
	//QTextCursor cur = m_cmdLineEdit->textCursor();
	const int d = m_cmdLineEdit->cursorPosition() - m_autoCmplIndex;
	if( d != 0 )
		m_cmdLineEdit->setSelection(m_autoCmplIndex, d);
	m_cmdLineEdit->insert(text);
}
void MainWindow::autoCmplRejected()
{
	closeAutoCompletionDlg();
}
//void MainWindow::autoCmplFocusOut()
//{
//	closeAutoCompletionDlg();
//}
void MainWindow::closeAutoCompletionDlg()
{
	if( m_autoCompletionDlg == 0 )
		return;
	delete m_autoCompletionDlg;
	m_autoCompletionDlg = 0;
	m_autoCmplDlgClosed = true;
}
void MainWindow::onUpCmdLineEdit()
{
	const QString &text = m_cmdLineEdit->text();
	if( text.isEmpty() ) return;
	if( text[0] == ':' ) {
		if( m_exCmdHist.isEmpty() ) return;
		for(;;) {
			if( --m_exCmdHistIndex < 0 ) {
				m_exCmdHistIndex = m_exCmdHist.size();
				m_cmdLineEdit->setText(m_cmdLineText);
				break;
			}
			QString txt = ":" + m_exCmdHist[m_exCmdHistIndex];
			if( txt.startsWith(m_cmdLineText) ) {
				m_cmdLineEdit->setText(txt);
				break;
			}
		}
	} else if( text[0] == '/' || text[0] == '?' ) {
		if( !m_findStringCB->count() ) return;
		if( m_findStringHistIndex >= 0 ) {
			int ix = m_findStringCB->currentIndex();
			if( ++ix >= m_findStringCB->count() ) ix = 0;
			m_findStringCB->setCurrentIndex(ix);
		}
		m_cmdLineEdit->setText(text[0] + m_findStringCB->currentText());
		m_findStringHistIndex = m_findStringCB->currentIndex();
	}
}
void MainWindow::onDownCmdLineEdit()
{
	const QString &text = m_cmdLineEdit->text();
	if( text.isEmpty() ) return;
	if( text[0] == ':' ) {
		if( m_exCmdHist.isEmpty() ) return;
		for(;;) {
			if( ++m_exCmdHistIndex ==  m_exCmdHist.size() ) {
				m_cmdLineEdit->setText(m_cmdLineText);
				break;
			}
			if( m_exCmdHistIndex >  m_exCmdHist.size() )
				m_exCmdHistIndex =  0;
			QString txt = ":" + m_exCmdHist[m_exCmdHistIndex];
			if( txt.startsWith(m_cmdLineText) ) {
				m_cmdLineEdit->setText(txt);
				break;
			}
		}
	} else if( text[0] == '/' || text[0] == '?' ) {
		if( !m_findStringCB->count() ) return;
		if( m_findStringHistIndex >= 0 ) {
			int ix = m_findStringCB->currentIndex();
			if( --ix < 0 ) ix = m_findStringCB->count() -1;
			m_findStringCB->setCurrentIndex(ix);
		}
		m_cmdLineEdit->setText(text[0] + m_findStringCB->currentText());
		m_findStringHistIndex = m_findStringCB->currentIndex();
	}
}
void MainWindow::showCurrentDir()
{
	showMessage(QDir::current().absolutePath(), 5000);
}
enum {
	UNKNOWN = 0,
	OK,
	ERR,
};
bool getNum(EditView *view, LineNumbers &lns, const QString &buf, int &ix, int &num, byte &status)
{
	if( buf[ix] >= '0' && buf[ix] <= '9' ) {
		num = 0;
		while( ix < buf.size() && buf[ix] >= '0' && buf[ix] <= '9' ) {
			num = num * 10 + buf[ix++].unicode() - '0';
		}
		status = OK;
		return true;
	}
	if( buf[ix] == '+' || buf[ix] == '-' || buf[ix] == ',' || buf[ix] == ';' ) {		//	. が省略された場合
		num = lns.m_curLine;
		status = OK;
		return true;
	}
	if( buf[ix] == '.' ) {
		num = lns.m_curLine;
		++ix;
		status = OK;
		return true;
	}
	if( buf[ix] == '$' ) {
		num = lns.m_lineCount;
		++ix;
		status = OK;
		return true;
	}
	if( buf[ix] == '\'' || buf[ix] == '`' ) {
		if( ++ix >= buf.size() || buf[ix] < 'a' && buf[ix] > 'z' ) {
			status = ERR;
			return false;
		}
		pos_t pos = view->buffer()->markPos(buf[ix++].unicode());
		if( pos < 0 ) {
			status = ERR;
			return false;
		}
		num = view->positionToLine(pos) + 1;
		status = OK;
		return true;
	}
	if( buf[ix] == '/' || buf[ix] == '?' ) {
		if( view == 0 ) {
			status = ERR;
			return false;
		}
		QChar sep = buf[ix++];
		QString pat;
		QChar ch;
		while( ix < buf.size() && (ch = buf[ix++]) != sep ) {
			pat += ch;
			if( ch == '\\' && ix < buf.size() )
				pat += buf[ix++];
		}
		SSSearch sssrc;
		pos_t pos;
		if( sep == '/' ) {
			int from = view->lineStartPosition(view->cursorLine() + 1);
			pos = view->buffer()->indexOf(sssrc, (wchar_t *)pat.data(), pat.size(), from, SSSearch::REGEXP);
			if( pos < 0 )
				pos = view->buffer()->indexOf(sssrc, (wchar_t *)pat.data(), pat.size(), 0, SSSearch::REGEXP, from);
		} else {
			int from = view->lineStartPosition(view->cursorLine());
			pos = view->buffer()->rIndexOf(sssrc, (wchar_t *)pat.data(), pat.size(), from, SSSearch::REGEXP);
			if( pos < 0 )
				pos = view->buffer()->rIndexOf(sssrc, (wchar_t *)pat.data(), pat.size(),
																view->bufferSize(), SSSearch::REGEXP, from);
		}
		if( pos < 0 ) {
			status = ERR;
			return false;
		}
		num = view->positionToLine(pos) + 1;
		status = OK;
		return true;
	}
	//status = false;
	return false;
}
bool getOne(EditView *view, LineNumbers &lns, const QString &buf, int &ix, int &num, byte &status)
{
	num = 0;
	if( buf[ix] == '%' ) {
		++ix;
		++lns.m_nLines;
		lns.m_second = 1;
		num = lns.m_lineCount;
		status = OK;
		return true;
	}
	if( getNum(view, lns, buf, ix, num, status) ) {
		status = OK;
		while( ix < buf.size() && (buf[ix] == '+' || buf[ix] == '-') ) {
			QChar op = buf[ix++];
			int num2 = 0;
			if( ix == buf.size() || buf[ix] == ',' || buf[ix] == ';'  )
				num2 = 1;		//	+, - の直後の数字が省略された場合
			else if( !getNum(view, lns, buf, ix, num2, status) ) {
				num = 0;		//	エラー
				break;
			}
			if( op == '+' )
				num += num2;
			else
				num -= num2;
		}
	}
	if( status == OK && (num < 1 || num > lns.m_lineCount) ) {
		status = ERR;
		return false;
	}
	return status == OK;
}
//	行番号部分をパースし、lns に結果を格納
bool parseLineNumbers(EditView *view, LineNumbers &lns, QString &cmd)
{
	byte status = UNKNOWN;
	int ix = 0;
	int num;
	while( ix < cmd.size() && getOne(view, lns, cmd, ix, num, status) ) {
		++lns.m_nLines;
		lns.m_first = lns.m_second;
		lns.m_second = num;
		if( ix >= cmd.size() || cmd[ix] != ',' && cmd[ix] != ';' )
			break;
		if( cmd[ix] != ';' )
			lns.m_curLine = num;
		++ix;
	}
	if( status == ERR ) return false;
	if( !lns.m_nLines )
		lns.m_second = lns.m_curLine;
	if( lns.m_nLines <= 1 )
		lns.m_first = lns.m_second;
	cmd = cmd.mid(ix);
	return true;
}
void MainWindow::appendToExCmdHist(const QString &cmd)
{
	int ix;
	while( (ix = m_exCmdHist.indexOf(cmd)) >= 0 )
		m_exCmdHist.removeAt(ix);		//	重複削除
	m_exCmdHist.push_back(cmd);
	while( m_exCmdHist.size() > N_CMD_HIST )
		m_exCmdHist.pop_front();
	m_exCmdHistIndex = -1;
}
void MainWindow::doExCommand(QString cmd, bool bGlobal)
{
	EditView *view;
	if( !m_viEngine->globDoing() ) {
		view = /*m_testView != 0 ? m_testView :*/ currentWidget();
		if( !isEditView(view) ) view = 0;
		m_viEngine->setView(view);
	} else
		view = m_viEngine->view();
	//	undone: 行番号部分構文解析
	LineNumbers &lns = m_viEngine->lns();
	QString cmd0 = cmd;
	if( !bGlobal ) {
		appendToExCmdHist(cmd);
		lns = LineNumbers();
		if( view != 0 ) {
			lns.m_curLine = view->cursorLine() + 1;
			lns.m_lineCount = view->lineCount();
		}
		if( !parseLineNumbers(view, lns, cmd) ) {		//	行番号部分をパースし、lns に結果を格納
			showMessage(tr("Range Syntax Error, or Out of range."), 5000);
			return;
		}
	}
	if( cmd.isEmpty() ) {
		if( view != 0 && lns.m_first )
			view->jumpToLine(lns.m_second - 1, /*vi:*/true);		//	:1,2 の場合は2行目にジャンプ
		return;
	}
	if( cmd[0] == '!' ) {
		execCommand(cmd.mid(1));
		return;
	}
#if	1
	bool exmark = false;		//	!指定
	int ix = 0;
	while( ix < cmd.size() && (isAlpha(cmd[ix].unicode()) || cmd[ix] == '>'|| cmd[ix] == '<') )
		++ix;		//	コマンドは英字 or < >
	int ix2 = ix;
	if( ix2 < cmd.size() && cmd[ix2] == '!' ) {
		++ix2;
		exmark = true;
	}
	while( ix2 < cmd.size() && cmd[ix2] == ' ' ) ++ix2;
	QString arg = cmd.mid(ix2);
	cmd = cmd.left(ix);
#else
	QString arg;
	int ix = cmd.indexOf(' ');
	if (ix >= 0) {
		arg = cmd.mid(ix+1);
		cmd = cmd.left(ix);
	}
#endif
	if( cmd == "s" || cmd == "subst" || cmd == "substitute" ) {
		m_viEngine->setLastSubstCmd(/*":" +*/ cmd0);
		doSubstitute(view, arg);
		return;
	}
	if( cmd == "d" || cmd == "del" || cmd == "delete" ) {
		m_viEngine->setCmd(ViCmd::EX_DELETE_LINE);
		if( view != 0 )
			view->doViCommand();
		return;
	}
	if( cmd == "mo" || cmd == "mov" || cmd == "move" ) {
		m_viEngine->setCmd(ViCmd::EX_MOVE_LINE);
		doMoveCopyCommand(view, arg);
		return;
	}
	if( cmd == "co" || cmd == "cop" || cmd == "copy" ) {
		m_viEngine->setCmd(ViCmd::EX_COPY_LINE);
		doMoveCopyCommand(view, arg);
		return;
	}
	if( cmd == ">" ) {
		if( view != 0 ) {
			m_viEngine->setCmd(ViCmd::EX_SHIFT_RIGHT);
			view->doViCommand();
		}
		return;
	}
	if( cmd == "<" ) {
		if( view != 0 ) {
			m_viEngine->setCmd(ViCmd::EX_SHIFT_LEFT);
			view->doViCommand();
		}
		return;
	}
	if( cmd == "p" ) {
		if( view != 0 ) {
			for (int i = lns.m_first; i <= lns.m_second; ++i) {
				QString text = view->getLineText(i-1);
				doOutput(text);
			}
		}
		return;
	}
	if( cmd == "P" ) {
		if( view != 0 ) {
			if( !bGlobal || !m_viEngine->isPathNamePrinted() ) {
				QString title = view->fullPathName();
				if( title.isEmpty() )
					title = view->title();
				doOutput("\"" + title + "\":\n");
				m_viEngine->setPathNamePrinted(true);
			}
			for (int i = lns.m_first; i <= lns.m_second; ++i) {
				QString text = view->getLineText(i-1);
				doOutput(QString(" %1: ").arg(i) + text);
			}
		}
		return;
	}
	if( cmd == "g" || cmd == "v" ) {
		if( view == 0 ) return;
		bool isGlobal = cmd == "g";
		if( arg.isEmpty() ) {
			showMessage(tr("Syntax Error. usage: g[lobal]/pat/command"));
			return;
		}
		QChar sep = arg[0];
		int ix = 1;
		while( ix < arg.size() ) {
			QChar ch = arg[ix++];
			if( ch == sep ) break;
			if( ch == '\\' && ix < arg.size() )
				++ix;
		}
		QString pat = arg.mid(1, ix - 2);
		arg = arg.mid(ix);
		view->buffer()->setGlobalFlag((wchar_t *)pat.data(), pat.size());
		m_viEngine->setPathNamePrinted(false);
		view->openUndoBlock();
		m_viEngine->setUndoBlockOpened(true);
		int first, last;
		if( lns.m_nLines != 0 ) {
			first = lns.m_first - 1;
			last = qMin(lns.m_second, view->lineCount());
		} else {
			first = 0;
			last = view->lineCount();
		}
		if( arg == "d" ) {
			for (int dln = last; --dln >= first; ) {
				if( isGlobal && (view->lineFlags(dln) & Buffer::LINEFLAG_GLOBAL) != 0
					|| !isGlobal && (view->lineFlags(dln) & Buffer::LINEFLAG_GLOBAL) == 0)
				{
					pos_t ls = view->lineStartPosition(dln);
					pos_t nxls = view->lineStartPosition(dln+1);
					view->deleteText(ls, nxls - ls);
				}
			}
		} else {
			m_viEngine->setGlobDoing(true);
			//int lineCount = view->lineCount();
			for (int ln = first; ln < last; ++ln) {
				if( isGlobal && (view->lineFlags(ln) & Buffer::LINEFLAG_GLOBAL) != 0
					|| !isGlobal && (view->lineFlags(ln) & Buffer::LINEFLAG_GLOBAL) == 0)
				{
					lns.m_first = lns.m_second = ln + 1;
					doExCommand(arg, /*global:*/true);
					//if( view->lineCount() < lineCount ) {		//	行削除された場合への対応
					//	ln -= lineCount - view->lineCount();
					//	lineCount = view->lineCount();
					//}
				}
			}
			m_viEngine->setGlobDoing(false);
		}
		m_viEngine->setUndoBlockOpened(false);
		view->closeUndoBlock();
		return;
	}
	if( cmd == "type" && arg[0] == '=' ) {
		onTypeChanged(arg.mid(1));
		return;
	}
	if( cmd == "font" && arg[0] == '=' ) {
		arg = arg.mid(1);
		EditView *view = /*m_testView != 0 ? m_testView :*/ currentWidget();
		if( !isEditView(view) || arg.isEmpty() ) return;
		TypeSettings *typeSettings = view->typeSettings();
		typeSettings->setTextValue(TypeSettings::FONT_NAME, arg);
		//setTypeSettings(view, typeSettings);
		view->updateFont();
		view->update();
		return;
	}
	if( cmd == "e" || cmd == "edit" ) {
		if( !exmark )
			view = openFile(arg /*, true*/);
		else if( view != 0 && arg.isEmpty() ) {
			view->setModified(false);
			reloadRequested(view);
		}
		return;
	}
	//if( view != 0 && arg.isEmpty() && (cmd == "e!" || cmd == "edit!") ) {
	//	view->setModified(false);
	//	reloadRequested(view);
	//	return;
	//}
	if( cmd == "w" || cmd == "write" ) {
		if( !arg.isEmpty() ) {
		} else {
			if( view != 0 )
				view->saveFile();
		}
	}
	if( cmd == "wq" || cmd == "writequit" || cmd == "x" || cmd == "exit" ) {
		if( arg.isEmpty() ) {
			if( view != 0 )
				view->saveFile();
			tabCloseRequested(ui.tabWidget->currentIndex());
		} else
			showMessage(tr("illegal argument."), 5000);
		return;
	}
	if( cmd == "q" || cmd == "quit" ) {
		if( !exmark ) {
			if( arg.isEmpty() )
				tabCloseRequested(ui.tabWidget->currentIndex());
			else
				showMessage(tr("illegal argument."), 5000);
		} else {
			if( view != 0 )
				view->setModified(false);
			tabCloseRequested(ui.tabWidget->currentIndex());
		}
		return;
	}
	//if( cmd == "q!" || cmd == "quit!" ) {
	//	if( view != 0 )
	//		view->setModified(false);
	//	tabCloseRequested(ui.tabWidget->currentIndex());
	//	return;
	//}
	if( cmd == "n" || cmd == "ne" || cmd == "next" ) {
		on_action_NextTab_triggered();
		return;
	}
	if( cmd == "pr" || cmd == "prev" ) {
		on_action_PrevTab_triggered();
		return;
	}
	if( cmd == "cd" ) {
		if (QDir::setCurrent(arg)) {
			showCurrentDir();
		} else {
			QString mess = tr("%1 is not proper directatory\n").arg(arg);
			showMessage(mess, 5000);
		}
		return;
	}
	if( cmd == "pwd" ) {
		QString cur = QDir::currentPath();
		showMessage(cur, 5000);
		return;
	}
	if( cmd == "zc" ) {
		on_action_ZenCoding_triggered();
		return;
	}
	if( cmd == "test" /*&& !arg.isEmpty()*/ ) {
		if( arg.isEmpty() )
			arg = "c:/vivi/ssetest.txt";
		testViCommands(this, m_viEngine, arg);
		return;
	}
	showMessage(tr("illegal commad:") + cmd, 5000);
}
void MainWindow::doMoveCopyCommand(EditView *view, QString &arg)
{
	if( arg.isEmpty() || !isDigit(arg[0].unicode()) ) {
		showMessage(tr("Syntax Error. usega: {range}mo[ve]{dest}"), 5000);
		return;
	}
	int dest = arg.toInt();
	LineNumbers &lns = m_viEngine->lns();
	if( m_viEngine->cmd() == ViCmd::EX_MOVE_LINE
		&& dest >= lns.m_first && dest < lns.m_second )
	{
		showMessage(tr("illegal linenumbers."), 5000);
		return;
	}
	if( dest > lns.m_lineCount ) {
		showMessage(tr("dest line-number is Out of Range."), 5000);
		return;
	}
	lns.m_dest = dest;
	if( view != 0 )
		view->doViCommand();
	return;
}
//
bool MainWindow::parseSubstisute(const QString &arg, QString &pat, QString &rep, QString &opt)
{
	if( arg.isEmpty() ) return false;
	QChar sep = arg[0];
	int ix = 1;
	for (;;) {
		if( ix == arg.size() ) return true;
		QChar ch = arg[ix++];
		if( ch == sep ) break;
		pat += ch;
		if( ch == '\\' && ix < arg.size() )
			pat += arg[ix++];
	}
	while( ix <arg.size() ) {
		QChar ch = arg[ix++];
		if( ch == sep ) break;
		rep += ch;
		if( ch == '\\' && ix < arg.size() )
			rep += arg[ix++];
	}
	//rep = unEscape(rep);
	opt = arg.mid(ix);
	return true;
}
void MainWindow::doSubstitute(EditView *view, const QString &arg)
{
	//m_viEngine->setLastSubstCmd(m_cmdLineEdit->text());
	if( view == 0 ) return;
	QString pat, rep, opt;
	if( !parseSubstisute(arg, pat, rep, opt) ) {
		showMessage(tr("Syntax Error. usage: {range}s/pat/rep/opt"), 5000);
		return;
	}
	if( pat.isEmpty() )
		pat = m_findString;
	LineNumbers &lns = m_viEngine->lns();
	view->substitute(lns.m_first - 1, lns.m_second - 1, pat, rep, opt);
}
