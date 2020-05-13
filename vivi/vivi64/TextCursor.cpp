#include "TextCursor.h"
#include "assocParen.h"
#include "EditView.h"
#include "viewLineMgr.h"
#include "Document.h"
#include "MainWindow.h"
#include "ViEngine.h"
#include "../buffer/UTF16.h"
#include "../buffer/bufferUtl.h"

bool isSafeChar(wchar_t ch);

inline bool isSpace(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isSpaceOrNewLine(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}
inline bool isSpaceOrNewLineOrSharp(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '#';
}
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
inline bool isAlpha(wchar_t ch)
{
	return ch < 0x100 && isalpha(ch);
}

TextCursor::TextCursor(EditView *view, pos_t pos, int anchor)
	: m_view(view)
	//, m_boxSelectionMode(false)
	, m_mode(NOMAL_MODE)
	, m_pos(pos)
	, m_anchor(anchor)
	, m_px(0)
{
	int ln = m_view->positionToLine(pos);
	m_viewLine = m_view->docLineToViewLine(ln);
}
TextCursor::TextCursor(const TextCursor &x)
	: m_view(x.m_view)
	//, m_boxSelectionMode(x.m_boxSelectionMode)
	, m_mode(x.m_mode)
	, m_pos(x.m_pos)
	, m_anchor(x.m_anchor)
	, m_viewLine(x.m_viewLine)
	, m_px(x.m_px)
	, m_boxCurLine(x.m_boxCurLine)
	, m_boxCurPx1(x.m_boxCurPx1)
	, m_boxCurPx2(x.m_boxCurPx2)
	, m_boxAnchorLine(x.m_boxAnchorLine)
	, m_boxAnchorPx1(x.m_boxAnchorPx1)
	, m_boxAnchorPx2(x.m_boxAnchorPx2)
{
	//m_viewLine = m_view->viewLine();
}
wchar_t TextCursor::charAt() const
{
	return m_view->charAt(position());
}
wchar_t TextCursor::charAt(int pos) const
{
	return m_view->charAt(pos);
}
int TextCursor::viewLineStartPosition(int vln) const
{
	return m_view->viewLineMgr()->viewLineStartPosition(vln);
}
int TextCursor::positionInLine() const
{
	int ln = m_view->positionToLine(m_pos);
	return m_pos - m_view->lineStartPosition(ln);
}
void TextCursor::movePosition(int op, int mode, int n, bool vi)
{
	pos_t pos = position();
	const pos_t pos0 = pos;
	Q_ASSERT( pos >= 0 );
	int vln0 = viewLine();
	int vln = vln0;
	int dln = m_view->viewLineToDocLine(vln);
	switch( op ) {
	case LEFT: {
		pos_t ls = viewLineStartPosition(vln);
		for (int  i = 0; i < n && pos != 0; ++i) {
			if( vi && ls == pos ) break;
			if( --pos < ls ) {
				ls = viewLineStartPosition(--vln);
				if( pos != 0 && m_view->charAt(pos) == '\n'
					&& m_view->charAt(pos - 1) == '\r' )
				{
					--pos;
				}
			}
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	}
	case RIGHT: {
		if( pos == m_view->bufferSize() ) return;
		pos_t nxls = viewLineStartPosition(vln+1);
		for (int  i = 0; i < n; ++i) {
#if	0
			if( vi ) {
				if( mode == MOVE_ANCHOR
					&& (m_view->charAt(pos+1) == '\r' || m_view->charAt(pos+1) == '\n') )
					break;
				if( mode == KEEP_ANCHOR
					&& (m_view->charAt(pos) == '\r' || m_view->charAt(pos) == '\n') )
					break;
			}
#endif
			auto ch = m_view->charAt(pos);
			if( m_view->charAt(pos) == '\r' || m_view->charAt(pos) == '\n' ) {
				if( m_view->charAt(pos) == '\r' && m_view->charAt(pos+1) == '\n' )
					pos += 2;
				else
					pos += 1;
				++vln;
				//ln = m_view->viewLineToDocLine(++vln, offset);
				//pos = viewLineStartPosition(vln);
				nxls = viewLineStartPosition(vln+1);
			} else {
				if( ++pos >= nxls && vln < m_view->viewLineMgr()->EOFLine() ) {
					++vln;
					//ln = m_view->viewLineToDocLine(++vln, offset);
					nxls = viewLineStartPosition(vln+1);
				}
			}
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - m_view->viewLineStartPosition(vln));
		break;
	}
	case UP:
		if( (vln -= n) < 0 ) vln = 0;
		
			pos = m_view->viewLineMgr()->viewLineStartPosition(vln);
			pos += m_view->pxToOffset(vln, m_px);
		break;
	case DOWN:
		vln = qMin(vln + n, m_view->EOFLine());
#if	1
			pos = m_view->viewLineMgr()->viewLineStartPosition(vln);
			pos += m_view->pxToOffset(vln, m_px);
#else
		if( vln == vln0 && mode != KEEP_ANCHOR ) {
			clearSelection();
			return;
		}
		if( isBoxSelectionMode() ) {
			int ln = m_view->viewLineToDocLine(vln);
			pos = m_view->lineStartPosition(ln);
			pos += m_view->pxToOffset(vln, m_boxCurPx1);
			pos_t posNL = m_view->newLinePosition(ln);
			if( pos >= posNL ) {
				//	改行以降の場合、px1, px2 は変化しない
				m_pos = pos;
				m_boxCurLine = vln;
				if( mode == MOVE_ANCHOR ) {
					m_anchor = pos;
					m_anchorViewLine = m_viewLine;
					copyBoxCurToAnchor();
				}
				return;
			}
		} else {
			pos = m_view->viewLineMgr()->viewLineStartPosition(vln);
			pos += m_view->pxToOffset(vln, m_px);
		}
#endif
		break;
	case PREV_WORD:
		pos = prevWord(n);
		while( pos < m_view->viewLineStartPosition(vln) ) {
			--vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - m_view->viewLineStartPosition(vln));
		break;
	case NEXT_WORD:
		if( pos == m_view->bufferSize() ) return;
		pos = nextWord(n);
		while( pos <= m_view->bufferSize() && vln != m_view->EOFLine() && pos >= viewLineStartPosition(vln+1) ) {
			++vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case BEG_WORD:
		pos = begWord();
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case END_WORD:
		if( pos == m_view->bufferSize() ) return;
		pos = endWord(n);
		if( vi && mode != KEEP_ANCHOR ) {
			if( vi && pos == pos0 + 1 )
				pos = endWord(1);
			--pos;
		}
		vln = m_view->viewLineMgr()->positionToViewLine(pos);
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case NEXT_WORD_NOSKIPSPC:
		if( pos == m_view->bufferSize() ) return;
		pos = nextWord(n, true);
		while( pos < m_view->bufferSize() && pos >= viewLineStartPosition(vln+1) ) {
			++vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case NEXT_CAP_WORD:
		if( (pos = nextCapWord(n)) >= viewLineStartPosition(vln+1) ) {
			++vln;
		}
		if( m_mode == VI_CHAR_SEL_MODE ) --pos;
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case NEXT_SS_WORD:
		if( pos == m_view->bufferSize() ) return;
		pos = nextSSWord(n);
		while( pos < m_view->bufferSize() && pos >= viewLineStartPosition(vln+1) ) {
			++vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case NEXT_SS_WORD_NOSKIPSPC:
		if( pos == m_view->bufferSize() ) return;
		pos = nextSSWord(n, true);
		while( pos < m_view->bufferSize() && pos >= viewLineStartPosition(vln+1) ) {
			++vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case PREV_SS_WORD:
		pos = prevSSWord(n);
		while( pos < m_view->viewLineStartPosition(vln) ) {
			--vln;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - m_view->viewLineStartPosition(vln));
		break;
	case END_SS_WORD:
		if( pos == m_view->bufferSize() ) return;
		pos = endSSWord(n);
		if( vi && mode != KEEP_ANCHOR )
			--pos;
		vln = m_view->viewLineMgr()->positionToViewLine(pos);
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case BEG_LINE:
		pos = viewLineStartPosition(vln);
		m_px = m_view->viewLineOffsetToPx(vln, 0);
		break;
	case HOME_LINE:
		pos = viewLineStartPosition(vln);
		while( pos < m_view->bufferSize() && isSpace(m_view->charAt(pos)) )
			++pos;
		if( position() == pos )
			pos = viewLineStartPosition(vln);
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case FIRST_NOSPACE:
		pos = viewLineStartPosition(vln);
		while( pos < m_view->bufferSize() && isSpace(m_view->charAt(pos)) )
			++pos;
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case LAST_CHAR_LINE:
	case END_LINE: {
		if( pos != m_view->bufferSize() ) {
			pos = viewLineStartPosition(vln + 1);
			wchar_t pch = m_view->charAt(pos-1);
			if( pch == '\n') {
				if( m_view->charAt(pos-2) == '\r' )
					pos -= 2;
				else
					--pos;
			} else if( pch == '\r' )
				--pos;
			m_px = INT_MAX;
		}
		if( op == LAST_CHAR_LINE && pos != m_view->lineStartPosition(dln) )
			--pos;
		break;
	}
	case BEG_DOC:
		vln = 0;
		pos = 0;
		m_px = 0;
		break;
	case END_DOC:
		pos = m_view->bufferSize();
		vln = m_view->EOFLine();
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	case ASSOC_PAREN: {
		//pos = associatedParen();
		bool bSharp;
		pos_t pos0 = pos;
		pos = assocParenPosition(m_view->typeSettings(), *m_view->buffer(), pos, bSharp);
		vln = m_view->viewLineMgr()->positionToViewLine(pos);
		if( mode == KEEP_ANCHOR && pos > pos0 && bSharp ) {
			//	#if から #endif までを選択する場合は #endif の次行まで選択
			pos = m_view->viewLineStartPosition(vln+1) - 1;
		}
		if( mode == KEEP_ANCHOR ) {
			if( pos < m_anchor )
				++m_anchor;
			else if( pos > m_anchor )
				++pos;
		}
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		break;
	}
	case BEG_OF_CUR_SECTION:
		if( !dln ) {
			pos = 0;
			vln = 0;
			break;
		}
		pos = m_view->viewLineStartPosition(dln);
		for (int i = 0; pos != 0 && i < n; ++i) {
			if( m_view->charAt(pos) == '{' ) {
				pos = m_view->viewLineStartPosition(--dln);
			}
			while( pos != 0 && m_view->charAt(pos) != '{' ) {
				pos = m_view->viewLineStartPosition(--dln);
			}
		}
		vln = m_view->docLineToViewLine(dln);
		break;
	case BEG_OF_NEXT_SECTION:
		pos = m_view->viewLineStartPosition(dln);
		for (int i = 0; pos < m_view->bufferSize() && i < n; ++i) {
			if( m_view->charAt(pos) == '{' ) {
				pos = m_view->viewLineStartPosition(++dln);
			}
			while( pos < m_view->bufferSize() && m_view->charAt(pos) != '{' ) {
				pos = m_view->viewLineStartPosition(++dln);
			}
		}
		vln = m_view->docLineToViewLine(dln);
		break;
	case PREV_BLANK_LINE:
		pos = m_view->viewLineStartPosition(vln);
		for (int i = 0; pos != 0 && i < n; ++i) {
			while( pos != 0 && isNewLine(m_view->charAt(pos)) ) {
				--vln;
				pos = m_view->viewLineStartPosition(vln);
			}
			while( pos != 0 && !isNewLine(m_view->charAt(pos)) ) {
				--vln;
				pos = m_view->viewLineStartPosition(vln);
			}
		}
		break;
	case NEXT_BLANK_LINE:
		pos = m_view->viewLineStartPosition(vln);
		for (int i = 0; pos < m_view->bufferSize() && i < n; ++i) {
			while(  pos < m_view->bufferSize() && isNewLine(m_view->charAt(pos)) ) {
				++vln;
				pos = m_view->viewLineStartPosition(vln);
			}
			while(  pos < m_view->bufferSize() && !isNewLine(m_view->charAt(pos)) ) {
				++vln;
				pos = m_view->viewLineStartPosition(vln);
			}
		}
		break;
	case PREV_HEADING_LINE: {
		int ln = m_view->viewLineToDocLine(vln);
		pos = m_view->lineStartPosition(ln);
		if( pos != 0 ) {
			//ln = m_view->viewLineToDocLine(--vln, offset);
			pos = m_view->lineStartPosition(--ln);		//	強制的に１行移動
		}
		while( pos != 0 && isSpaceOrNewLineOrSharp(m_view->charAt(pos)) ) {
			//ln = m_view->viewLineToDocLine(--vln, offset);
			pos = m_view->lineStartPosition(--ln);
		}
		pos_t p = pos;
		//int v = vln;
		for(;;) {
			if( !pos ) {
				p = pos;
				break;
			}
			if( isSpaceOrNewLineOrSharp(m_view->charAt(pos)) )
				break;
			p = pos;
			pos = m_view->lineStartPosition(--ln);
		}
#if	0
		while( pos != 0 && !isSpaceOrNewLineOrSharp(m_view->charAt(pos)) ) {
			p = pos;
			//ln = m_view->viewLineToDocLine(--vln, offset);
			pos = m_view->lineStartPosition(--ln);
		}
#endif
		vln = m_view->viewLineMgr()->positionToViewLine(pos = p);
		m_px = m_view->viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln));
		//vln = m_view->docLineToViewLine(ln = l);
		//ln = m_view->viewLineToDocLine(vln = v, offset);
		break;
	}
	case NEXT_HEADING_LINE: {
		int ln = m_view->viewLineToDocLine(vln);
		pos = m_view->lineStartPosition(ln);
		while( pos < m_view->bufferSize() && !isSpaceOrNewLineOrSharp(m_view->charAt(pos)) ) {
			//ln = m_view->viewLineToDocLine(++vln, offset);
			pos = m_view->lineStartPosition(++ln);
		}
		while( pos < m_view->bufferSize() && isSpaceOrNewLineOrSharp(m_view->charAt(pos)) ) {
			//++vln;
			//ln = m_view->viewLineToDocLine(++vln, offset);
			pos = m_view->lineStartPosition(++ln);
		}
		vln = m_view->viewLineMgr()->positionToViewLine(pos);
		break;
	}
	default:
		return;
	}
	//m_pos = pos;
	setLineAndPosition(vln, pos, mode);
}
void TextCursor::setLineAndPosition(int vln, pos_t pos, int mode)
{
	Q_ASSERT( pos >= 0 );
	m_viewLine = vln;
	m_pos = pos;
#if	0
	if( isBoxSelectionMode() ) {
		updateBoxCur();
	}
#endif
	if( mode == MOVE_ANCHOR && m_mode == NOMAL_MODE ) {
		m_anchor = pos;
		m_anchorViewLine = m_viewLine;
		//if( isBoxSelectionMode() )
		//	copyBoxCurToAnchor();
	}
}
uchar TextCursor::getCharType(wchar_t &ch)
{
	if( m_pos == m_view->bufferSize() )
		return CT_EOF;
	ch = m_view->charAt(m_pos);
	return UTF16CharType(ch);
}
//	cw: true の場合は、継続する空白をスキップしない
int TextCursor::nextWord(int n, bool cw)
{
	uchar type2;
	const bool HTML = false;
	const bool dollarPrefix = false;
	const bool dollar = false;
	const bool skipTailSpaces = !cw;
	const bool vi = false;
	const bool skipNewlines = false;
	wchar_t ch;
	while( --n >= 0 ) {
		uchar type = getCharType(ch);
		if( type == CT_EOF ) break;
		bool dollar = true;
		switch( type ) {
		case CTSB_SPACE:
		case CTSB_ALNUM:
		case CTSB_KANA:
		case CTSB_SYM:
		case CTDB_SPACE:
			do {
				if( ch != '$' ) dollar = false;
				++m_pos;
			} while( (type2 = getCharType(ch)) == type && !(HTML && ch == '<') );		//	同じ文字種、または <
			if( dollar && dollarPrefix && type2 == CTSB_ALNUM ) {
				type = type2;
				goto skipAlnum;
			}
skipSpaces:
			if( n == 0 && !skipTailSpaces ) break;
			while( (type = getCharType(ch)) == CTSB_SPACE || type == CTDB_SPACE )	//	継続スペース
				++m_pos;
//skipNewlines:
			if( n > 0 || vi && skipNewlines ) {
				while( (type = getCharType(ch)) == CT_NEWLINE || type == CTSB_SPACE || type == CTDB_SPACE )
					++m_pos;
			}
			break;
		case CTDB_KANJI:
			do {
				++m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			while( getCharType(ch) == CTDB_CONT )		//	継続文字（ヽヾゝゞ〃仝々）
				++m_pos;
			goto skipSpaces;
			break;
		case CTDB_HIRA:
		case CTDB_KANA:
		case CTDB_ALNUM:
		case CTDB_SYM:
		case CTDB_CONT:
skipAlnum:
			do {
				++m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			goto skipSpaces;
		case CT_NEWLINE:
			if( ch == '\r' && m_view->charAt(m_pos+1) == '\n' )
				++m_pos;
		case CTSB_OTHER:
		case CTDB_OTHER:
			++m_pos;
			goto skipSpaces;
		default:
			//ASSERT(0);
			Q_ASSERT(0);
		}
	}
	return m_pos;
}
int TextCursor::nextCapWord(int n)
{
	while( --n >= 0 ) {
		wchar_t ch = m_view->charAt(m_pos);
		bool upper = false;
		if( ch >= 'A' && ch <= 'Z') {
			upper = true;
		} else if( ch >= 'a' && ch <= 'z' || ch == '_' ) {
		} else {
			return m_pos;
			//return nextWord(n);
		}
		for(;;) {
			ch = m_view->charAt(++m_pos);
			if( ch >= 'A' && ch <= 'Z') {
				if( !upper ) break;
			} else if( ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' ) {
				upper = false;
			} else { //if( ch == '_' )
				break;
				//return m_pos;
			}
		}
	}
	return m_pos;
}
int TextCursor::prevWord(int n)
{
	const bool HTML = false;
	const bool dollarPrefix = false;
	const bool dollar = false;
	const bool skipTailSpaces = true;
	const bool vi = false;
	const bool skipNewlines = false;
	wchar_t ch;
	uchar type;
	int vln = viewLine();
	int offset;
	pos_t ls = m_view->lineStartPosition(m_view->viewLineToDocLine(vln, offset));
	while( --n >= 0 ) {
		if( !m_pos ) break;
		--m_pos;
		if( m_pos != 0 && m_view->charAt(m_pos-1) == '\r' && m_view->charAt(m_pos) == '\n' )
			--m_pos;
		while( (type = getCharType(ch)) == CTSB_SPACE || type == CTDB_SPACE ||		//	継続スペース
				vi && type == CT_NEWLINE )		//	vi モードでは改行は空白扱い
		{
			if( !m_pos ) return 0;
			if( m_pos == ls ) break;	//	行頭の場合
			--m_pos;
		}
		type = getCharType(ch);
		//if( vi && type == CT_NEWLINE )
		//	type = CTSB_SPACE;		//	vi モードでは改行は空白扱い
		if( type == CT_EOF ) break;
		bool alnum = false;
		switch( type ) {
		case CTSB_ALNUM:
			alnum = true;
		case CTSB_SPACE:
		case CTSB_KANA:
		case CTSB_SYM:
		case CTDB_SPACE:
			do {
				if( !m_pos ) return 0;
				--m_pos;
			} while( !(HTML && ch == '<') && getCharType(ch) == type );		//	同じ文字種、または <
			break;
		case CTDB_CONT:
			do {
				if( !m_pos ) return 0;
				--m_pos;
			} while( getCharType(ch) == type );		//	継続文字
			if( type != CTDB_KANJI )
				break;
			//	スルー
		case CTDB_KANJI:
			do {
				if( !m_pos ) return 0;
				--m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			break;
		case CTDB_HIRA:
		case CTDB_KANA:
		case CTDB_ALNUM:
		case CTDB_SYM:
			do {
				if( !m_pos ) return 0;
				--m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			break;
		case CT_NEWLINE:
		case CTSB_OTHER:
		case CTDB_OTHER:
			--m_pos;
			break;
		default:
			//ASSERT(0);
			Q_ASSERT(0);
		}
#if 0
		if( alnum && m_vwLineMgr->getTypeSettings()->getBoolValue(TYPESTG_DOLLAR_PREFIX_WORD) ) {
			while( getChar() == '$' ) {
				if( !m_pos ) return 0;
				--m_pos;
			}
		}
#endif
		++m_pos;
	}
	return m_pos;
}
int TextCursor::begWord()
{
	if( !m_pos ) return m_pos;
	int work = m_pos;
	prevWord(1);			//	前の単語先頭に移動
	if( m_pos == m_view->bufferSize() ) {
		//m_pos = work;
	} else {
		int beg = m_pos;
		nextWord(1);			//	次の単語先頭に移動
		if( work != m_pos )			//	元の位置が単語先頭でない
			m_pos = beg;
	}
	return m_pos;
}
int TextCursor::endWord(int n)
{
	uchar t;
	wchar_t ch;
	uchar type2;
	//const bool dollarPrefix = m_vwLineMgr->getTypeSettings()->getBoolValue(TYPESTG_DOLLAR_PREFIX_WORD);
	for (int i = 0; i < n; ++i) {
		while( (t = getCharType(ch)) == CTSB_SPACE || t == CTDB_SPACE )	//	スペース類をスキップ
			++m_pos;
		uchar type = getCharType(ch);
		bool dollar = true;
		if( type == CT_EOF ) return true;
		switch( type ) {
		case CTSB_SPACE:
		case CTSB_ALNUM:
		case CTSB_KANA:
		case CTSB_SYM:
		case CTDB_SPACE:
			do {
				if( ch != '$' ) dollar = false;
				++m_pos;
			} while( (type2 = getCharType(ch)) == type );		//	同じ文字種
			break;
		case CTDB_KANJI:
			do {
				++m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			while( getCharType(ch) == CTDB_CONT )		//	継続文字（ヽヾゝゞ〃仝々）
				++m_pos;
			break;
		case CTDB_HIRA:
		case CTDB_KANA:
		case CTDB_ALNUM:
		case CTDB_SYM:
		case CTDB_CONT:
	skipAlnum:
			do {
				++m_pos;
			} while( getCharType(ch) == type );		//	同じ文字種
			break;
		case CT_NEWLINE:
		case CTSB_OTHER:
		case CTDB_OTHER:
			++m_pos;
			break;
		default:
			Q_ASSERT(0);
		}
	}
	return m_pos;
}
int TextCursor::nextSSWord(int n, bool cw)
{
	wchar_t ch;
	while( --n >= 0 ) {
		uchar type = getCharType(ch);		//	ch に文字を読み込む
		switch( type ) {
		default:
			do {
				//mvRight();
				++m_pos;
			} while( m_pos < m_view->bufferSize() && (type = getCharType(ch)) != CTSB_SPACE
				&& type != CTDB_SPACE && type != CT_NEWLINE && type != CT_EOF );
			if( cw )
				return m_pos;		//	cw の場合は継続空白類をスキップしない
			//	下にスルー
		case CTSB_SPACE:
		case CT_NEWLINE:
		case CTDB_SPACE:
			do {
				//mvRight();
				++m_pos;
			} while( m_pos < m_view->bufferSize() &&
					((type = getCharType(ch)) == CTSB_SPACE || type == CTDB_SPACE || type == CT_NEWLINE) );
			break;
		}
	}
	return m_pos;
}
int TextCursor::prevSSWord(int n)
{
	byte type;
	wchar_t ch;
	while( --n >= 0 ) {
		if( !m_pos ) break;		//	バッファ先頭にいる場合
		--m_pos;
		if( !m_pos ) break;
		while( (type = getCharType(ch)) == CTSB_SPACE || type == CTDB_SPACE || type == CT_NEWLINE ) {//	継続スペース
			if( !--m_pos )
				return m_pos;
		}
		type = getCharType(ch);
		if( type == CT_EOF ) break;
		switch( type ) {
		default:
			do {
				if( !--m_pos )
					return m_pos;
			} while( (type = getCharType(ch)) != CTSB_SPACE && type != CTDB_SPACE && type != CT_NEWLINE && type != CT_EOF );
			if( /*!skipTailSpaces ||*/ (/*!skipNewlines &&*/ type == CT_NEWLINE) ) break;
		case CTSB_SPACE:
		case CT_NEWLINE:
		case CTDB_SPACE:
			//do {
			//	mvLeft();
			//} while( (type = getCharType(ch)) == CTSB_SPACE || type == CTDB_SPACE || type == CT_NEWLINE );
			break;
		}
		//mvRight();
		++m_pos;
	}
	return m_pos;
}
int TextCursor::endSSWord(int n)
{
	wchar_t ch;
	if( getCharType(ch) == CT_EOF ) return false;
	uchar type;
	++m_pos;
	while( --n >= 0 ) {
		while( (type = getCharType(ch)) == CT_NEWLINE || type == CTSB_SPACE || type == CTDB_SPACE ) {
			++m_pos;
		}
		while( m_pos < m_view->bufferSize() && (type = getCharType(ch)) != CT_NEWLINE
					&& type != CTSB_SPACE && type != CTDB_SPACE && type != CT_EOF )
		{
			++m_pos;
		}
	}
	return m_pos;
}
void TextCursor::setMode(byte mode)
{
	if( (m_mode = mode) == VI_LINE_SEL_MODE )
		m_anchorViewLine = m_viewLine;
}
void TextCursor::clearSelection()
{
	m_mode = NOMAL_MODE;
	m_anchor = m_pos;
}
void TextCursor::setPosition(pos_t pos, int mode)
{
	Q_ASSERT( pos >= 0 );
	//int ln = m_view->positionToLine(pos);
	//m_viewLine = m_view->docLineToViewLine(ln);
	m_viewLine = m_view->viewLineMgr()->positionToViewLine(pos);
	int ln = m_view->viewLineToDocLine(m_viewLine);
	m_pos = pos;
	m_px = m_view->viewLineOffsetToPx(m_viewLine, pos - m_view->viewLineStartPosition(m_viewLine));
#if	0
	if( isBoxSelectionMode() ) {
		updateBoxCur();
	}
#endif
	if( mode == MOVE_ANCHOR && m_mode == NOMAL_MODE ) {
		m_anchor = pos;
		m_anchorViewLine = m_viewLine;
#if	0
		if( isBoxSelectionMode() )
			copyBoxCurToAnchor();
#endif
	}
}
int TextCursor::selectionSize() const
{
	int sz = m_pos - m_anchor;
	if( sz < 0 ) sz = -sz;
	return sz;
}
int TextCursor::selectionFirst() const
{
	if( m_mode != VI_LINE_SEL_MODE )
		return qMin(m_pos, m_anchor);
	int vln = qMin(m_viewLine, m_anchorViewLine);
	//return m_view->viewLineStartPosition(vln);
	//	行選択モードの場合、選択範囲は論理行単位
	int dln = m_view->viewLineToDocLine(vln);
	return m_view->lineStartPosition(dln);
}
int TextCursor::selectionLast() const
{
	if( m_mode == VI_CHAR_SEL_MODE )
		return qMax(m_pos, m_anchor) + 1;
	if( m_mode != VI_LINE_SEL_MODE )
		return qMax(m_pos, m_anchor);
	int vln = qMax(m_viewLine, m_anchorViewLine);
	//return m_view->viewLineStartPosition(vln+1);
	int dln = m_view->viewLineToDocLine(vln);
	return m_view->lineStartPosition(dln+1);
}
int TextCursor::selectionFirstLine() const
{
	return qMin(m_viewLine, m_anchorViewLine);
}
int TextCursor::selectionLastLine() const
{
	return qMax(m_viewLine, m_anchorViewLine);
}
QString TextCursor::selectedText() const
{
	if( !hasSelection() ) return QString();
	int first = selectionFirst();
	int last = selectionLast();
	return m_view->text(first, last - first);
}
bool TextCursor::getSelectedLineRange(int &dln1, int &dln2) const
{
	if( !hasSelection() ) return false;
	dln1 = m_view->viewLineToDocLine(selectionFirstLine());
	dln2 = m_view->viewLineToDocLine(selectionLastLine());
	if( selectionLast() == m_view->lineStartPosition(dln2) )
		--dln2;
	return true;
}
void TextCursor::deleteChar(bool isBS, bool vi)
{
#if	0
	if( isBoxSelectionMode() ) {
		deleteBoxSelected(isBS);
		return;
	}
#endif
	if( !hasSelection() ) {
		movePosition(RIGHT, KEEP_ANCHOR);
		if( !hasSelection() ) return;
	}
	if( vi ) {
		QString txt = selectedText();
		m_view->mainWindow()->viEngine()->setYankText(txt, /*bLine:*/false);
	}
	pos_t pos = selectionFirst();
	m_view->deleteText(pos, selectionLast() - pos, isBS);
	setPosition(pos);
	clearSelection();
	m_viewLine = m_view->viewLineMgr()->positionToViewLine(pos);
	m_px = m_view->viewLineOffsetToPx(m_viewLine, pos - m_view->viewLineStartPosition(m_viewLine));
	m_view->update();
	m_view->document()->updateView(m_view);
#if	0
	pos_t pos = selectionFirst();
	if( pos == m_view->document()->size() ) return;
	if( !hasSelection() ) {
		int sz = charAt(pos) == '\r' && pos + 1 < m_view->document()->size() && charAt(pos+1) == '\n' ? 2 : 1;
		m_view->deleteText(pos, sz, BS);
	} else
		m_view->deleteText(pos, selectionLast() - pos, BS);
	setPosition(pos);
	clearSelection();
	m_viewLine = m_view->viewLineMgr()->positionToViewLine(pos);
	m_px = m_view->viewLineOffsetToPx(m_viewLine, pos - m_view->viewLineStartPosition(m_viewLine));
	m_view->update();
	m_view->document()->updateView(m_view);
#endif
}
void TextCursor::insertText(const QString &text)
{
	Q_ASSERT( m_pos >= 0 );
#if	0
	m_view->clearOpenCloseParenPos();
	if( isBoxSelectionMode() ) {
		insertTextBoxSelected(text);
		return;
	}
#endif
	bool im = m_view->isModified();
	if( hasSelection() ) {
		pos_t pos = selectionFirst();
		bool bo = m_view->buffer()->isUndoBlockOpened();
		if( !bo )
			m_view->openUndoBlock();
		m_view->deleteText(pos, selectionLast() - pos, true);
		m_view->insertTextRaw(pos, text);
		if( !bo )
			m_view->closeUndoBlock();
		setPosition(pos += text.size());
	} else {
		m_view->insertTextRaw(position(), text);
		//int sz = text == "\r\n" ? 1 : text.size();
		QString t = text;
		t.replace("\r\n", "\n");	//	CRLF は1回で移動
		movePosition(RIGHT, MOVE_ANCHOR, t.size());
	}
	m_view->update();
	m_view->document()->updateView(m_view);
	if( !im )
		m_view->emitModifiedChanged();
}
