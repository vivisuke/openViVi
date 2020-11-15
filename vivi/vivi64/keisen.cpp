//--------------------------------------------------------------------------
//
//			File:			"keisen.cpp"
//			Created:		14-May-2002
//			Author:			Nobuhide tsuda
//			Description:
//
//--------------------------------------------------------------------------

#include "mainWindow.h"
#include "editView.h"
#include "viewLineMgr.h"
#include "textCursor.h"
#include "document.h"

typedef unsigned char uint8;

#define		KT_CODE_BEG		0x2500
#define		KT_CODE_END		0x254b
#define		KT_FISRT_BYTE		((uchar)0x25)
#define		KT_SECOND_BYTE_BEG	((uchar)0x00)
#define		KT_SECOND_BYTE_END	((uchar)0x4b)

#define		KT_RIGHT_ARRAW_CODE		((ushort)L'→')
#define		KT_LEFT_ARRAW_CODE		((ushort)L'←')
#define		KT_UP_ARRAW_CODE		((ushort)L'↑')
#define		KT_DOWN_ARRAW_CODE		((ushort)L'↓')

//	方向は処理の都合で上から反時計周り

#define		KT_UP_MASK			0xc0
#define		KT_LEFT_MASK		0x30
#define		KT_DOWN_MASK		0x0c
#define		KT_RIGHT_MASK		0x03

//
//		ＳＪＩＳの各文字の上下左右のつながり状態を示すテーブル
//
//		各値は以下の定数の組み合わせ
//
#define		KT_UP_THICK			0x80
#define		KT_UP_THIN			0x40
#define		KT_LEFT_THICK		0x20
#define		KT_LEFT_THIN		0x10
#define		KT_DOWN_THICK		0x08
#define		KT_DOWN_THIN		0x04
#define		KT_RIGHT_THICK		0x02
#define		KT_RIGHT_THIN		0x01

#define		KT_STRING			0

#define		KEISEN_HIGHBYTE			0x25
#define		KEISEN_LOWBYTE_FIRST	0x00
#define		KEISEN_LOWBYTE_LAST		0x4b

uchar keisenTable[] = {
	/*	U+2500	─	*/	KT_LEFT_THIN | KT_RIGHT_THIN,
	/*	U+2501	━	*/	KT_LEFT_THICK | KT_RIGHT_THICK,		//	
	/*	U+2502	│	*/	KT_UP_THIN | KT_DOWN_THIN,
	/*	U+2503	┃	*/	KT_UP_THICK | KT_DOWN_THICK,
	0, 0, 0, 0, 0, 0, 0, 0,
	/*	U+250c	┌	*/	KT_DOWN_THIN | KT_RIGHT_THIN,
	0, 0,
	/*	U+250f	┏	*/	KT_DOWN_THICK | KT_RIGHT_THICK,
	/*	U+2510	┐	*/	KT_LEFT_THIN | KT_DOWN_THIN,
	0, 0, 
	/*	U+2513	┓	*/	KT_LEFT_THICK | KT_DOWN_THICK,
	/*	U+2514	└	*/	KT_UP_THIN | KT_RIGHT_THIN,
	0, 0,
	/*	U+2517	┗	*/	KT_UP_THICK | KT_RIGHT_THICK,
	/*	U+2518	┘	*/	KT_LEFT_THIN | KT_UP_THIN,
	0, 0,
	/*	U+251b	┛	*/	KT_LEFT_THICK | KT_UP_THICK,
	/*	U+251c	├	*/	KT_UP_THIN | KT_DOWN_THIN | KT_RIGHT_THIN,
	/*	U+251d	┝	*/	KT_UP_THIN | KT_DOWN_THIN | KT_RIGHT_THICK,
	0, 0, 
	/*	U+2520	┠	*/	KT_UP_THICK | KT_DOWN_THICK | KT_RIGHT_THIN,
	0, 0, 
	/*	U+2523	┣	*/	KT_UP_THICK | KT_DOWN_THICK | KT_RIGHT_THICK,
	/*	U+2524	┤	*/	KT_UP_THIN | KT_DOWN_THIN | KT_LEFT_THIN,
	/*	U+2525	┥	*/	KT_UP_THIN | KT_DOWN_THIN | KT_LEFT_THICK,
	0, 0, 
	/*	U+2528	┨	*/	KT_UP_THICK | KT_DOWN_THICK | KT_LEFT_THIN,
	0, 0,
	/*	U+252b	┫	*/	KT_UP_THICK | KT_DOWN_THICK | KT_LEFT_THICK,
	/*	U+252c	┬	*/	KT_DOWN_THIN | KT_LEFT_THIN | KT_RIGHT_THIN,
	0, 0,
	/*	U+252f	┯	*/	KT_LEFT_THICK | KT_RIGHT_THICK | KT_DOWN_THIN,
	/*	U+2530	┰	*/	KT_LEFT_THIN | KT_RIGHT_THIN | KT_DOWN_THICK,
	0, 0,
	/*	U+2533	┳	*/	KT_DOWN_THICK | KT_LEFT_THICK | KT_RIGHT_THICK,
	/*	U+2534	┴	*/	KT_UP_THIN | KT_LEFT_THIN | KT_RIGHT_THIN,
	0, 0,
	/*	U+2537	┷	*/	KT_LEFT_THICK | KT_RIGHT_THICK | KT_UP_THIN,
	/*	U+2538	┸	*/	KT_LEFT_THIN | KT_RIGHT_THIN | KT_UP_THICK,
	0, 0,
	/*	U+253b	┻	*/	KT_UP_THICK | KT_LEFT_THICK | KT_RIGHT_THICK,
	/*	U+253c	┼	*/	KT_UP_THIN | KT_DOWN_THIN | KT_LEFT_THIN | KT_RIGHT_THIN,
	0, 0,
	/*	U+253f	┿	*/	KT_LEFT_THICK | KT_RIGHT_THICK | KT_UP_THIN | KT_DOWN_THIN,
	0, 0,
	/*	U+2542	╂	*/	KT_UP_THICK | KT_DOWN_THICK | KT_LEFT_THIN | KT_RIGHT_THIN,
	0, 0, 0, 0, 0, 0, 0, 0,
	/*	U+254b	╋	*/	KT_UP_THICK | KT_DOWN_THICK | KT_LEFT_THICK | KT_RIGHT_THICK,
};

static void toKeisenString(QString &str, uchar state, bool hankakuSpace)
{
	if( !state ) {
		str = hankakuSpace ? "  " : "　";
		return;
	}
	for(int i=0;i<=KT_CODE_END-KT_CODE_BEG;++i) {
		if( keisenTable[i] == state ) {
			str = QString((wchar_t)(i + KT_CODE_BEG));
			return;
		}
	}
	str = "??";
}

//
//		現在の状態から罫線を引いた状態を計算する
//
//		state:		現在の罫線状態
//		dir:		罫線を引く方向（KT_XXX_MASK）
//		type:		KT_XXX_THIN または KT_XXX_THICK
//
static void toKeisenString(QString &str, uchar state,
								uchar dir, uchar type,
								bool destArraw, bool hankakuSpace)
{
	uchar opdir, optype;		//	反対側のマスクと線種
	uchar dir2, type2;			//	dir から９０度ずらしたマスクと線種
	uchar dir3, type3;			//	dir から－９０度ずらしたマスクと線種
	opdir = (dir >> 4) | (dir << 4);
	optype = (type >> 4) | (type << 4);
	dir2 = (dir >> 2) | (dir << 6);
	dir3 = (dir >> 6) | (dir << 2);
	type2 = state & dir2;
	type3 = state & dir3;
	type3 = (type3<<4) | (type3>>4);				//	比較のために180度回転
	if( !type2 && !type3 ) {						//	直角方向に罫線が無い場合
		if( !(state & opdir) &&						//	３方向に罫線なし
			destArraw )								//	罫線矢印モード
		{
			switch( dir ) {
#if	0
			case KT_LEFT_MASK:		str = QChar(L'→');	break;	//	→
			case KT_RIGHT_MASK:		str = QChar(L'←');	break;	//	←
			case KT_DOWN_MASK:		str = QChar(L'↑');	break;	//	↑
			case KT_UP_MASK:		str = QChar(L'↓');	break;	//	↓
#else
			case KT_LEFT_MASK:		str = QString((const QChar*)L"→"); break;
			case KT_RIGHT_MASK:		str = QString((const QChar*)L"←"); break;
			case KT_DOWN_MASK:		str = QString((const QChar*)L"↑"); break;
			case KT_UP_MASK:		str = QString((const QChar*)L"↓"); break;
#endif
			}
			return;
		}
		state = (state & (uchar)~(dir|opdir)) | type | optype;
	} else {
		state = (state & (uchar)~dir) | type;
		if( type != 0 ) {							//	罫線を引く場合
			if( state & opdir ) {					//	反対方向に罫線が存在する場合
				state = (state & (uchar)~opdir) | optype;
				if( type2 && type3 && type2 != type3 )
					state = (state & (uchar)~(dir2|dir3)) |
									(type>>2) | (type<<6) | (type>>6) | (type<<2);
			} else {
				if( type2 != type3 ) {				//	直角方向の罫線が異なる場合
					if( type2 )
						state = (state & (uchar)~dir2) | (type>>2) | (type<<6);
					if( type3 )
						state = (state & (uchar)~dir3) | (type>>6) | (type<<2);
				}
			}
		} else {									//	罫線を消す場合
			if( type2 && type3 ) {					//	両方の直角方向に罫線がある
				if( type2 != type3 )				//	通常はありえない
					state = (state & (uchar)~dir2) | type3;
			} else {
				if( (optype = (state & opdir)) != 0 ) {	//	逆方向に罫線がある
					if( type2 )
						state = (state & (uchar)~dir2) | (optype>>6) | (optype<<2);
					if( type3 )
						state = (state & (uchar)~dir3) | (optype>>2) | (optype<<6);
				} else {
					if( type3 )
						state = (state & (uchar)~dir2) | type3;
					else
						state = (state & (uchar)~dir3) | (type2<<4) | (type2>>4);
				}
			}
		}
	}
	toKeisenString(str, state, hankakuSpace);
}
void EditView::drawKeisen(int key, bool erase)		//	罫線モードで罫線を引く
{
	//##clearSelectMode();
	switch( key ) {
	case Qt::Key_Up:
		drawKeisenUp(erase);
		break;
	case Qt::Key_Down:
		drawKeisenDown(erase);
		break;
	case Qt::Key_Left:
		drawKeisenLeft(erase);
		break;
	case Qt::Key_Right:
		drawKeisenRight(erase);
		break;
	}
}
void EditView::drawKeisenLeft(bool erase)			//	罫線モードで罫線を引く
{
	int length = 0, eolOffset = 0 /*, indent*/;
#if	0	//##
	ctchar *ptr = NULL;
	if( getViewLineMgr()->getLineText(getViewCursor()->getLine(), ptr, length) )
		eolOffset = getEOLOffset(ptr, length);
	if( getViewCursor()->getColumn() /*- indent*/ < 2 )
		return;
#endif
	QString kstr1, kstr2;
	//{
		uchar type = 0;
		if( !erase ) {
			switch( mainWindow()->keisenType() ) {
			case KEISEN_THIN:		type = KT_LEFT_THIN;	break;
			case KEISEN_THICK:		type = KT_LEFT_THICK;	break;
			//case KEISEN_HANKAKU:	type = KT_LEFT_THIN;	break;
			}
		}
		uchar state, up, down, left, right;
		getAroundKeisenState(state, up, down, left, right);
#if	0	//##
		toKeisenString(kstr2, state, KT_LEFT_MASK, type, 0, theApp.m_padHankakuSpc);
		toKeisenString(kstr1, left, KT_RIGHT_MASK, type>>4,
							!erase && true /*m_option->isValid(VWOPT_KEISEN_ARRAW) ? 1 : 0*/,
							theApp.m_padHankakuSpc);
		if( !erase || getViewCursor()->getOffset() < eolOffset )		//	改行位置以外の場合
			kstr1 += kstr2;
	//}
	int c1 = getViewCursor()->getColumn() - 2;
	int line = getViewCursor()->getLine();
	int offset1 = getViewLineMgr()->columnToOffset(line, ptr, ptr+eolOffset, c1);
	int offset = offset1;
	if( c1 < getViewCursor()->getColumn() - 2 ) {
		if( theApp.m_padHankakuSpc ) {
			while( (c1 += 1) <= getViewCursor()->getColumn() - 2 ) {
				kstr1 = _T(" ") + kstr1;		//	半角空白
				offset += 1;
			}
		} else {
			if( (getViewCursor()->getColumn() - c1) & 1 ) {
				kstr1 = _T(" ") + kstr1;
				offset += 1;
			}
			while( (c1 += 2) <= getViewCursor()->getColumn() - 2 ) {
				kstr1 = _T("　") + kstr1;		//	全角空白
				offset += 2;
			}
		}
	}
	STextPos pos1(getViewCursor()->getLine(), offset1);
	int c2 = getViewCursor()->getColumn() + 2;
	int offset2 = getViewLineMgr()->columnToOffset(line, ptr, ptr+eolOffset, c2);
	if( c2 < getViewCursor()->getColumn() + 2 && ptr[offset2] != '\t' && getViewCursor()->getOffset() < eolOffset )
		kstr1 += " ";
	STextPos pos2(getViewCursor()->getLine(), offset2);
	doReplaceTextNoKeisenProtect(pos1, pos2, kstr1);
	invalidateCursor();
	getViewCursor()->m_line = line;
	getViewCursor()->m_offset = offset;
	getViewCursor()->updateColumn();
	makeCursorInView();
	invalidateCursor();
#endif
}
void EditView::drawKeisenRight(bool erase)		//	罫線モードで罫線を引く
{
	uchar type;
	if( erase )
		type = 0;
	else {
		switch( mainWindow()->keisenType() ) {
		case KEISEN_THIN:		type = KT_RIGHT_THIN;	break;
		case KEISEN_THICK:		type = KT_RIGHT_THICK;	break;
		//case KEISEN_HANKAKU:	type = KT_RIGHT_THIN;	break;
		}
	}
	uchar state, up, down, left, right;
	getAroundKeisenState(state, up, down, left, right);
	QString kstr, kstr2;
	toKeisenString(kstr, state, KT_RIGHT_MASK, type, false, true /*theApp.m_padHankakuSpc*/);
	toKeisenString(kstr2, right, KT_LEFT_MASK, type<<4,
				!erase && true /*m_option->isValid(VWOPT_KEISEN_ARRAW) ? 1 : 0*/,
				true /*theApp.m_padHankakuSpc*/);
	const bool atNewLine = textCursor()->charAt() == L'\n' || textCursor()->charAt() == L'\r';
	if( !erase || !atNewLine )		//	改行位置以外の場合
		kstr += kstr2;
	if( !atNewLine ) {	//	改行位置でない場合
		textCursor()->movePosition(TextCursor::RIGHT, TextCursor::KEEP_ANCHOR);		//	暫定コード
	}
	textCursor()->insertText(kstr);
	textCursor()->movePosition(TextCursor::LEFT);

}
void EditView::drawKeisenUp(bool erase, bool bUndoBlock)			//	罫線モードで罫線を引く
{
}
void EditView::drawKeisenDown(bool erase, bool)			//	罫線モードで罫線を引く
{
}
void EditView::drawKeisenNextPrevLine(QString&, int)
{
}
//		キャレット位置、その周りの罫線の状態を返す
void EditView::getAroundKeisenState(uchar &state,
									 uchar &up, uchar &down,
									 uchar &left, uchar &right)
{
	state = up = down = left = right = 0;
	int offset, c2;
	ushort code;
	int length = 0/*, indent*/;
	pos_t pos = m_textCursor->position();
	int line = document()->positionToLine(pos);
	pos_t start = lineStartPosition(line);	//	行先頭位置
	//const tchar *ptr = NULL;
	//int line = getViewCursor()->getLine();
	if( line > 0 ) {		//	ひとつ上の状態
		//VERIFY( getViewLineMgr()->getLineText(getViewCursor()->getLine()-1, ptr, length) );
		//##c2 = getViewCursor()->getColumn();
		//##offset = pos - start;
		//##if( c2 == getViewCursor()->getColumn() ) {
		TextCursor cur(*textCursor());
		cur.movePosition(TextCursor::UP);
			code = buffer()->charAt(cur.position());
			if( code >= KT_CODE_BEG && code <= KT_CODE_END )
				state = ((up = keisenTable[code - KT_CODE_BEG]) & KT_DOWN_MASK) << 4;
		//##}
	}
	if( line != viewLineMgr()->EOFLine() ) {
		TextCursor cur(*textCursor());
		cur.movePosition(TextCursor::DOWN);
			code = buffer()->charAt(cur.position());
			if( code >= KT_CODE_BEG && code <= KT_CODE_END )
				state = ((down = keisenTable[code - KT_CODE_BEG]) & KT_DOWN_MASK) << 4;
	}
#if	0	//##
	if( getViewCursor()->getLine() < getViewLineMgr()->getLineCount() ) {		//	ひとつ下の状態
		VERIFY( getViewLineMgr()->getLineText(getViewCursor()->getLine()+1, ptr, length) );
		c2 = getViewCursor()->getColumn();
		offset = getViewLineMgr()->columnToOffset(line, ptr, ptr+length, c2);
#ifdef	_UNICODE
		if( c2 == getViewCursor()->getColumn() ) {
			code = ptr[offset];
			if( code >= KT_CODE_BEG && code <= KT_CODE_END )
				state |= ((down = keisenTable[code - KT_CODE_BEG]) & KT_UP_MASK) >> 4;
		}
#else
		if( c2 == getViewCursor()->getColumn() && isDBCSLeadByte(ptr[offset]) ) {
			code = ((uchar)ptr[offset] << 8) + (uchar)ptr[offset+1];
			if( code >= KT_CODE_BEG && code <= KT_CODE_END )
				state |= ((down = keisenTable[code - KT_CODE_BEG]) & KT_UP_MASK) >> 4;
		}
#endif
	}
	if( !getViewLineMgr()->getLineText(getViewCursor()->getLine(), ptr, length /*, indent*/) ) return;
#ifdef	_UNICODE
	if( getViewCursor()->getOffset() > 0 ) {
		code = ptr[getViewCursor()->getOffset()-1];
		if( code >= KT_CODE_BEG && code <= KT_CODE_END )
			state |= ((left = keisenTable[code - KT_CODE_BEG]) & KT_RIGHT_MASK) << 4;
	}
#else
	if( getViewCursor()->getOffset() >= 2 && getDBCharType(ptr, getViewCursor()->getOffset()-2) == CT_DOUBLEBYTE1 ) {
		code = ((uchar)ptr[getViewCursor()->getOffset()-2] << 8) + (uchar)ptr[getViewCursor()->getOffset()-1];
		if( code >= KT_CODE_BEG && code <= KT_CODE_END )
			state |= ((left = keisenTable[code - KT_CODE_BEG]) & KT_RIGHT_MASK) << 4;
	}
#endif
#ifdef	_UNICODE
	if( getViewCursor()->getOffset() + 1 < length ) {
		code = ptr[getViewCursor()->getOffset()+1];
		if( code >= KT_CODE_BEG && code <= KT_CODE_END )
			state |= ((right = keisenTable[code - KT_CODE_BEG]) & KT_LEFT_MASK) >> 4;
	}
#else
	if( getViewCursor()->getOffset() <= length - 4 && getDBCharType(ptr, getViewCursor()->getOffset()+2) == CT_DOUBLEBYTE1 ) {
		code = ((uchar)ptr[getViewCursor()->getOffset()+2] << 8) + (uchar)ptr[getViewCursor()->getOffset()+3];
		if( code >= KT_CODE_BEG && code <= KT_CODE_END )
			state |= ((right = keisenTable[code - KT_CODE_BEG]) & KT_LEFT_MASK) >> 4;
	}
#endif
#endif
}

///////
