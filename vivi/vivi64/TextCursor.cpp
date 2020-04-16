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
		if( pos == 0 ) return;
		--pos;
		break;
	}
	case RIGHT: {
		if( pos < m_view->buffer()->size() )
			++pos;
		break;
	}
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
	}
	m_pos = pos;
}
