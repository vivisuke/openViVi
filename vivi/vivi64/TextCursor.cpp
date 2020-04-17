#include "TextCursor.h"
#include "EditView.h"
#include "viewLineMgr.h"

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
int TextCursor::viewLineStartPosition(int vln) const
{
	return m_view->viewLineMgr()->viewLineStartPosition(vln);
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
	case HOME_LINE:
		pos = viewLineStartPosition(vln);
		while( pos < m_view->bufferSize() && isSpace(m_view->charAt(pos)) )
			++pos;
		if( position() == pos )
			pos = viewLineStartPosition(vln);
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
	if( mode == MOVE_ANCHOR && m_mode == NOMAL_MODE ) {
		m_anchor = pos;
		m_anchorViewLine = m_viewLine;
		if( isBoxSelectionMode() )
			copyBoxCurToAnchor();
	}
#endif
}
