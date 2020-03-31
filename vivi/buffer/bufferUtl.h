//----------------------------------------------------------------------
//
//			File:			"bufferUtl.h"
//			Created:		17-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_BUFFERUTL_H
#define		_HEADER_BUFFERUTL_H

#include <QString>
//#include <string>

QString getText(const Buffer &, int pos, int sz);
QString getLineText(const Buffer &, int ln);
bool startsWith(const Buffer&, int pos, const QString &);

#if		0
typedef const char cchar;
typedef const wchar_t cwchar;

class Buffer;

//	pos位置の括弧に対応する括弧位置を返す。無い場合は -1
int assocParenPosition(const Buffer &, int pos);
int assocParenPositionForward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen);
int assocParenPositionBackward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen);
#endif

#if 0
//	オートインデントテキスト取得
std::wstring autoIndentText(const Buffer &buffer,
							int pos,				//	現カーソル位置
							cwchar *newLineText,	//	改行文字列
							bool nxln = true,		//	次に行を挿入
							bool bCPP = false);
#endif


#endif		//_HEADER_BUFFERUTL_H
