#include "editView.h"

void EditView::convert_to_lt_gt()
{
	int first = m_textCursor->selectionFirst();
	int last = m_textCursor->selectionLast();
	if( first == last ) {
		first = 0;
		last = bufferSize();
	}
	openUndoBlock();
	while( first < last ) {
		wchar_t ch = charAt(first++);
		QString to;
		switch( ch ) {
			case '<':	to = "&lt;";	break;
			case '>':	to = "&gt;";	break;
			case '&':	to = "&amp;";	break;
		}
		if( !to.isEmpty() ) {
			m_textCursor->setPosition(first);
			m_textCursor->movePosition(TextCursor::LEFT, TextCursor::KEEP_ANCHOR);
			m_textCursor->insertText(to);
			last += to.size() - 1;
		}
	}
	closeUndoBlock();
}
void EditView::convert_lt_gt_to()
{
	int first = m_textCursor->selectionFirst();
	int last = m_textCursor->selectionLast();
	if( first == last ) {
		first = 0;
		last = bufferSize();
	}
	openUndoBlock();
	while( first < last ) {
		wchar_t ch = charAt(first++);
		int sz = 0;
		QString to;
		if( ch == '&' && first < last ) {
			wchar_t ch2 = charAt(first);
#if		0
			if( ch2 == '&' ) {
				sz = 2;
				to = "&";
			} else 
#endif
			if( first + 2 < last && charAt(first+1) == 't' && charAt(first+2) == ';' ) {
				if( ch2 == 'l' ) {
					sz = 4;
					to = "<";
				} else if( ch2 == 'g' ) {
					sz = 4;
					to = ">";
				}
			} else if( ch2 == 'a' && first + 3 < last && charAt(first+1) == 'm'
					&& charAt(first+2) == 'p' && charAt(first+3) == ';' )
			{
				sz = 5;
				to = "&";
			}
			if( !to.isEmpty() ) {
				m_textCursor->setPosition(first-1);
				m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR, sz);
				m_textCursor->insertText(to);
				last += to.size() - sz;
			}
		}
	}
	closeUndoBlock();
}
void EditView::convert_tabSpace()
{
	int first = m_textCursor->selectionFirst();
	int last = m_textCursor->selectionLast();
	if( first == last ) {
		first = 0;
		last = bufferSize();
	}
	m_textCursor->clearSelection();
	QFontMetrics fm(font());
	int spwd = fm.width(" ");		//	空白文字幅
	openUndoBlock();
	while( first < last ) {
		if( charAt(first) == '\t' ) {
			pos_t pos = first;
			int ln = positionToLine(pos);
			pos_t ls = lineStartPosition(ln);
			int px1 = offsetToPx(ln, pos - ls);
			//int column1 = pos;		//	undone: タブ幅を正確に計算
			while( ++first < last && charAt(first) == '\t' ) ;
			int px2 = offsetToPx(ln, first - ls);
			int n = (px2 - px1) / spwd;
			QString spc(n, ' ');
			//int column2 = pos + (first - pos) * typeSettings()->intValue(TypeSettings::TAB_WIDTH);
			//QString spc(column2 - column1, ' ');
			m_textCursor->setPosition(pos);
			m_textCursor->setPosition(first, TextCursor::KEEP_ANCHOR);
			m_textCursor->insertText(spc);
			last += spc.size() - (first - pos);
		} else
			++first;
	}
	closeUndoBlock();
	makeCursorInView();
	update();
}
