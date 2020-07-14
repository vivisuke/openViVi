#include "editView.h"

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
