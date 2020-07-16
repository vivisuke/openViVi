#include<QtGui>
#include <QStringList>
#include <QDir>

#include "EditView.h"
#include "Document.h"
#include "TextCursor.h"
#include "mainwindow.h"
#include "globalSettings.h"
#include "typeSettings.h"
#include "ViEngine.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"
#include "../buffer/MarkMgr.h"
#include "../buffer/sssearch.h"

inline bool isNewLineChar(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
//----------------------------------------------------------------------
void EditView::doFindText(const QString &text0)
{
	if( text0.isEmpty() ) return;
	bool forward = text0[0] == '/';
	QString pat = text0.mid(1);
	doFindText(pat, forward);
}
void EditView::doFindText(const QString &pat, bool forward)		//	/ ? 検索処理
{
	//	undone:		/text/opt 対応
	//	undone:		//	対応
	uint opt = SSSearch::REGEXP;
	if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) ) opt |= SSSearch::IGNORE_CASE;
	//if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) ) opt |= SSSearch::WHOLE_WORD_ONLY;
	bool rc = forward ?
						findForward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), true, /*vi:*/true)
						: findBackward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), /*vi:*/true);
	if( !rc )
		showMessage(tr("'%1' was not found.").arg(pat));
	else {
		mainWindow()->setFindString(pat);
	}
	setFocus();
	update();
}
void EditView::viFindCharForward(wchar_t qch, bool bPrev, int mvmd, int repCnt)
{
	int dln = viewLineToDocLine(m_textCursor->viewLine());
	pos_t last = lineStartPosition(dln+1);
	pos_t pos = m_textCursor->position();
	while( ++pos < last ) {
		if( charAt(pos) == qch && --repCnt == 0 ) {
			if( bPrev ) --pos;
			if( mvmd == TextCursor::KEEP_ANCHOR ) ++pos;
			m_textCursor->setPosition(pos, mvmd);
			return;
		}
	}
}
void EditView::viFindCharBackward(wchar_t qch, bool bPrev, int mvmd, int repCnt)
{
	int dln = viewLineToDocLine(m_textCursor->viewLine());
	pos_t first = lineStartPosition(dln);
	pos_t pos = m_textCursor->position();
	while( --pos >= first ) {
		if( charAt(pos) == qch && --repCnt == 0 ) {
			if( bPrev ) ++pos;
			m_textCursor->setPosition(pos, mvmd);
			return;
		}
	}
}
#if	1
void EditView::findNext(const QString &pat, bool vi)
{
	auto opt = mainWindow()->getSearchOpt(vi);
	//uint opt = 0;
	//if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) ) opt |= SSSearch::IGNORE_CASE;
	//if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) ) opt |= SSSearch::WHOLE_WORD_ONLY;
	QTime tm;
	tm.start();
	bool rc = findForward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), true, vi);
	int ms = tm.elapsed();
	if( !rc )
		showMessage(tr("'%1' was not found (%2 msec).").arg(pat).arg(ms), 3000);
	else {
		showMessage(tr("(%1 msec).").arg(ms), 3000);
		mainWindow()->setFindString(pat);
	}
	setFocus();
	update();
}
void EditView::findPrev(const QString &pat, bool vi)
{
	auto opt = mainWindow()->getSearchOpt(vi);
	//uint opt = 0;
	//if( globSettings()->boolValue(GlobalSettings::IGNORE_CASE) ) opt |= SSSearch::IGNORE_CASE;
	//if( globSettings()->boolValue(GlobalSettings::WHOLE_WORD_ONLY) ) opt |= SSSearch::WHOLE_WORD_ONLY;
	if( !findBackward(pat, opt, globSettings()->boolValue(GlobalSettings::LOOP_SEARCH), vi) )
		showMessage(tr("'%1' was not found.").arg(pat), 3000);
	else
		mainWindow()->setFindString(pat);
	setFocus();
	update();
}
#endif
void EditView::curMove(int mv, int n, bool vi)
{
	m_textCursor->movePosition(mv, TextCursor::MOVE_ANCHOR, n, vi);
	makeCursorInView();
	update();
}
void EditView::viYankText()
{
	if( m_textCursor->hasSelection() ) {
		const QString txt = m_textCursor->selectedText();
		mainWindow()->viEngine()->setYankText(txt, false);
		//m_textCursor->clearSelection();
		m_textCursor->setPosition(m_textCursor->selectionFirst());
		showMessage(tr("%1 characters are yanked.").arg(txt.size()));
	}
}
void EditView::viYankLine()
{
	TextCursor cur(*m_textCursor);
	int repCnt = mainWindow()->viEngine()->repeatCount();
	if( !cur.hasSelection() ) {
		cur.movePosition(TextCursor::BEG_LINE);
		cur.movePosition(TextCursor::DOWN, TextCursor::KEEP_ANCHOR, repCnt);
		if( !cur.hasSelection() ) 
			cur.movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
	}
	if( !cur.hasSelection() ) return;
	QString txt = cur.selectedText();
	if( !txt.isEmpty() && txt.back() != '\n' && txt.back() != '\r' )
		txt += mainWindow()->newLineText();		
	mainWindow()->viEngine()->setYankText(txt, true);
	showMessage(tr("%1 lines, %2 characters are yanked.").arg(repCnt).arg(txt.size()));
}
void EditView::viPastePrev()
{
	bool bLine;		//	行単位ペースト
	const QString &txt = mainWindow()->viEngine()->yankText(bLine);
	int repCnt = mainWindow()->viEngine()->repeatCount();
	QString text;
	for (int i = 0; i < repCnt; ++i) {
		text += txt;
	}
	if( text.isEmpty() ) return;
	if( !bLine ) {
		insertText(text);
		m_textCursor->movePosition(TextCursor::LEFT);
	} else {
		//openPrevLine();
		m_textCursor->movePosition(TextCursor::BEG_LINE);
		pos_t pos = m_textCursor->position();
		insertText(text);
		m_textCursor->setPosition(pos);
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	}
}
void EditView::viPasteNext()
{
	bool bLine;		//	行単位ペースト
	const QString &txt = mainWindow()->viEngine()->yankText(bLine);
	int repCnt = mainWindow()->viEngine()->repeatCount();
	QString text;
	for (int i = 0; i < repCnt; ++i) {
		text += txt;
	}
	if( text.isEmpty() ) return;
	if( !bLine ) {
		if( !isNewLineChar(charAt(m_textCursor->position())) )
			m_textCursor->movePosition(TextCursor::RIGHT);
		insertText(text);
		m_textCursor->movePosition(TextCursor::LEFT);
	} else {
		//openNextLine();
		m_textCursor->movePosition(TextCursor::BEG_LINE);
		m_textCursor->movePosition(TextCursor::DOWN);
		pos_t pos = m_textCursor->position();
		insertText(text);
		m_textCursor->setPosition(pos);
		m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
	}
}
void EditView::doViCommand()		//	viEngine が保有する vi コマンドを実行
{
	ViEngine *viEngine = mainWindow()->viEngine();
	int repCnt = viEngine->repeatCount();
	int mvmd = !viEngine->cdy()
						? TextCursor::MOVE_ANCHOR : TextCursor::KEEP_ANCHOR;
	const QString &cmdText = viEngine->cmdText();
	const LineNumbers &lns = viEngine->lns();
	switch (viEngine->cmd()) {
		case ViCmd::CUR_LEFT:
			m_textCursor->movePosition(TextCursor::LEFT, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_RIGHT:
			m_textCursor->movePosition(TextCursor::RIGHT, mvmd, repCnt, cmdText[cmdText.size()-1] != 'a');
			break;
		case ViCmd::CUR_UP:
			m_textCursor->movePosition(TextCursor::UP, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_DOWN:
			m_textCursor->movePosition(TextCursor::DOWN, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_CAP_WORD:
			m_textCursor->movePosition(TextCursor::NEXT_CAP_WORD, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_NEXT_WORD:
			if( viEngine->cdy() != 'c' )
				m_textCursor->movePosition(TextCursor::NEXT_WORD, mvmd, repCnt, true);
			else
				m_textCursor->movePosition(TextCursor::NEXT_WORD_NOSKIPSPC, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_END_WORD:
			m_textCursor->movePosition(TextCursor::END_WORD, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_PREV_WORD:
			m_textCursor->movePosition(TextCursor::PREV_WORD, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_BEG_LINE:
			m_textCursor->movePosition(TextCursor::BEG_LINE, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_NEXT_SS_WORD:
			if( viEngine->subMode() != 'c' )
				m_textCursor->movePosition(TextCursor::NEXT_SS_WORD, mvmd, repCnt, true);
			else
				m_textCursor->movePosition(TextCursor::NEXT_SS_WORD_NOSKIPSPC, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_END_SS_WORD:
			m_textCursor->movePosition(TextCursor::END_SS_WORD, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_PREV_SS_WORD:
			m_textCursor->movePosition(TextCursor::PREV_SS_WORD, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_FIRST_NOSPACE:
			m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mvmd, repCnt, true);
			break;
		case ViCmd::CUR_END_LINE:
			m_textCursor->movePosition(TextCursor::END_LINE, mvmd, repCnt, !viEngine->cdy());
			break;
		case ViCmd::CUR_LAST_CHAR_LINE:
			if( !viEngine->cdy() )
				m_textCursor->movePosition(TextCursor::LAST_CHAR_LINE, mvmd, repCnt, true);
			else
				m_textCursor->movePosition(TextCursor::END_LINE, mvmd, repCnt, false);
			break;
		case ViCmd::CUR_NEXT_FNS_LINE:		//	+ コマンド
			curMove(TextCursor::DOWN, repCnt);
			curMove(TextCursor::FIRST_NOSPACE);
			break;
		case ViCmd::CUR_PREV_FNS_LINE:		//	- コマンド
			curMove(TextCursor::UP, repCnt);
			curMove(TextCursor::FIRST_NOSPACE);
			break;
		case ViCmd::CUR_PREV_BLANK_LINE:
			m_textCursor->movePosition(TextCursor::PREV_BLANK_LINE, mvmd, repCnt);
			break;
		case ViCmd::CUR_NEXT_BLANK_LINE:
			m_textCursor->movePosition(TextCursor::NEXT_BLANK_LINE, mvmd, repCnt);
			break;
		case ViCmd::BEG_OF_CUR_SECTION: {
			//begOfCurSection();
			int v = m_scrollY0;
			m_textCursor->movePosition(TextCursor::BEG_OF_CUR_SECTION, mvmd, repCnt);
			makeCursorInView();
			if( v != m_scrollY0 ) {
				int vln = m_textCursor->viewLine();
				while( vln != 0 ) {
					wchar_t ch = charAt(viewLineStartPosition(vln-1));
					if( isSpaceChar(ch) || ch == '}' ) break;
					--vln;
				}
				m_scrollY0 = vln;
			}
			break;
		}
		case ViCmd::BEG_OF_NEXT_SECTION:
			//begOfNextSection();
			m_textCursor->movePosition(TextCursor::BEG_OF_NEXT_SECTION, mvmd, repCnt);
			break;
		case ViCmd::CUR_BEG_DOC:
			m_textCursor->movePosition(TextCursor::BEG_DOC, mvmd);
			break;
		case ViCmd::CUR_END_DOC:
			m_textCursor->movePosition(TextCursor::END_DOC, mvmd);
			break;
		case ViCmd::CUR_NTH_COLUMN: {
			pos_t pos = columnToPos(m_textCursor->viewLine(), repCnt-1);
			m_textCursor->setPosition(pos, mvmd);
			break;
		}
		case ViCmd::CUR_JUMP_LINE:
			jumpToLine(repCnt - 1, /*vi:*/true);
			break;
		case ViCmd::CUR_ASSOC_PAREN:
			m_textCursor->movePosition(TextCursor::ASSOC_PAREN, mvmd, /*vi:*/true);
			break;
		case ViCmd::CUR_TOP_OF_SCREEN:
			curTopOfScreen(true, repCnt);
			break;
		case ViCmd::CUR_MIDDLE_OF_SCREEN:
			curMiddleOfScreen(true);
			break;
		case ViCmd::CUR_BOTTOM_OF_SCREEN:
			curBottomOfScreen(true, repCnt);
			break;
		case ViCmd::SCROLL_CUR_AT_TOP:
			scrollCurTopOfScreen();
			break;
		case ViCmd::SCROLL_CUR_IN_MIDDLE:
			scrollCurCenterOfScreen();
			break;
		case ViCmd::SCROLL_CUR_AT_BOTTOM:
			scrollCurBottomOfScreen();
			break;
		case ViCmd::SEARCH_PAT: {
			bool fwd;
			const QString &pat = viEngine->searchPat(fwd);
			doFindText(pat, fwd);
			break;
		}
		case ViCmd::SEARCH_CUR_WORD: {
			mainWindow()->setSearchWordOpt(false); 
			mainWindow()->setSearchRegExpOpt(true);
			QString pat;
			searchCurWord(pat, /*vi:*/true);
			break;
		}
		case ViCmd::SEARCH_NEXT: {
			QString pat = mainWindow()->findString();
			if( !pat.isEmpty() ) {
				mainWindow()->setMatchedString(pat);
				//mainWindow()->setShowMatchedBG(true);
				for (int i = 0; i < repCnt; ++i)
					findNext(pat, /*vi=*/true);
			}
			break;
		}
		case ViCmd::SEARCH_PREV: {
			QString pat = mainWindow()->findString();
			if( !pat.isEmpty() ) {
				mainWindow()->setMatchedString(pat);
				//mainWindow()->setShowMatchedBG(true);
				for (int i = 0; i < repCnt; ++i)
					findPrev(pat, /*vi=*/true);
			}
			break;
		}
		case ViCmd::SEARCH_CHAR_f:
			viFindCharForward(viEngine->lastFindChar(), false, mvmd, repCnt);
			break;
		case ViCmd::SEARCH_CHAR_t:
			viFindCharForward(viEngine->lastFindChar(), true, mvmd, repCnt);
			break;
		case ViCmd::SEARCH_CHAR_F:
			viFindCharBackward(viEngine->lastFindChar(), false, mvmd, repCnt);
			break;
		case ViCmd::SEARCH_CHAR_T:
			viFindCharBackward(viEngine->lastFindChar(), true, mvmd, repCnt);
			break;
		case ViCmd::SELECT_CHAR_MODE:
			if( m_textCursor->mode() == TextCursor::VI_CHAR_SEL_MODE ) {
				if( m_textCursor->position() == m_textCursor->anchor() ) {
					m_textCursor->setMode(TextCursor::NOMAL_MODE);
					m_textCursor->movePosition(TextCursor::BEG_WORD);
					m_textCursor->setMode(TextCursor::VI_CHAR_SEL_MODE);
					m_textCursor->movePosition(TextCursor::END_WORD, TextCursor::MOVE_ANCHOR, 1, /*vi:*/true);
				}
			} else
				m_textCursor->setMode(TextCursor::VI_CHAR_SEL_MODE);
			break;
		case ViCmd::SELECT_LINE:
			m_textCursor->setMode(TextCursor::VI_LINE_SEL_MODE);
			//m_textCursor->movePosition(TextCursor::BEG_LINE);
			//m_textCursor->movePosition(TextCursor::DOWN, TextCursor::KEEP_ANCHOR);
			break;
		case ViCmd::TEXT_OBJECT_I:
			textObject(viEngine->textObjectChar(), /*inner*/true);
			break;
		case ViCmd::TEXT_OBJECT_A:
			textObject(viEngine->textObjectChar(), /*inner*/false);
			break;
		case ViCmd::OPEN_NEXT_LINE:
			openNextLine();
			break;
		case ViCmd::OPEN_PREV_LINE:
			openPrevLine();
			break;
		case ViCmd::REPLACE_CHAR: {
			m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, repCnt, /*vi:*/true);
			QString txt;
			for (int i = 0; i < repCnt; ++i) {
				txt += cmdText[cmdText.size() - 1];
			}
			m_textCursor->insertText(txt);
			m_textCursor->movePosition(TextCursor::LEFT);
			break;
		}
		case ViCmd::DELETE_TO_EOL:
			m_textCursor->movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
			if( m_textCursor->hasSelection() ) {
				m_textCursor->deleteChar(false, /*vi:*/true);
				if( !viEngine->cmdText().endsWith("C") )		//	undone: 回数指定
					m_textCursor->movePosition(TextCursor::LEFT, TextCursor::MOVE_ANCHOR, 1, /*vi:*/true);
			}
			break;
		case ViCmd::DELETE_LINE:
			m_textCursor->movePosition(TextCursor::BEG_LINE);
			m_textCursor->movePosition(TextCursor::DOWN, TextCursor::KEEP_ANCHOR, repCnt);
			if( !m_textCursor->hasSelection() )	//	EOF が空行でない場合
				m_textCursor->movePosition(TextCursor::END_LINE, TextCursor::KEEP_ANCHOR);
			if( !m_textCursor->hasSelection() ) return;
			mainWindow()->viEngine()->setYankText(m_textCursor->selectedText(), true);
			m_textCursor->deleteChar();
			break;
		case ViCmd::DELETE_CHAR:
			if( !m_textCursor->hasSelection() )
				m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, repCnt, /*vi:*/true);
			if( m_textCursor->hasSelection() ) {
				//if( mainWindow()->viEngine()->cdy() != 'c' )
				//	openUndoBlock();
				m_textCursor->deleteChar(false, /*vi:*/true);
				pos_t pos = m_textCursor->position();
				if( (isNewLineChar(charAt(pos))
					|| pos == bufferSize() /*&& pos != m_view->viewLineStartPosition(viewLine())*/)
					&& mainWindow()->viEngine()->cdy() != 'c'
					&& mainWindow()->viEngine()->lastCmdChar() != 's' )
				{
					m_textCursor->movePosition(TextCursor::LEFT, TextCursor::MOVE_ANCHOR, 1, /*vi:*/true);
				}
			}
			break;
		case ViCmd::DELETE_PREV_CHAR:
			if( !m_textCursor->hasSelection() )
				m_textCursor->movePosition(TextCursor::LEFT, TextCursor::KEEP_ANCHOR, repCnt, /*vi:*/true);
			if( m_textCursor->hasSelection() ) {
				m_textCursor->deleteChar(true, /*vi:*/true);
			}
			break;
		case ViCmd::TAB_SHIFT:
			if( m_textCursor->hasSelection() ) {
				int ln1 = viewLineToDocLine(m_textCursor->selectionFirstLine());
				int ln2 = viewLineToDocLine(m_textCursor->selectionLastLine());
				if( m_textCursor->selectionLast() == lineStartPosition(ln2) )
					--ln2;
				indent(ln1, ln2, false);
			}
			break;
		case ViCmd::SHIFT_LEFT:
			if( m_textCursor->hasSelection() ) {
				int ln1 = viewLineToDocLine(m_textCursor->selectionFirstLine());
				int ln2 = viewLineToDocLine(m_textCursor->selectionLastLine());
				if( m_textCursor->selectionLast() == lineStartPosition(ln2) )
					--ln2;
				revIndent(ln1, ln2, false);
			} else {
				int ln1 = viewLineToDocLine(m_textCursor->viewLine());
				revIndent(ln1, ln1 + repCnt - 1, true);
				m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mvmd, repCnt, true);
			}
			break;
		case ViCmd::SHIFT_RIGHT:
			if( m_textCursor->hasSelection() ) {
				int ln1 = viewLineToDocLine(m_textCursor->selectionFirstLine());
				int ln2 = viewLineToDocLine(m_textCursor->selectionLastLine());
				if( m_textCursor->selectionLast() == lineStartPosition(ln2) )
					--ln2;
				indent(ln1, ln2, false);
			} else {
				int ln1 = viewLineToDocLine(m_textCursor->viewLine());
				indent(ln1, ln1 + repCnt - 1, true);
				m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mvmd, repCnt, true);
			}
			break;
		case ViCmd::JOIN_LINES:
			joinLines(repCnt, true);
			break;
		case ViCmd::TOGGLE_UPPER_LOWER:
			toggleUpperLowerCase();
			break;
		case ViCmd::TOGGLE_TRUE_FALSE:
			toggleTrueFalse();
			break;
		case ViCmd::INCREMENT:
			incDec(true, repCnt);
			break;
		case ViCmd::DECREMENT:
			incDec(false, repCnt);
			break;
		case ViCmd::YANK_TEXT:
			viYankText();
			break;
		case ViCmd::YANK_LINE:
			viYankLine();
			break;
		case ViCmd::PASTE_PREV:
			viPastePrev();
			break;
		case ViCmd::PASTE_NEXT:
			viPasteNext();
			break;
		case ViCmd::UNDO:
			undo();
			break;
		case ViCmd::REDO:
			redo();
			break;
		case ViCmd::OPEN_UNDO_BLOCK:
			openUndoBlock();
			break;
		case ViCmd::CLOSE_UNDO_BLOCK:
			closeUndoBlock();
			break;
		case ViCmd::CLOSE_ALL_UNDO_BLOCK:
			closeAllUndoBlock();
			break;
		case ViCmd::MARK:
			setMark(m_textCursor->position(), viEngine->lastCmdChar().unicode());
			break;
		case ViCmd::JUMP_MARK_LINE: {
			//jumpMarkPos(viEngine->lastCmdChar().unicode());
			pos_t pos = buffer()->markPos(viEngine->lastCmdChar().unicode());
			if( pos < 0 ) return;
			m_textCursor->setPosition(pos, mvmd);
			m_textCursor->movePosition(TextCursor::FIRST_NOSPACE, mvmd);
			break;
		}
		case ViCmd::JUMP_MARK_POS: {
			//jumpMarkPos(viEngine->lastCmdChar().unicode());
			pos_t pos = buffer()->markPos(viEngine->lastCmdChar().unicode());
			if( pos < 0 ) return;
			m_textCursor->setPosition(pos, mvmd);
			break;
		}
		case ViCmd::EX_DELETE_LINE: {
			int firstPos = lineStartPosition(lns.m_first-1);
			int secondPos = lineStartPosition(lns.m_second-1+1);		//	行番号は1オリジン
			deleteText(firstPos, secondPos - firstPos);
			m_textCursor->setPosition(firstPos);
			m_textCursor->movePosition(TextCursor::FIRST_NOSPACE);
			break;
		}
		case ViCmd::EX_MOVE_LINE: {
			int firstPos = lineStartPosition(lns.m_first-1);
			int secondPos = lineStartPosition(lns.m_second-1+1);		//	行番号は1オリジン
			int destPos = lineStartPosition(lns.m_dest-1+1);
			if( destPos == firstPos ) return;
			m_textCursor->setPosition(firstPos);
			m_textCursor->setPosition(secondPos, TextCursor::KEEP_ANCHOR);
			QString text = m_textCursor->selectedText();
			openUndoBlock();
			m_textCursor->deleteChar();
			if( destPos >= firstPos ) destPos -= secondPos - firstPos;
			m_textCursor->setPosition(destPos);
			m_textCursor->insertText(text);
			closeUndoBlock();
			break;
		}
		case ViCmd::EX_COPY_LINE: {
			int firstPos = lineStartPosition(lns.m_first-1);
			int secondPos = lineStartPosition(lns.m_second-1+1);		//	行番号は1オリジン
			int destPos = lineStartPosition(lns.m_dest-1+1);
			m_textCursor->setPosition(firstPos);
			m_textCursor->setPosition(secondPos, TextCursor::KEEP_ANCHOR);
			QString text = m_textCursor->selectedText();
			m_textCursor->setPosition(destPos);
			m_textCursor->insertText(text);
			break;
		}
		case ViCmd::EX_SHIFT_LEFT:
			revIndent(lns.m_first-1, lns.m_second-1, false);
			break;
		case ViCmd::EX_SHIFT_RIGHT:
			indent(lns.m_first-1, lns.m_second-1, false);
			break;
	}
	//##updateScrollBarInfo();
	makeCursorInView();
	checkAssocParen();
	update();
}
