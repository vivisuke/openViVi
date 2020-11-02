#include "editView.h"

typedef unsigned short ushort;

//		ぁ U+3041 〜 ヶ（ひらがな）U+3096
//		ァ U+30a1 〜 ヶ（カタカナ）U+30f6
#define		HIRA_FIRST		0x3041
#define		HIRA_END		0x3096
#define		KANA_FIRST		0x30a1
#define		KANA_END		0x30f6
#define		HANKANA_FIRST		0xff61	//	｡
#define		HANKANA_END			0xff9f	//	ﾟ
#define		ZEN_EXMARK			0xff01	//	!
#define		ZEN_TILDE			0xff5e	//	~
#define		ZENDIGIT_FIRST		0xff10
#define		ZENDIGIT_END		0xff19

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
QChar toLower(QChar ch)
{
	return ch.toLower();
}
QChar toUpper(QChar ch)
{
	return ch.toUpper();
}
void EditView::convert_toLowerCase()
{
	convert(toLower);
}
void EditView::convert_toUpperCase()
{
	convert(toUpper);
}
QChar hiraganaToKatakana(QChar ch)
{
	ushort u = ch.unicode();
	if( u >= HIRA_FIRST && u <= HIRA_END )
		return QChar(u + KANA_FIRST - HIRA_FIRST);
	else
		return ch;
}
QChar katakanaToHiragana(QChar ch)
{
	ushort u = ch.unicode();
	if( u >= KANA_FIRST && u <= KANA_END )
		return QChar(u + HIRA_FIRST - KANA_FIRST);
	else
		return ch;
}
void EditView::convert_HiraganaToKatakana()
{
	convert(hiraganaToKatakana);
}
void EditView::convert_KatakanaToHiragana()
{
	convert(katakanaToHiragana);
}
void EditView::convert(qcharFunc func)
{
	int first = m_textCursor->selectionFirst();
	int last = m_textCursor->selectionLast();
	if( first == last ) {
		first = 0;
		last = bufferSize();
	}
	m_textCursor->clearSelection();
	openUndoBlock();
	while( first < last ) {
		QChar ch = charAt(first);
		QChar ch2 = func(ch);
		if( ch != ch2 ) {
			m_textCursor->setPosition(first);
			m_textCursor->setPosition(first+1, TextCursor::KEEP_ANCHOR);
			m_textCursor->insertText(QString(ch2));
		}
		++first;
	}
	closeUndoBlock();
	makeCursorInView();
	update();
}
