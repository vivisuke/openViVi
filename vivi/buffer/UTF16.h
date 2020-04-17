//----------------------------------------------------------------------
//
//			File:			"UTF16.h"
//			Created:		03-8-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_UTF16_H
#define		_HEADER_UTF16_H

typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short ushort;

enum {
	CT_EOF = 0,
	CT_NEWLINE,
	CTSB_SPACE,
	CTSB_ALNUM,
	CTSB_KANA,
	CTSB_SYM,
	CTSB_OTHER,
	CTDB_SPACE,
	CTDB_HIRA,
	CTDB_KANA,
	CTDB_ALNUM,
	CTDB_KANJI,
	CTDB_SYM,
	CTDB_CONT,
	CTDB_OTHER,
};
byte UTF16CharType(wchar_t);

#endif		//_HEADER_UTF16_H
