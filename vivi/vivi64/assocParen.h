//----------------------------------------------------------------------
//
//			File:			"assocParen.h"
//			Created:		20-10-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_ASSOCPAREN_H
#define		_HEADER_ASSOCPAREN_H

typedef const char cchar;
//typedef unsigned short wchar_t;
//typedef const wchar_t cwchar;

class Buffer;
class TypeSettings;

#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif

//	pos位置の括弧に対応する括弧位置を返す。無い場合は -1
pos_t assocParenPosition(TypeSettings *, const Buffer &, pos_t pos, bool &bSharp);
pos_t assocParenPositionForward(TypeSettings *, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen);
pos_t assocParenPositionBackward(TypeSettings *, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen);


#endif		//_HEADER_ASSOCPAREN_H
