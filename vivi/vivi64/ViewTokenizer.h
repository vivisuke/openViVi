#pragma once

#include <QString>

typedef unsigned char byte;

class Buffer;
class TypeSettings;

class ViewTokenizer
{
public:
	enum {
		UNKNOWN = 0,
		STRING,
		ALNUM,		//	英字で始まる英数字列
		DIGITS,		//	10進数
		QUOTED,		//	シングル、ダブルクォートされた文字列
		SYMBOL,		//	!"# などの記号類（半角空白も含む）
		ZEN_SPACE,	//	全角空白
		HTMLTAG,	//	<...> 内テキスト
		COMMENT,	//	コメント
		CTRL,		//	0x20 未満
		NEWLINE,	//	CR/LF/CRLF
		URL,
		HTML_SPECIAL_CHARS,		//	&lt; 等のHTML特殊文字
		OTHER,		//	その他（マルチバイト文字など）
	};
public:
	ViewTokenizer(const TypeSettings *typeSettings, const Buffer *buffer, int first, int sz, int last);
	~ViewTokenizer() {}

public:
	int		ix() const { return m_ix; }
	int		tokenix() const { return m_tokenix; }
	byte	tokenType() const { return m_tokenType; }
	bool	isInLineComment() const { return m_inLineComment; }
	bool	isInBlockComment() const { return m_inBlockComment; }
	//bool	isInScript() const { return m_inScript; }
	bool	isInTag() const { return m_inHTMLTag; }
	bool	isInPHP() const { return m_inPHP; }
	bool	isTokenInComment() const { return m_isTokenInComment; }
	//bool	isTokenInString() const { return m_inString; }
	bool	isInHTMLTag() const { return m_inHTMLTag; }
	bool	isTokenInHTMLTag() const { return m_isTokenInHTMLTag; }
	bool	isTokenNewLine() const { return m_isTokenNewLine; }
	bool	isInScriptStartEnd() const { return m_inScriptStartEnd; }
	QString	tagText() const { return m_tagText; }
	QString quotedText() const { return m_quotedText; }
	QString fullText() const { return m_fullText; }
	QString orgText() const { return m_orgText; }

public:
	void	setQuoteChar(char q) { m_quoteChar = q; }
	void	setInLineComment(bool b) { m_inLineComment = b; }
	void	setInBlockComment(bool b) { m_inBlockComment = b; }
	//void	setInString(bool b) { m_inString = b; }
	//void	setInScrpt(bool b) { m_inScript = b; }
	void	setInTag(bool b) { m_inHTMLTag= b; }
	void	setInPHP(bool b) { m_inPHP = b; }
	void	setInScriptStartEnd(bool b) { m_inScriptStartEnd = b; }
	void	setBlockCommentText(const QString &, const QString &);
	void	setLineCommentText(const QString &);
	void	setQuotedText(const QString &);
	QString	nextToken(/*int &type*/);
	void	setCursorLine() { m_cursorLine = true; }

private:
	QString	m_tokenText;
	QString m_fullText;		//	カーソル位置以降も含めたテキスト
	int		m_ix;		//	次の文字
	int		m_tokenix;	//	トークン最初の文字のバッファ内文字位置
	int		m_quotedTextix;
	int		m_lastBuffer;
	int		m_lastBuffer2;			//	ホントのバッファ末尾
	byte	m_tokenType;				//	トークン種別
	bool	m_inLineComment;
	bool	m_inBlockComment;
	//bool	m_inScript;
	bool	m_inPHP;
	bool	m_isTokenInComment;
	//bool	m_inString;
	bool	m_inHTMLTag;						//	"<" ... ">" の中
	int		m_htmlTagLvl;						//	"<" のネスト数
	bool	m_inScriptStartEnd;				//	"<script> ... </script> の中
	bool	m_isTokenInHTMLTag;
	bool	m_isTokenNewLine;
	bool	m_startTag;								//	true for "<", false for "</"
	bool	m_cursorLine;
	char	m_quoteChar;							//	ダブル・シングルクォート中の場合に有効
	const TypeSettings	*m_typeSettings;
	const Buffer	*m_buffer;
	QString	m_orgText;							//	&lt; 等のHTML特殊文字
	QString	m_tagText;							//	"<" タグテキスト
	QString	m_quotedText;						//	文字列テキスト
	QString	m_lineCommentText;
	QString	m_begBlockCommentText;
	QString	m_endBlockCommentText;
};

