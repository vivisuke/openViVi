//----------------------------------------------------------------------
//
//			File:			"charEncoding.h"
//			Created:		03-Mar-2011
//			Author:			津田伸秀
//			Description:	文字コード判定関数宣言
//
//----------------------------------------------------------------------

/*

	Copyright (C) 2011 by Nobuhide Tsuda


*/

#pragma once

#ifndef		_HEADER_CHARENCODING_H
#define		_HEADER_CHARENCODING_H

#include <QString>
typedef unsigned char byte;
typedef const unsigned char cbyte;
typedef unsigned char uchar;
typedef const char cchar;
typedef const unsigned char cuchar;

#define		UTF8_BOM_LENGTH		3
#define		UTF16_BOM_LENGTH	2

extern uchar	UTF8_BOM[];
extern uchar	UTF16LE_BOM[];
extern uchar	UTF16BE_BOM[];

namespace CharEncoding {
	enum {
		UNKNOWN = 0,
		SJIS,
		EUC,
		UTF8,
		UTF16LE,
		UTF16BE,
	};
	enum {
		CRLF = 0,
		LF,
		CR,
	};
}

inline bool isHankakuKana(uchar uch)
{
	return uch >= 0xa1 && uch < 0xe0;
}
inline int isEUCcode(uchar uch)
{
	return uch >= 0xa1 && uch <= 0xfe;
}
bool isDBCSLeadByte(uchar ch);
bool isDBCSSecondByte(uchar ch);
uchar	checkCharEncoding(cuchar *, cuchar *, int &BOMLength);
bool	loadFile(const QString& pathName, QString& buf, QString& errMess, uchar& charEncoding, int& BOMLength);
//bool	loadFile(const QString &, QString &, QString &, uchar * = 0, bool * = 0);
//bool	getTextCodec(const QString &fileName, QString &errorString, QTextCodec *&codec,
//						int &bomLength, byte &);

byte checkNewLineCode(cbyte *ptr, cbyte *endptr, byte charCode);

#endif		//_HEADER_CHARENCODING_H
