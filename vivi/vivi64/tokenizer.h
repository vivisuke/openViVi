//----------------------------------------------------------------------
//
//			File:			"tokenizer.h"
//			Created:		17-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_TOKENIZER_H
#define		_HEADER_TOKENIZER_H

typedef unsigned char uchar;
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号

#include <QString>

class Buffer;
class TypeSettings;

class Tokenizer
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
	Tokenizer(const Buffer &, pos_t pos, pos_t last, bool = true, const TypeSettings* = 0);
	Tokenizer(const Buffer *, pos_t pos, pos_t last, bool = true, const TypeSettings* = 0);
	Tokenizer(const Tokenizer &x);
	~Tokenizer(void);

public:
	int		tokenType() const { return m_tokenType; }
	QString	tokenText() const { return m_tokenText; }
	QString	prevTokenText() const { return m_prevText; }
	int		tokenLineNum() const { return m_tokenLineNum; }
	int		tokenPosition() const { return m_tokenPosition; }
	int		nextPosition() const { return m_pos; }
	wchar_t	nextChar() const;
	wchar_t	nextNextChar() const;
	bool	isFirstToken() const { return m_firstToken; }
	int		lineNumber() const { return m_lineNum; };

public:
	QChar	skipSpace();		//	空白類をスキップし、その次の文字を返す
	QChar	skipSpaceOrNewLine();		//	空白類をスキップし、その次の文字を返す
	int		nextToken();
	//QChar	nextChar();
	QString	nextTokenText() { nextToken(); return tokenText(); }
	void		skipLine();					//	次の行の最初のトークンまでスキップ
	int		nextLineToken();			//	次の行の最初のトークンまで読み進める
	//uchar	getNextChar();
	void	ungetToken() { m_pushed = true; };
	void	setPosition(pos_t pos) { m_pos = pos; }
	void	setLast(pos_t last) { m_last = last; }

protected:
	void	init();

private:
	bool	m_bString;		//	".."、'...' 文字列有効
	bool	m_pushed;
	bool	m_firstToken;	//	行内の最初のトークン
	int		m_lineNum;		//	現在行番号 (1..*)
	pos_t	m_pos;
	pos_t	m_last;
	const Buffer	&m_buffer;
	uchar	m_tokenType;	//	トークンタイプ
	QString	m_tokenText;	//	現在トークン文字列
	QString	m_prevText;		//	ひとつ前のトークン文字列
	int		m_tokenLineNum;
	int		m_tokenPosition;	//	トークン開始オフセット
	const TypeSettings	*m_typeSettings;
	QString		m_lineComment;		//	行コメント文字
};

#endif		//_HEADER_TOKENIZER_H
