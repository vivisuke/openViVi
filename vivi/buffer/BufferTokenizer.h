//----------------------------------------------------------------------
//
//			File:			"BufferTokenizer.h"
//			Created:		29-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_BUFFERTOKENIZER_H
#define		_HEADER_BUFFERTOKENIZER_H

typedef unsigned char byte;

class Buffer;

class BufferTokenizer
{
public:
	enum {
		UNDEF = 0,	//	初期値
		IDENT,		//	英字で始まる英数字列
		NUMBER,		//	数字で始まる英数字列
		STRING,		//	文字列
		SYMBOL,		//	その他記号
		END_OF_FILE,
	};
public:
	BufferTokenizer(const Buffer &, int, int);
	
private:
	const Buffer	&m_buffer;
	int		m_pos;
	int		m_last;
	bool	m_pushed;
	bool	m_firstToken;	//	行内の最初のトークン
	int		m_lineNum;		//	現在行番号 (1..*)
	byte	m_tokenType;	//	トークンタイプ
	int		m_tokenFirst;
	int		m_tokenSize;
	int		m_prevTokenFirst;
	int		m_prevTokenSize;
	//QString	m_tokenText;	//	現在トークン文字列
	//QString	m_prevText;		//	ひとつ前のトークン文字列
	int		m_tokenLineNum;
	int		m_tokenPosition;	//	トークン開始オフセット
};

#endif		//_HEADER_BUFFERTOKENIZER_H
