﻿#include "TextCursor.h"
#include "EditView.h"

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
		if( m_pos == 0 ) return;
		--m_pos;
		break;
	}
	case RIGHT: {
		if( m_pos < m_view->buffer()->size() )
			++m_pos;
		break;
	}
	}
}
