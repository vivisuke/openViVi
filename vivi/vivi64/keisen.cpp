//--------------------------------------------------------------------------
//
//			File:			"keisen.cpp"
//			Created:		14-May-2002
//			Author:			Nobuhide tsuda
//			Description:
//
//--------------------------------------------------------------------------

#include "editView.h"

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



