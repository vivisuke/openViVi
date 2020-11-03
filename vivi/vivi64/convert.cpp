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
QChar hanNumberToZenNumber(QChar ch)
{
	if( ch >= '0' && ch <= '9' )
		return QChar(ch.unicode() - '0' + ZENDIGIT_FIRST);
	else
		return ch;
}
QChar zenNumberToHanNumber(QChar ch)
{
	ushort code = ch.unicode();
	if( code >= ZENDIGIT_FIRST && code <= ZENDIGIT_END )
		return QChar(code  - ZENDIGIT_FIRST + '0');
	else
		return ch;
}
void EditView::convert_HanNumberToZenNumber()
{
	convert(hanNumberToZenNumber);
}
void EditView::convert_ZenNumberToHanNumber()
{
	convert(zenNumberToHanNumber);
}
QChar hanAlnumToZenAlnum(QChar ch)
{
	if( ch >= '0' && ch <= '9' )
		return QChar(ch.unicode() - '0' + ZENDIGIT_FIRST);
	else if( ch >= 'a' && ch <= 'z' )
		return QChar(ch.unicode() - 'a' + L'ａ');
	else if( ch >= 'A' && ch <= 'Z' )
		return QChar(ch.unicode() - 'A' + L'Ａ');
	else
		return ch;
}
QChar zenAlnumToHanAlnum(QChar ch)
{
	ushort code = ch.unicode();
	if( code >= ZENDIGIT_FIRST && code <= ZENDIGIT_END )
		return QChar(code  - ZENDIGIT_FIRST + '0');
	else if( code >= L'ａ' && code <= L'ｚ' )
		return QChar(code  - L'ａ' + 'a');
	else if( code >= L'Ａ' && code <= L'Ｚ' )
		return QChar(code  - L'Ａ' + 'A');
	else
		return ch;
}
void EditView::convert_HanAlnumToZenAlnum()
{
	convert(hanAlnumToZenAlnum);
}
void EditView::convert_ZenAlnumToHanAlnum()
{
	convert(zenAlnumToHanAlnum);
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
ushort toZenkakuKanaTableUTF16[] = {
/* 6 */	       L'。', L'「', L'」', L'、', L'・', L'ヲ', L'ァ',
		L'ィ', L'ゥ', L'ェ', L'ォ', L'ャ', L'ュ', L'ョ', L'ッ',
/* 7 */	L'ー', L'ア', L'イ', L'ウ', L'エ', L'オ', L'カ', L'キ',
		L'ク', L'ケ', L'コ', L'サ', L'シ', L'ス', L'セ', L'ソ',
/* 8 */	L'タ', L'チ', L'ツ', L'テ', L'ト', L'ナ', L'ニ', L'ヌ',
		L'ネ', L'ノ', L'ハ', L'ヒ', L'フ', L'ヘ', L'ホ', L'マ',
/* 9 */	L'ミ', L'ム', L'メ', L'モ', L'ヤ', L'ユ', L'ヨ', L'ラ',
		L'リ', L'ル', L'レ', L'ロ', L'ワ', L'ン', L'゛', L'゜',
};
const byte withDakuten[] = {		//	L'ァ' から、濁点可能かどうか
	   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 1, 0, 1, 0, 1, 	//	L'カ', L'キ' L'ク'
	0, 1, 0, 1, 0, 1, 0, 1,  0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 0, 1, 0, 1, 0,  1, 0, 0, 0, 0, 0, 0, 3,
	0, 0, 3, 0, 0, 3, 0, 0,  3, 0, 0, 3, 0, 0, 0, 0,
};
ushort toZenkakuTableUTF16[256] = {
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	L'　', L'！', L'”', L'＃', L'＄', L'％', L'＆', L'’',
		L'（', L'）', L'＊', L'＋', L'，', L'‐', L'．', L'／',
/* 3 */	L'０', L'１', L'２', L'３', L'４', L'５', L'６', L'７',
		L'８', L'９', L'：', L'；', L'＜', L'＝', L'＞', L'？',
/* 4 */	L'＠', L'Ａ', L'Ｂ', L'Ｃ', L'Ｄ', L'Ｅ', L'Ｆ', L'Ｇ',
		L'Ｈ', L'Ｉ', L'Ｊ', L'Ｋ', L'Ｌ', L'Ｍ', L'Ｎ', L'Ｏ',
/* 5 */	L'Ｐ', L'Ｑ', L'Ｒ', L'Ｓ', L'Ｔ', L'Ｕ', L'Ｖ', L'Ｗ',
		L'Ｘ', L'Ｙ', L'Ｚ', L'［', L'￥', L'］', L'＾', L'＿',
/* 6 */	L'｀', L'ａ', L'ｂ', L'ｃ', L'ｄ', L'ｅ', L'ｆ', L'ｇ',
		L'ｈ', L'ｉ', L'ｊ', L'ｋ', L'ｌ', L'ｍ', L'ｎ', L'ｏ',
/* 7 */	L'ｐ', L'ｑ', L'ｒ', L'ｓ', L'ｔ', L'ｕ', L'ｖ', L'ｗ',
		L'ｘ', L'ｙ', L'ｚ', L'｛', L'｜', L'｝', L'〜', 0,
};
void EditView::convert_HanCharToZenChar(bool ansi)
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
		ushort code = charAt(first);
		if( ansi && code >= ' ' && code <= 0x7f && toZenkakuTableUTF16[code] != 0 ) {
			m_textCursor->setPosition(first);
			m_textCursor->setPosition(first+1, TextCursor::KEEP_ANCHOR);
			m_textCursor->insertText(QString(QChar(toZenkakuTableUTF16[code])));
		} else if( code >= HANKANA_FIRST && code <= HANKANA_END ) {
			m_textCursor->setPosition(first);
			code = toZenkakuKanaTableUTF16[code - HANKANA_FIRST];
			if( charAt(first+1) == L'ﾞ' && withDakuten[code - L'ァ'] ) {
				m_textCursor->setPosition(first+2, TextCursor::KEEP_ANCHOR);
				m_textCursor->insertText(QString(QChar(code+1)));
			} else if( charAt(first+1) == L'ﾟ' && (withDakuten[code - L'ァ'] & 2) ) {
				m_textCursor->setPosition(first+2, TextCursor::KEEP_ANCHOR);
				m_textCursor->insertText(QString(QChar(code+2)));
			} else {
				m_textCursor->setPosition(first+1, TextCursor::KEEP_ANCHOR);
				m_textCursor->insertText(QString(QChar(code)));
			}
		}
		++first;
	}
	closeUndoBlock();
	makeCursorInView();
	update();
}
void EditView::convert_HanKanaToZenKana()
{
	convert_HanCharToZenChar(false);
}
static const wchar_t *toHankakuKanaTable[] = {
	L"ｧ", L"ｱ", L"ｨ", L"ｲ", L"ｩ", L"ｳ", L"ｪ",
	L"ｴ", L"ｫ", L"ｵ", L"ｶ", L"ｶﾞ", L"ｷ", L"ｷﾞ", L"ｸ", 
	L"ｸﾞ", L"ｹ", L"ｹﾞ", L"ｺ", L"ｺﾞ", L"ｻ", L"ｻﾞ", L"ｼ",
	L"ｼﾞ", L"ｽ", L"ｽﾞ", L"ｾ", L"ｾﾞ", L"ｿ", L"ｿﾞ", L"ﾀ", 
	L"ﾀﾞ", L"ﾁ", L"ﾁﾞ", L"ｯ", L"ﾂ", L"ﾂﾞ", L"ﾃ", L"ﾃﾞ",
	L"ﾄ", L"ﾄﾞ", L"ﾅ", L"ﾆ", L"ﾇ", L"ﾈ", L"ﾉ", L"ﾊ", 
	L"ﾊﾞ", L"ﾊﾟ", L"ﾋ", L"ﾋﾞ", L"ﾋﾟ", L"ﾌ", L"ﾌﾞ", L"ﾌﾟ",
	L"ﾍ", L"ﾍﾞ", L"ﾍﾟ", L"ﾎ", L"ﾎﾞ", L"ﾎﾟ", L"ﾏ", L"ﾐ", 
	L"ﾑ", L"ﾒ", L"ﾓ", L"ｬ", L"ﾔ", L"ｭ", L"ﾕ", L"ｮ",
	L"ﾖ", L"ﾗ", L"ﾘ", L"ﾙ", L"ﾚ", L"ﾛ", L"ヮ", L"ﾜ", 
	L"ヰ", L"ヱ", L"ｦ", L"ﾝ", L"ｳﾞ", L"ヵ", L"ヶ", 
};
ushort zenCharToHanChar(ushort code)
{
	if( code >= ZEN_EXMARK && code <= ZEN_TILDE )
		return code - ZEN_EXMARK + '!';
	else if( code == L'　' )
		return ' ';
	else
		return code;
}
void EditView::convert_ZenCharToHanChar(bool ansi)
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
		ushort code = charAt(first);
		ushort code2;
		if( ansi && (code2 = zenCharToHanChar(code)) != code ) {
			m_textCursor->setPosition(first);
			m_textCursor->setPosition(first+1, TextCursor::KEEP_ANCHOR);
			m_textCursor->insertText(QString(QChar(code2)));
			++first;
		} else if( code >= KANA_FIRST && code <= KANA_END ) {
			m_textCursor->setPosition(first);
			m_textCursor->setPosition(first+1, TextCursor::KEEP_ANCHOR);
			//int ix = code - L'ァ';
			QString txt((QChar *)toHankakuKanaTable[code - L'ァ']);
			m_textCursor->insertText(txt);
			first += txt.size();
		} else
			++first;
	}
	closeUndoBlock();
	makeCursorInView();
	update();
}
void EditView::convert_ZenKanaToHanKana()
{
	convert_ZenCharToHanChar(false);
}
