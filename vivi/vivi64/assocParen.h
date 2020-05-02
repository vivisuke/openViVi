//----------------------------------------------------------------------
//
//			File:			"assocParen.h"
//			Created:		20-10-2013
//			Author:			�Óc�L�G
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

//	pos�ʒu�̊��ʂɑΉ����銇�ʈʒu��Ԃ��B�����ꍇ�� -1
pos_t assocParenPosition(TypeSettings *, const Buffer &, pos_t pos, bool &bSharp);
pos_t assocParenPositionForward(TypeSettings *, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen);
pos_t assocParenPositionBackward(TypeSettings *, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen);


#endif		//_HEADER_ASSOCPAREN_H
