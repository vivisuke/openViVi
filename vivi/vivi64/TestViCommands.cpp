//----------------------------------------------------------------------
//
//			File:			"TestViCommands.cpp"
//			Created:		24-Feb-2011
//			Author:			Nobuhide Tsuda
//			Description:	自動テスト関数実装
//
//----------------------------------------------------------------------

/*

	Copyright (C) 2011 by Nobuhide Tsuda

*/

#include <QtGui>
#include "TestViCommands.h"
#include "mainwindow.h"
#include "ViEngine.h"
#include "EditView.h"
#include "charEncoding.h"
#include "typeSettings.h"

#if 0
void readToBuffer(QTextStream &in, ViEngine *viEngine, QString &buffer);
void checkBuffer(QTextStream &in, MainWindow *mainWindow, ViEngine *viEngine, QString &buffer);
#endif

struct TestFailedItem {
	QString	m_path;
	QString	m_message;
	int		m_lineNum;
public:
	TestFailedItem() : m_lineNum(0) {}
	TestFailedItem(const QString &path, int lineNum, const QString &messsage = QString())
		: m_path(path), m_lineNum(lineNum), m_message(messsage)
		{}
};

class TestViCommands
{
public:
	TestViCommands(MainWindow *mainWindow, ViEngine *viEngine, const QString &testFileName);

public:
	void	doTest();

protected:
	void	readToBuffer(QTextStream &in);
	bool	checkBuffer(QTextStream &in);
	void	checkCursor();

	void	skipBlank();
	int		getNumber(/*int = 0*/);
	QString	getDQString();

private:
	MainWindow	*m_mainWindow;
	ViEngine	*m_viEngine;
	const QString &m_testFileName;
	int			m_lineNum;
	QString		m_buffer;
	int			m_ix;
	int			m_nTested;
	int			m_nFailed;
	QVector<TestFailedItem>	m_failedPos;
};

TestViCommands::TestViCommands(MainWindow *mainWindow, ViEngine *viEngine, const QString &testFileName)
	: m_mainWindow(mainWindow), m_viEngine(viEngine), m_testFileName(testFileName)
{
}
void TestViCommands::doTest()
{
#if	0	//##
	if( m_testFileName.isEmpty() ) {
		m_mainWindow->doOutput("need fileName.\n");
		m_mainWindow->doOutput("usage: test <testFileName>\n");
		return;
	}
	EditView *view = m_mainWindow->testView();
	const QString fontName = view->typeSettings()->textValue(TypeSettings::FONT_NAME);
	//if( view == 0 ) {
	//	m_mainWindow->addNewView(0, "Test");
	//	return;
	//}
#if 0		//##
	QString buffer;
	QString errorString;
	if( !::loadFile(m_testFileName, buffer, errorString) ) {
		m_mainWindow->doOutput(QObject::tr("can't open '%1'.\n").arg(m_testFileName));
		return;
	}
	m_mainWindow->doOutput(QObject::tr("testing '%1' (Qt verdion %2)\n")
							.arg(m_testFileName).arg(QT_VERSION_STR));
	QTextStream in(&buffer);
#else
	QFile f(m_testFileName);
	if( !f.open(QIODevice::ReadOnly) ) {
		m_mainWindow->doOutput(QObject::tr("can't open '%1'.\n").arg(m_testFileName));
		return;
	}
	QTextStream in(&f);
#endif
	m_mainWindow->setTesting(true);
	m_viEngine->setMode(Mode::COMMAND);
	m_nTested = 0;
	m_nFailed = 0;
	m_lineNum = 0;
	bool skipReadLine = false;
	while( !in.atEnd() ) {
		if( !skipReadLine ) {
			m_buffer = in.readLine();
			++m_lineNum;
		} else
			skipReadLine = false;
		if( m_buffer.isEmpty() ) continue;
		if( m_buffer[0] == '<' ) {
			readToBuffer(in);
			skipReadLine = true;
		} else if( m_buffer[0] == '>' ) {
			if( checkBuffer(in) )
				skipReadLine = true;
		} else if( m_buffer[0] == '/' || m_buffer[0] == '?' ) {
			m_viEngine->doFind(m_buffer.mid(1), m_buffer[0] == '/');
		//} else if( m_buffer[0] == ':' ) {
		//	m_viEngine->doExCommand(m_buffer.mid(1));
		} else {
			m_ix = 0;
			while( m_ix < m_buffer.length() ) {
				QChar ch = m_buffer[m_ix++];
				if( ch == ';' || ch == '#' || ch == '/' )	//	行コメント
					break;
				if( ch == ' ' || ch == '\t' )
					continue;
				if( ch == '=' )
					checkCursor();
				else if( ch == '"' ) {
					QString cmd = getDQString();
					m_viEngine->doViCommand(cmd);
				} else if( ch.isDigit() ) {
					--m_ix;
					QChar cmd(getNumber(/*ch.unicode() - '0'*/));
					m_viEngine->doViCommand(cmd.unicode());
				} else if( ch == ':' ) {
					m_mainWindow->doExCommand(m_buffer.mid(m_ix));
					break;
				} else if( ch == L'←' ) {
					m_mainWindow->doCurLeft();
				} else if( ch == L'→' ) {
					m_mainWindow->doCurRight();
				} else if( ch == L'↑' ) {
					m_mainWindow->doCurUp();
				} else if( ch == L'↓' ) {
					m_mainWindow->doCurDown();
				}
			}
		}
	}
	qDebug() << "test finished.";
	m_mainWindow->doOutput("\n");
	m_mainWindow->doOutput(QObject::tr("%1 failed / %2 tested.\n").arg(m_nFailed).arg(m_nTested));

	for(QVector<TestFailedItem>::const_iterator itr = m_failedPos.begin(), iend = m_failedPos.end();
		itr != iend; ++itr)
	{
		m_mainWindow->doOutput(QObject::tr("\"%1\"(%2):%3\n")
								.arg(itr->m_path).arg(itr->m_lineNum).arg(itr->m_message));
	}
	m_viEngine->setMode(Mode::COMMAND);
	m_viEngine->setPrevMode(Mode::COMMAND);		//	pop で元に戻りますように
	m_mainWindow->setTesting(false);
	view->typeSettings()->setTextValue(TypeSettings::FONT_NAME, fontName);
#endif
}
void TestViCommands::skipBlank()
{
	while( m_ix < m_buffer.length() && (m_buffer[m_ix] == ' ' ||  m_buffer[m_ix] == '\t') )
		++m_ix;
}
int TestViCommands::getNumber(/*int value*/)
{
	int value = 0;
	skipBlank();
	if( m_ix + 1 < m_buffer.length() && m_buffer[m_ix] == '0' &&
		m_buffer[m_ix+1].toLower() == 'x' )
	{
		m_ix += 2;
		int t;
		while( m_ix < m_buffer.length() ) {
			QChar ch = m_buffer[m_ix];
			if( ch.isDigit() )
				t = ch.unicode() - '0';
			else {
				ch = ch.toLower();
				if( ch < 'a' || ch > 'f' )
					break;
				t = ch.unicode() - 'a' + 10;
			}
			value = value * 16 + t;
			++m_ix;
		}
	} else {
		while( m_ix < m_buffer.length() && m_buffer[m_ix].isDigit() )
			value = value * 10 + m_buffer[m_ix++].unicode() - '0';
	}
	skipBlank();
	return value;
}
QString TestViCommands::getDQString()
{
	QString text;
	while( m_ix < m_buffer.length() ) {
		QChar ch = m_buffer[m_ix++];
		if( ch == '"' ) {
			if( m_ix < m_buffer.length() && m_buffer[m_ix] == '"' )
				++m_ix;
			else
				break;
		}
		text += ch;
	}
	return text;
}
void TestViCommands::checkCursor()
{
#if	0		//##
	int line = getNumber();
	int offset = getNumber();
	EditView *view = m_mainWindow->testView();
	if( view == 0 ) return;
	pos_t pos = view->cursorPosition();
	int dln = view->cursorLine();
	pos_t ls = view->lineStartPosition(dln);
	if( line == dln + 1 && offset == pos - ls )
		m_mainWindow->doOutput(".");
	else {
		qDebug() << "failed at checkCursor()";
		m_mainWindow->doOutput("F");
		const QString mess = QObject::tr("cursor pos %1 %2 expected, but %3 %4 actually.")
						.arg(line).arg(offset)
						.arg(dln + 1).arg(pos - ls);
		m_failedPos.push_back(TestFailedItem(m_testFileName, m_lineNum, mess));
				
		++m_nFailed;
	}
	++m_nTested;
#endif
}
void TestViCommands::readToBuffer(QTextStream &in)
{
#if	0		//##
	EditView *view = m_mainWindow->testView();
	if( view == 0 ) return;
	view->deleteText(0, view->bufferSize());
	for(;;) {
		view->insertText(m_buffer.mid(1));
		view->insertText("\n");
		if( in.atEnd() ) {
			m_buffer.clear();
			break;
		}
		m_buffer = in.readLine();
		++m_lineNum;
		if( m_buffer.isEmpty() || m_buffer[0] != '<' ) {
			//cur.movePosition(QTextCursor::Start);
			//editor->setTextCursor(cur);
			view->setCursorPosition(0);
			view->clearUndoMgr();
			return;
		}
	}
#endif
}
bool TestViCommands::checkBuffer(QTextStream &in)
{
#if	0	//##
	EditView *view = m_mainWindow->testView();
	if( view == 0 ) return false;
	bool rc = true;
	int dln = 0;
	while( !m_buffer.isEmpty() && m_buffer[0] == '>' ) {
		if( dln >= view->lineCount() ) {
			return false;
		}
		QString txt = view->getLineText(dln).remove(QRegExp("\\s+$"));;
		if( txt == m_buffer.mid(1) )
			m_mainWindow->doOutput(".");
		else {
			m_mainWindow->doOutput("F");
			QString mess = QObject::tr("'%1' expected, but '%2'.").arg(m_buffer.mid(1)).arg(txt);
			m_failedPos.push_back(TestFailedItem(m_testFileName, m_lineNum, mess));
			++m_nFailed;
		}
		++m_nTested;
		++dln;
		if( in.atEnd() ) {
			m_buffer.clear();
			break;
		}
		m_buffer = in.readLine();
		++m_lineNum;
	}
	while( dln < view->lineCount() ) {
		m_mainWindow->doOutput("F");
		QString mess = QObject::tr("'%1' is NOT expected.").arg(view->getLineText(dln));
		m_failedPos.push_back(TestFailedItem(m_testFileName, m_lineNum, mess));
		++m_nFailed;
		++m_nTested;
		++dln;
	}
#endif
	return true;
}

void testViCommands(MainWindow *mainWindow, ViEngine *viEngine, const QString &fileName)
{
	TestViCommands tvc(mainWindow, viEngine, fileName);
	tvc.doTest();
}


#if 0
void testViCommands(MainWindow *mainWindow, ViEngine *viEngine, const QString &fileName)
{
	if( fileName.isEmpty() ) {
		mainWindow->doOutput("need fileName.\n");
		mainWindow->doOutput("usage: test <testFileName>\n");
		return;
	}
	QFile f(fileName);
	if( !f.open(QIODevice::ReadOnly) ) {
		mainWindow->doOutput(QObject::tr("can't open '%1'.\n").arg(fileName));
		return;
	}
	QTextStream in(&f);
	QString buffer;
	bool skipReadLine = false;
	while( !in.atEnd() ) {
		if( !skipReadLine )
			buffer = in.readLine();
		else
			skipReadLine = false;
		//mainWindow->doOutput(buffer);
		//mainWindow->doOutput("\n");
		if( buffer.isEmpty() ) continue;
		if( buffer[0] == '<' ) {
			readToBuffer(in, viEngine, buffer);
			skipReadLine = true;
		} else if( buffer[0] == '>' ) {
			checkBuffer(in, mainWindow, viEngine, buffer);
			skipReadLine = true;
		}
	}
}
void readToBuffer(QTextStream &in, ViEngine *viEngine, QString &buffer)
{
#if	1
	EditView *view = viEngine->testView();
	if( view == 0 ) return;
	//view ->document()->clear();
	//view->setCursorPosition(0);
	view->deleteText(0, view->bufferSize());
	for(;;) {
		view->insertText(buffer.mid(1));
		view->insertText("\n");
		if( in.atEnd() ) {
			buffer.clear();
			break;
		}
		buffer = in.readLine();
		if( buffer.isEmpty() || buffer[0] != '<' ) {
			//cur.movePosition(QTextCursor::Start);
			//view->setTextCursor(cur);
			view->setCursorPosition(0);
			return;
		}
	}
#endif
}
void checkBuffer(QTextStream &in, MainWindow *mainWindow, ViEngine *viEngine, QString &buffer)
{
#if	0
	EditView *editor = viEngine->editor();
	QTextBlock block = editor->document()->firstBlock();
	while( !buffer.isEmpty() && buffer[0] == '>' ) {
		if( block.isValid() && block.text() == buffer.mid(1) )
			mainWindow->doOutput(".");
		else
			mainWindow->doOutput("F");
		if( block.isValid() )
			block = block.next();
		if( in.atEnd() ) {
			buffer.clear();
			break;
		}
		buffer = in.readLine();
	}
	while( block.isValid() ) {
		mainWindow->doOutput("F");
		block = block.next();
	}
#endif
}
#endif

#if 0
TestViCommands::TestViCommands(QObject *parent)
	: QObject(parent)
{

}

TestViCommands::~TestViCommands()
{

}
#endif
