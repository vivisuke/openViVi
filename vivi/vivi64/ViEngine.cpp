#include "ViEngine.h"
//#include "EditView.h"
#include <QString>
#include <QDebug>


ViEngine::ViEngine(QObject *parent)
	: QObject(parent)
	, m_mode(Mode::INSERT)
	, m_prevMode(Mode::INSERT)
	, m_noInsModeAtImeOpenStatus(false)
	, m_redoRecording(false)
	, m_redoing(false)
	//, m_redoCount(1)
	, m_view(0)
	, m_lastRepeatCount(0)
	, m_vMode(0)
	, m_globalDoing(false)
{
	resetStatus();
}
ViEngine::~ViEngine()
{
}
void ViEngine::resetStatus(bool clearRepCnt)
{
	m_subMode = 0;
	//m_vMode = 0;
	m_redoRecording = false;
	m_redoing = false;
	if( m_undoBlockOpened ) {
		m_cmd = ViCmd::CLOSE_ALL_UNDO_BLOCK;
		emit cmdFixed();
		m_undoBlockOpened = false;
	}
	m_cdy = 0;
	m_yankBufferChar = 0;
	m_cmdText.clear();
	m_cdyRepCount = 1;
	if( clearRepCnt )
		m_repeatCount = 0;
}
void ViEngine::openUndoBlock()
{
	m_cmd = ViCmd::OPEN_UNDO_BLOCK;
	emit cmdFixed();
	m_undoBlockOpened = true;
}
void ViEngine::clearLineNumbers()
{
	m_lns = LineNumbers();
}
void ViEngine::imeOpenStatusChanged()
{
	if( mode() == Mode::COMMAND && !m_noInsModeAtImeOpenStatus )
		setMode(Mode::INSERT);
}
void ViEngine::setMode(byte mode)
{
	if( mode == m_mode ) return;
	m_prevMode = m_mode;
	m_mode = mode;
	m_noInsModeAtImeOpenStatus = true;
	emit modeChanged();
	m_noInsModeAtImeOpenStatus = false;
}
void ViEngine::setPrevMode(byte mode)
{
	m_prevMode = mode;
}
void ViEngine::popMode()			//	モードを元に戻す
{
	if( m_prevMode && m_prevMode != m_mode ) {
		setMode(m_prevMode);
		m_prevMode = 0;
	}
}
void ViEngine::setIncSearchViewPos(EditView *view, pos_t pos)
{
	m_incSearchView = view;
	m_incSearchPos = pos;
}
const QString ViEngine::yankText(bool &bLine) const
{
	const YankBufferItem *ptr = yankBufferItem();
	bLine = ptr->m_bLine;
	return ptr->m_text;
}
void ViEngine::setYankText(const QString &text, bool bLine)
{
	YankBufferItem *ptr = yankBufferItem();
	ptr->m_bLine = bLine;
	ptr->m_text = text;
}
YankBufferItem *ViEngine::yankBufferItem()
{
	//if( !m_yankBufferChar )
	//	return &m_yankBuffer;
	if( m_yankBufferChar >= '0' && m_yankBufferChar <= '9' )
		return &m_namedYB[m_yankBufferChar - '0'];
	if( m_yankBufferChar >= 'a' && m_yankBufferChar <= 'z' )
		return &m_namedYB[m_yankBufferChar - 'a' + 10];
	return &m_yankBuffer;
}
const YankBufferItem *ViEngine::yankBufferItem() const
{
	//if( !m_yankBufferChar )
	//	return &m_yankBuffer;
	if( m_yankBufferChar >= '0' && m_yankBufferChar <= '9' )
		return &m_namedYB[m_yankBufferChar - '0'];
	if( m_yankBufferChar >= 'a' && m_yankBufferChar <= 'z' )
		return &m_namedYB[m_yankBufferChar - 'a' + 10];
	return &m_yankBuffer;
}
void ViEngine::appendInsertedText(const QString &text)
{
	if( m_redoRecording ) {
		//if( text[0] == 'H' )
		//	qDebug() << "appendInsertedText" << text;
		m_insertedText += text;
	}
}
void ViEngine::removeFromInsertedText(const QString &text)
{
	if( m_insertedText.endsWith(text) )
		m_insertedText = m_insertedText.left(m_insertedText.size() - text.size());
}
//	挿入文字列アップデート
void ViEngine::onBackSpace()
{
	//	undone: Ctrl + BackSpace 対応
	if( m_redoRecording && !m_insertedText.isEmpty() )
		m_insertedText = m_insertedText.left(m_insertedText.size() - 1);
}
void ViEngine::doFind(const QString &pat, bool forward)
{
	m_searchPat = pat;
	m_searchFwd = forward;
	m_cmd = ViCmd::SEARCH_PAT;
	emit cmdFixed();
}
void ViEngine::processCommandText(const QString &text, bool hasSelection)
{
	//if( mode() == Mode::INSERT && text.indexOf(0x1b) < 0 ) {
	//	emit insertText(text);
	//} else {
		for (int i = 0; i < text.size(); ++i) {
			if( mode() == Mode::CMDLINE ) {
				emit doExCommand(text.mid(i));
				return;
			}
			processCommand(text[i].unicode(), hasSelection);
		}
	//}
}
void ViEngine::processCommand(wchar_t ch, bool hasSelection)
{
	switch (mode()) {
		case Mode::INSERT:
			if( ch == 0x1b ) {		//	Esc
				int reptCnt = repeatCount();
				if( m_redoRecording && reptCnt > 1 ) {		//	<n>i<text> Esc の場合
					QString text;
					for (int i = 0; i < reptCnt - 1; ++i) {
						text += m_insertedText;
					}
					emit insertText(text);
				}
				if( m_undoBlockOpened ) {
					m_cmd = ViCmd::CLOSE_ALL_UNDO_BLOCK;
					emit cmdFixed();
					m_undoBlockOpened = false;
				}
				resetStatus();
				m_cmd = ViCmd::CUR_LEFT;
				emit cmdFixed();
				setMode(Mode::COMMAND);
			} else {		//	Esc 以外の場合 → 文字挿入
				QString txt = QChar(ch);
				emit insertText(txt);
			}
			break;
		case Mode::REPLACE:
			if( ch == 0x1b ) {		//	Esc
				int reptCnt = repeatCount();
				if( m_redoRecording && reptCnt > 1 ) {
					QString text;
					for (int i = 0; i < reptCnt - 1; ++i) {
						text += m_insertedText;
					}
					emit insertText(text);
				}
				resetStatus();
				setMode(Mode::COMMAND);
			} else {
				QString txt = QChar(ch);
				//if( m_redoRecording )
				//	m_insertedText += txt;
				emit replaceText(txt);
			}
			break;
		case Mode::COMMAND:
			viCommand(ch, hasSelection);
			break;
		case Mode::CMDLINE:
			//qDebug() << "???";
			//setMode(Mode::COMMAND);
			//resetStatus();
			break;
	}
}
void ViEngine::viCommand(wchar_t ch, bool hasSelection)
{
	m_cmd = ViCmd::UNDETERMINED;
	if( m_subMode == 0
		&& (m_repeatCount != 0 && ch == '0'
				|| ch >= '1' && ch <= '9') )
	{
		m_repeatCount = m_repeatCount * 10 + ch - '0';
		return;
	}
	m_cmdText += QChar(ch);
	qDebug() << m_cmdText;
	m_toInsertMode = false;
	m_moved = false;
	m_editCmd = false;			//	.(redo) 対象コマンド
	if( m_subMode != 0 ) {
		processSubMode(ch);
		return;
	}
	switch (ch) {
		case 'c':
			if( m_cdy == 'c' ) {
				m_cmd = ViCmd::DELETE_LINE;
				doCmd();
				m_toInsertMode = true;
				m_cmd = ViCmd::OPEN_PREV_LINE;
				break;
			}
			if( hasSelection ) {
				m_cmd = ViCmd::DELETE_CHAR;
				m_toInsertMode = true;
				break;
			} else {
				//m_subMode = ch;
				m_cdy = ch;
				m_editCmd = true;
				m_cdyRepCount = repeatCount();
				m_repeatCount = 0;
			}
			return;
		case 'd':
			m_editCmd = true;
			if( m_cdy == 'd' ) {
				m_cmd = ViCmd::DELETE_LINE;
				break;
			}
			if( hasSelection ) {
				m_cmd = ViCmd::DELETE_CHAR;
				break;
			} else {
				//m_subMode = ch;
				m_cdy = ch;
				m_cdyRepCount = repeatCount();
				m_repeatCount = 0;
				return;
			}
		case 'y':
			if( m_cdy == 'y' ) {
				m_cmd = ViCmd::YANK_LINE;
				doCmd();
				return;
			}
			if( hasSelection ) {
				m_cmd = ViCmd::YANK_TEXT;
				break;
			}
			//m_subMode = ch;
			m_cdy = ch;
			m_cdyRepCount = repeatCount();
			m_repeatCount = 0;
			return;
		case 'f':
		case 'F':
		case 't':
		case 'T':
		case 'g':
		case 'm':
		case '\'':
		case '`':
		case '"':
		case 'r':
		case 'z':
		case 'Z':
		case '#':
		case '[':
		case ']':
		case '@':
			m_subMode = ch;
			return;
		case '>':
			m_editCmd = true;
			if( hasSelection || m_subMode == '>' ) {
				m_cmd = ViCmd::SHIFT_RIGHT;
				break;
			} else {
				m_subMode = ch;
				m_cdyRepCount = repeatCount();
				m_repeatCount = 0;
				return;
			}
		case '<':
			m_editCmd = true;
			if( hasSelection || m_subMode == '<' ) {
				m_cmd = ViCmd::SHIFT_LEFT;
				break;
			} else {
				m_subMode = ch;
				m_cdyRepCount = repeatCount();
				m_repeatCount = 0;
				return;
			}
		//	挿入モード遷移コマンドたち
		case 'i':
			if( m_vMode == 'v' || cdy() != 0 ) {
				m_subMode = 'i';
				return;
			}
			m_toInsertMode = true;
			break;
		case 'I':
			m_cmd = ViCmd::CUR_FIRST_NOSPACE;
			m_toInsertMode = true;
			break;
		case 'a':
			if( m_vMode == 'v' || cdy() != 0 ) {
				m_subMode = 'a';
				return;
			}
			m_cmd = ViCmd::CUR_RIGHT;
			m_toInsertMode = true;
			break;
		case 'A':
			m_cmd = ViCmd::CUR_END_LINE;
			m_toInsertMode = true;
			break;
		case 'C':
			m_cmd = ViCmd::DELETE_TO_EOL;
			m_toInsertMode = true;
			break;
		case 'o':
			m_toInsertMode = true;
			m_cmd = ViCmd::OPEN_NEXT_LINE;
			break;
		case 'O':
			m_toInsertMode = true;
			m_cmd = ViCmd::OPEN_PREV_LINE;
			break;
		case 's':
			openUndoBlock();
			m_toInsertMode = true;
			m_cmd = ViCmd::DELETE_CHAR;
			break;
		case 'S':
			m_cmd = ViCmd::DELETE_LINE;
			doCmd();
			m_toInsertMode = true;
			m_cmd = ViCmd::OPEN_PREV_LINE;
			break;
		//	上書きモード遷移
		case 'R':
			if( m_redoing ) {
				if( !m_insertedText.isEmpty() ) {
					emit replaceText(m_insertedText);
				}
				resetStatus();
			} else {
				m_redoCmd = "R";
				m_redoRecording = true;
				m_insertedText.clear();
				setMode(Mode::REPLACE);
			}
			return;
		//	編集コマンドたち
		case 'x':
			m_editCmd = true;
			m_cmd = ViCmd::DELETE_CHAR;
			break;
		case 'X':
			m_editCmd = true;
			m_cmd = ViCmd::DELETE_PREV_CHAR;
			break;
		case 'D':
			m_editCmd = true;
			m_cmd = ViCmd::DELETE_TO_EOL;
			break;
		case 'J':
			m_editCmd = true;
			m_cmd = ViCmd::JOIN_LINES;
			break;
		case '~':
			m_editCmd = true;
			m_cmd = ViCmd::TOGGLE_UPPER_LOWER;
			break;
		case 'Y':
			m_cmd = ViCmd::YANK_LINE;
			break;
		case 'p':
			m_editCmd = true;
			m_cmd = ViCmd::PASTE_NEXT;
			break;
		case 'P':
			m_editCmd = true;
			m_cmd = ViCmd::PASTE_PREV;
			break;
		case 'u':
			if( m_cdy != 0 || m_vMode == 'v' ) {
				m_cmd = ViCmd::CUR_CAP_WORD;
				m_moved = true;
			} else
				m_cmd = ViCmd::UNDO;
			break;
		case 'U':
			m_cmd = ViCmd::REDO;
			break;
		case '.':
			if( !m_redoCmd.isEmpty() ) {
				qDebug() << "redoing, m_insertedText = " << m_insertedText;
				//resetStatus();
				if( !m_repeatCount )		//	回数指定が無い場合は以前の回数を参照
					m_repeatCount = m_lastRepeatCount;
				resetStatus(false);		//	false for m_repeatCount をクリアしない
				//m_redoCount = repeatCount();
				m_redoing = true;
				processCommandText(m_redoCmd);
				m_redoing = false;
				if( m_undoBlockOpened ) {
					m_cmd = ViCmd::CLOSE_ALL_UNDO_BLOCK;
					emit cmdFixed();
					m_undoBlockOpened = false;
				}
				//m_redoCount = 1;
				//int cnt = repeatCount();
				//for (int i = 0; i < cnt; ++i) {
				//	processCommand(m_redoCmd);
				//}
			}
			resetStatus();
			return;
		//	カーソル移動コマンドたち
		case 0x08:		//	BackSpace
		case 'h':
			m_cmd = ViCmd::CUR_LEFT;
			m_moved = true;
			break;
		case ' ':
		case 'l':
			m_cmd = ViCmd::CUR_RIGHT;
			m_moved = true;
			break;
		case 'j':
			m_cmd = ViCmd::CUR_DOWN;
			m_moved = true;
			break;
		case 'k':
			m_cmd = ViCmd::CUR_UP;
			m_moved = true;
			break;
		case 'w':
			//if( m_subMode == 'c' )
			//	m_cmd = ViCmd::CUR_END_WORD;		//	cw の場合は単語に続く空白類は削除しない
			//else
				m_cmd = ViCmd::CUR_NEXT_WORD;
			m_moved = true;
			break;
		case 'e':
			m_cmd = ViCmd::CUR_END_WORD;
			m_moved = true;
			break;
		case 'b':
			m_cmd = ViCmd::CUR_PREV_WORD;
			m_moved = true;
			break;
		case 'W':
			//if( m_subMode == 'c' )
			//	m_cmd = ViCmd::CUR_END_SS_WORD;		//	cw の場合は単語に続く空白類は削除しない
			//else
				m_cmd = ViCmd::CUR_NEXT_SS_WORD;
			m_moved = true;
			break;
		case 'E':
			m_cmd = ViCmd::CUR_END_SS_WORD;
			m_moved = true;
			break;
		case 'B':
			m_cmd = ViCmd::CUR_PREV_SS_WORD;
			m_moved = true;
			break;
		case '0':
			m_cmd = ViCmd::CUR_BEG_LINE;
			m_moved = true;
			break;
		case '^':
			m_cmd = ViCmd::CUR_FIRST_NOSPACE;
			m_moved = true;
			break;
		case '$':
			m_cmd = ViCmd::CUR_LAST_CHAR_LINE;
			m_moved = true;
			break;
		case '|':
			m_cmd = ViCmd::CUR_NTH_COLUMN;
			m_moved = true;
			break;
		case ';':
			m_cmd = m_lastFindCmd;
			m_moved = true;
			break;
		case ',':
			switch( m_lastFindCmd ) {
				case ViCmd::SEARCH_CHAR_f:	m_cmd = ViCmd::SEARCH_CHAR_F; break;
				case ViCmd::SEARCH_CHAR_F:	m_cmd = ViCmd::SEARCH_CHAR_f; break;
				case ViCmd::SEARCH_CHAR_t:	m_cmd = ViCmd::SEARCH_CHAR_T; break;
				case ViCmd::SEARCH_CHAR_T:	m_cmd = ViCmd::SEARCH_CHAR_t; break;
			}
			m_moved = true;
			break;
		case '\r':
		case '\n':
		case '+':
			m_cmd = ViCmd::CUR_NEXT_FNS_LINE;
			m_moved = true;
			break;
		case '-':
			m_cmd = ViCmd::CUR_PREV_FNS_LINE;
			m_moved = true;
			break;
		case '{':
			m_cmd = ViCmd::CUR_PREV_BLANK_LINE;
			m_moved = true;
			break;
		case '}':
			m_cmd = ViCmd::CUR_NEXT_BLANK_LINE;
			m_moved = true;
			break;
		case 'H':
			m_cmd = ViCmd::CUR_TOP_OF_SCREEN;
			m_moved = true;
			break;
		case 'M':
			m_cmd = ViCmd::CUR_MIDDLE_OF_SCREEN;
			m_moved = true;
			break;
		case 'L':
			m_cmd = ViCmd::CUR_BOTTOM_OF_SCREEN;
			m_moved = true;
			break;
		case '%':
			m_cmd = ViCmd::CUR_ASSOC_PAREN;
			m_moved = true;
			break;
		case 'G':
			if( !m_repeatCount )
				m_cmd = ViCmd::CUR_END_DOC;
			else
				m_cmd = ViCmd::CUR_JUMP_LINE;
			m_moved = true;
			break;
		case '*':
			m_cmd = ViCmd::SEARCH_CUR_WORD;
			m_cmdLineChar = '/';		//	n が順方向になりますように
			m_moved = true;
			break;
		case 'n':
			if( m_cmdLineChar == '/' )
				m_cmd = ViCmd::SEARCH_NEXT;
			else
				m_cmd = ViCmd::SEARCH_PREV;
			m_moved = true;
			break;
		case 'N':
			if( m_cmdLineChar == '/' )
				m_cmd = ViCmd::SEARCH_PREV;
			else
				m_cmd = ViCmd::SEARCH_NEXT;
			m_moved = true;
			break;
		case '&':
			//processCommand(m_lastSubstCmd);
			emit doExCommand(m_lastSubstCmd);
			break;
		case ':':
			m_cmdLineChar = ch;
			setMode(Mode::CMDLINE);
			resetStatus();
			return;
		case '/':
		case '?':
			m_cmdLineChar = ch;
			setMode(Mode::CMDLINE);
			//resetStatus();
			return;
		case 'v':
			m_vMode = 'v';
			m_cmd = ViCmd::SELECT_CHAR_MODE;
			break;
		case 'V':
			m_vMode = 'V';
			m_cmd = ViCmd::SELECT_LINE;
			m_moved = true;
			break;
		case 0x1b:		//	Esc
			emit clearSelection();
			resetStatus();
			return;
		case '\t':
			m_cmd = ViCmd::TAB_SHIFT;
			break;
		default: {
			//	コマンドエラー
		}
	}
	doCmd();
}
void ViEngine::processSubMode(wchar_t ch)
{
	if (m_subMode == 'r') {
		if (ch != 0x1b) {		//	Esc は置換非対象
			m_cmd = ViCmd::REPLACE_CHAR;		//	1文字置換、置換文字は m_cmdText の最後の文字
			emit cmdFixed();
			if (!m_redoing) {
				m_lastRepeatCount = repeatCount();
				m_redoCmd = QString("r") + QChar(ch);
			}
		}
		resetStatus();
		return;
	}
	if (m_subMode == 'g') {
		switch (ch) {
		case 'g':
			m_cmd = ViCmd::CUR_BEG_DOC;
			m_moved = true;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == '>') {
		switch (ch) {
		case '>':
			m_cmd = ViCmd::SHIFT_RIGHT;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == '<') {
		switch (ch) {
		case '<':
			m_cmd = ViCmd::SHIFT_LEFT;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == 'f' || m_subMode == 'F' || m_subMode == 't' || m_subMode == 'T')
	{
		m_lastFindChar = ch;
		m_cmd = m_subMode == 'f' ? ViCmd::SEARCH_CHAR_f :
			m_subMode == 'F' ? ViCmd::SEARCH_CHAR_F :
			m_subMode == 't' ? ViCmd::SEARCH_CHAR_t : ViCmd::SEARCH_CHAR_T;
		m_lastFindCmd = m_cmd;
		m_moved = true;
		doCmd();
		return;
	}
	if (m_subMode == 'Z') {
		if (ch == 'Z') {
			m_cmd = ViCmd::SAVE_ALL_EXIT;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == 'z') {
		switch (ch) {
		case '\r':
		case '\n':
			m_cmd = ViCmd::SCROLL_CUR_AT_TOP;
			m_moved = true;
			doCmd();
			return;
		case '.':
			m_cmd = ViCmd::SCROLL_CUR_IN_MIDDLE;
			m_moved = true;
			doCmd();
			return;
		case '-':
			m_cmd = ViCmd::SCROLL_CUR_AT_BOTTOM;
			m_moved = true;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == '#') {
		switch (ch) {
		case '#':
		case '+':
			m_cmd = ViCmd::INCREMENT;
			m_editCmd = true;
			doCmd();
			return;
		case '-':
			m_cmd = ViCmd::DECREMENT;
			m_editCmd = true;
			doCmd();
			return;
		case '!':
			m_cmd = ViCmd::TOGGLE_TRUE_FALSE;
			m_editCmd = true;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == '[') {
		switch (ch) {
		case '[':
			m_cmd = ViCmd::BEG_OF_CUR_SECTION;
			m_moved = true;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == ']') {
		switch (ch) {
		case ']':
			m_cmd = ViCmd::BEG_OF_NEXT_SECTION;
			m_moved = true;
			doCmd();
			return;
		}
		//	invalid command
		resetStatus();
		return;
	}
	if (m_subMode == 'm') {
		if (ch >= 'a' && ch <= 'z') {
			m_cmd = ViCmd::MARK;
			doCmd();
		}
		resetStatus();
		return;
	}
	if (m_subMode == '\'') {
		if (ch >= 'a' && ch <= 'z') {
			m_cmd = ViCmd::JUMP_MARK_LINE;
			m_moved = true;
			doCmd();
		}
		resetStatus();
		return;
	}
	if (m_subMode == '`') {
		if (ch >= 'a' && ch <= 'z') {
			m_cmd = ViCmd::JUMP_MARK_POS;
			m_moved = true;
			doCmd();
		}
		resetStatus();
		return;
	}
	if (m_subMode == '"') {
		if (ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z') {
			m_yankBufferChar = ch;
			m_subMode = 0;
			return;
		}
		resetStatus();
		return;
	}
	if (m_subMode == '@') {
		if (ch == '@') {
			m_cmd = ViCmd::EXEC_YANK_TEXT;
			doCmd();
		}
		else if (ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z') {
			m_yankBufferChar = tolower(ch);
			m_cmd = ViCmd::EXEC_YANK_TEXT;
			doCmd();
		}
		resetStatus();
		return;
	}
	if (m_subMode == 'i') {
		if (ch == '"' || ch == '\'' || ch == '(' || ch == '{' || ch == '[' || ch == '<') {
			m_textObjectChar = ch;
			m_cmd = ViCmd::TEXT_OBJECT_I;
			m_moved = true;
			doCmd();
		}
		resetStatus();
		return;
	}
	if (m_subMode == 'a') {
		if (ch == '"' || ch == '\'' || ch == '(' || ch == '{' || ch == '[' || ch == '<') {
			m_textObjectChar = ch;
			m_cmd = ViCmd::TEXT_OBJECT_A;
			m_moved = true;
			doCmd();
		}
		resetStatus();
		return;
	}
	assert(0);
}
void ViEngine::doCmd()
{
	if( m_cmd != ViCmd::UNDETERMINED ) {
		if( m_editCmd && !m_redoing ) {
			m_redoCmd = m_cmdText;
			m_lastRepeatCount = repeatCount();
		}
		//m_lastCmdText = m_cmdText;
		emit cmdFixed();
		if( m_moved ) {
			if( !m_redoing && (m_cdy == 'c' || m_cdy == 'd') ) {
				m_lastRepeatCount = repeatCount();
			}
			m_repeatCount = 0;
			m_cdyRepCount = 1;
		}
		if( m_moved && (m_cdy == 'c' || m_cdy == 'd') ) {
			if( !m_redoing )
				m_redoCmd = m_cmdText;
			if( m_cdy == 'c' )
				openUndoBlock();
			//	(c|d|y|>|<)<move> の場合、移動は KEEP_ANCHOR で行われる
			m_cmd = ViCmd::DELETE_CHAR;
			emit cmdFixed();
			if( m_cdy == 'c' ) {
				m_toInsertMode = true;
				//openUndoBlock();
			}
		} else if( m_moved && m_cdy == 'y' ) {
			//	(c|d|y|>|<)<move> の場合、移動は KEEP_ANCHOR で行われる
			m_cmd = ViCmd::YANK_TEXT;
			emit cmdFixed();
		}
		if( !m_toInsertMode )
 			resetStatus();
	}
	if( m_toInsertMode ) {
		if( !m_redoing ) {
			m_redoCmd = m_cmdText;
			if( !m_cdy )
				m_lastRepeatCount = repeatCount();
			m_redoRecording = true;
			m_insertedText.clear();
			if( lastCmdChar() == 's' || lastCmdChar() == 'S' ) {
				m_repeatCount = 0;		//	数値{s|S} テキスト の場合、数値は削除{文字|行}数
			}
			setMode(Mode::INSERT);
		} else if( !m_insertedText.isEmpty() ) {
			QString text;
			if( lastCmdChar() == 's' || lastCmdChar() == 'S' ) {
				text = m_insertedText;
			} else {
				for (int i = 0; i < repeatCount(); ++i) {
					text += m_insertedText;
				}
			}
			emit insertText(text);
 			resetStatus();
			m_cmd = ViCmd::CUR_LEFT;
			m_repeatCount = 1;
			//m_redoCount = 1;
			emit cmdFixed();
 			resetStatus();
		}
	}
}
