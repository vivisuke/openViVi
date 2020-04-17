//----------------------------------------------------------------------
//
//			File:			"UTF16.cpp"
//			Created:		03-8-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#include "UTF16.h"

static uchar sbCharTypeTbl[] = {
/* 0 */	CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER,	CTSB_OTHER, CTSB_OTHER, CTSB_OTHER,
		CTSB_OTHER, CTSB_SPACE, CT_NEWLINE, CTSB_OTHER, CTSB_OTHER, CT_NEWLINE, CTSB_OTHER, CTSB_OTHER,
/* 1 */	CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER,
		CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER, CTSB_OTHER,
/* 2 */	CTSB_SPACE, CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,		//	  ! " # $ % & '
		CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,		//	( ) * + , - . /
/* 3 */	CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	0 CTSB_ALNUM 2 3 4 5 6 7
		CTSB_ALNUM, CTSB_ALNUM, CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,		//	8 9 : ; < = > ?
/* 4 */	CTSB_SYM,   CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	@ A B C D E F G
		CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	H I J K L M N O
/* 5 */	CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	P Q R S T U V W
		CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_ALNUM,		//	X Y Z [ \ ] ^ _
/* 6 */	CTSB_SYM,   CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	` a b c d e f g
		CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	h i j k l m n o
/* 7 */	CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM,		//	p q r s t u v w
		CTSB_ALNUM, CTSB_ALNUM, CTSB_ALNUM, CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,   CTSB_SYM,		//	x y z { | } ~

/* 8 */	CTSB_OTHER, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 9 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* a */	CTSB_OTHER, CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
		CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
/* b */	CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
		CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
/* c */	CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
		CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
/* d */	CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
		CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,  CTSB_KANA,
/* e */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* f */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, CTSB_OTHER, CTSB_OTHER,
};

byte UTF16CharType(wchar_t uch)
{
	if( uch < 0x80 )
		return sbCharTypeTbl[uch];
	if( uch >= 0x3300 && uch <= 0xabff ) return CTDB_KANJI;		//	äøéö
	if( uch >= 0x3041 && uch <= 0x3094 ) return CTDB_HIRA;		//	Ç–ÇÁÇ™Ç»
	if( uch >= 0x30a1 && uch <= 0x30fa || uch == 0x30fc )
		return CTDB_KANA;										//	ÉJÉ^ÉJÉiÇ‹ÇΩÇÕí∑âπ
	if( uch >= 0xff10 && uch <= 0xff19 ||						//	êîéö
		uch >= 0xff21 && uch <= 0xff3a ||						//	âpëÂï∂éö
		uch >= 0xff41 && uch <= 0xff5a || uch == 0xff3f )		//	âpè¨ï∂éö or ÉAÉìÉ_Å[ÉoÅ[
	{
		return CTDB_ALNUM;
	}
	if( uch == 0x309d || uch == 0x3093 || uch == 0x30fd || uch == 0x30fe)
		return CTDB_CONT;		//	åpë±ï∂éöÅiÅRÅSÅTÅUÅVÅWÅXÅj
	return CTDB_SYM;
}
