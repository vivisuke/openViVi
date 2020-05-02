//----------------------------------------------------------------------
//
//			File:			"autoIndent.cpp"
//			Created:		18-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include <QDebug>
#include <QString>
#include "tokenizer.h"
#include "typeSettings.h"
#include "assocParen.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"

typedef const wchar_t cwchar;

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}

bool isCppIndentWord(const QString &text)
{
	return /*text == "case" ||*/ text == "do" || text == "else" || text == "if"
					|| text == "for" || text == "switch" || text == "while";
}
#if	0
int getIndent(const Buffer &buffer, pos_t ls)
{
	pos_t pos = ls;
	while( isSpaceChar(buffer[pos]) ) ++pos;
	return pos - ls;
}
#endif
QString getLastToken(TypeSettings *typeSettings, const Buffer &buffer,
									pos_t pos0, pos_t last, pos_t &lastTokenPos)
{
	pos_t pos = pos0;
	while( pos < last && isSpaceChar(buffer[pos]) ) ++pos;
	const int indent = pos - pos0;
	Tokenizer dt(buffer, pos, last);
	QString lastToken = dt.tokenText();
	lastTokenPos = pos;
	bool inBlockComment = false;
	for(;;) {
		if( dt.nextToken() == Tokenizer::END_OF_FILE ) break;
		QString token = dt.tokenText();
		if( token.isEmpty() ) break;
		if( !inBlockComment ) {
			if( token == typeSettings->textValue(TypeSettings::LINE_COMMENT) ) break;
			if( token != typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG) ) {
				lastToken = token;
				lastTokenPos = dt.tokenPosition();
			} else
				inBlockComment = true;
		} else {
			if( token == typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END) )
				inBlockComment = false;
		}
	}
	return lastToken;
}
int getFirstLastToken(TypeSettings *typeSettings, const Buffer &buffer, pos_t pos0, pos_t last,
						QString &firstToken, QString &lastToken, QString &last2Token,
						int &nToken, int &last2Index)
{
	pos_t pos = pos0;
	while( pos < last && isSpaceChar(buffer[pos]) ) ++pos;
	const int indent = pos - pos0;
	Tokenizer dt(buffer, pos, last);
	last2Token.clear();
	firstToken = lastToken = dt.tokenText();
	//qDebug() << firstToken;
	//const int indent = dt.tokenPosition() - pos;
	//int type = dt.tokenType();
	bool inBlockComment = false;
	nToken = 1;
	int lastIndex = -1;
	for(;;) {
		if( dt.nextToken() == Tokenizer::END_OF_FILE ) break;
		QString token = dt.tokenText();
		if( token.isEmpty() ) break;
		if( firstToken.isEmpty() || firstToken == "}" )		//	行頭の } は無視
			firstToken = token;
		if( !inBlockComment ) {
			if( token == typeSettings->textValue(TypeSettings::LINE_COMMENT) ) break;
			if( token != typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG) ) {
				last2Token = lastToken;
				lastToken = token;
				last2Index = lastIndex;
				lastIndex = dt.tokenPosition();
				++nToken;
			} else
				inBlockComment = true;
		} else {
			if( token == typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END) )
				inBlockComment = false;
		}
	}
	return indent;
}
QString pythonAutoIndect(TypeSettings *typeSettings,
							QString text, 
							const Buffer &buffer,
							pos_t pos0)				//	カーソル位置
{
	int ln = buffer.positionToLine(pos0);
	pos_t ls = buffer.lineStartPosition(ln);
	QString firstToken;		//	first token
	QString lastToken;
	QString last2Token;
	int nToken, last2Index;
	int indent = getFirstLastToken(typeSettings, buffer, ls, pos0, firstToken, lastToken, last2Token, nToken, last2Index);
	if( lastToken == ":" )
		text += "\t";
	return text;
}
QString autoIndentText(TypeSettings *typeSettings,
							const Buffer &buffer,
							pos_t pos0,				//	カーソル位置
							cwchar *newLineText,		//	改行文字列
							bool nxln /*,				//	次に行を挿入
							bool bCPP*/)
{
	const bool bCPP = typeSettings->name() == "CPP"
						|| typeSettings->name() == "C#"
						|| typeSettings->name() == "JAVA"
						|| typeSettings->name() == "PHP";
	//delPrevSpace = false;
	QString text0((const QChar *)newLineText);
	QString text(text0);
	const int ln0 = buffer.positionToLine(pos0);		//	現在行番号
	int ln = ln0;
	pos_t ls = buffer.lineStartPosition(ln);
	int offset = pos0 - ls;	//	行頭からのオフセット
	pos_t nxls = buffer.lineStartPosition(ln + 1);
	QString firstToken;		//	first token
	QString lastToken;
	QString last2Token;
	int nToken, last2Index;
#if		1
	int indent = getFirstLastToken(typeSettings, buffer, ls, nxls, firstToken, lastToken, last2Token, nToken, last2Index);
	if( bCPP ) {
		if( last2Token == ")" && lastToken == ";" ) {
			//	undone:	関数コールの引数で改行し、); で終わっている場合は、対応する ( 行を参照行とする
			int apos = assocParenPositionBackward(typeSettings, buffer, last2Index, ')', '(');
			if( apos >= 0 ) {
				ln = buffer.positionToLine(apos);
				ls = buffer.lineStartPosition(ln);
				nxls = buffer.lineStartPosition(ln + 1);
				indent = getFirstLastToken(typeSettings, buffer, ls, nxls, firstToken, lastToken, last2Token, nToken, last2Index);
			}
		}
		while( ln > 0 && firstToken != "case" && lastToken == ":" && nToken <= 3 ) {		//	IDENT {slots}: の場合はスキップ
			nxls = ls;
			ls = buffer.lineStartPosition(--ln);
			offset = nxls - ls;
			indent = getFirstLastToken(typeSettings, buffer, ls, nxls, firstToken, lastToken, last2Token, nToken, last2Index);
		}
	}
	if( offset >= indent )
		text += getText(buffer, ls, indent);
#else
	pos_t pos = ls;
	if( offset != 0 ) {
		QString it;
		wchar_t ch;
		while( pos < buffer.size() && isSpaceChar(ch = buffer.charAt(pos++)) ) {
			it += QChar(ch);
		}
		if( it.size() <= offset )
			text += it;
	}
#endif
	if( typeSettings->name() == "PYTHON" ) {
		return pythonAutoIndect(typeSettings, text, buffer, pos0);
	}
	if( !bCPP ) return text;
	if( buffer[pos0] == '}' ) {	//	カーソル位置が } の場合
		int apos = assocParenPositionBackward(typeSettings, buffer, pos0, '}', '{');
		if( apos >= 0 ) {		//	対応する括弧がある
			text = text0;
			int ln = buffer.positionToLine(apos);
			pos_t ls = buffer.lineStartPosition(ln);
			int i = ls;
			while( isSpaceChar(buffer[i]) ) ++i;
			if( i > ls )
				text += getText(buffer, ls, i - ls);
			return text;
		}
	}
	//--pos;
	indent = getFirstLastToken(typeSettings, buffer, ls, pos0 /*nxls*/, firstToken, lastToken, last2Token, nToken, last2Index);

	if( lastToken == "{" ) {
		text += "\t";
	} else if( lastToken == ";" ) {
		//	if( ) hoge; の場合
		if( ln > 0 ) {
			int prls = buffer.lineStartPosition(ln - 1);
			pos_t ls = buffer.lineStartPosition(ln);
			QString firstToken, lastToken;
			indent = getFirstLastToken(typeSettings, buffer, prls, ls, firstToken, lastToken, last2Token, nToken, last2Index);
			if( lastToken != "{" && lastToken != ";" && isCppIndentWord(firstToken) ) {
				// if( ) { 改行でなく、
				//	if( ) 改行 hoge; の場合は、if 行のインデントに合わせる
				text = text0 + getText(buffer, prls, indent);
			}
		}
#if		1
	} else if( lastToken == "}" ) {
		//	if( ) { ... } の場合
		int apos = assocParenPositionBackward(typeSettings, buffer, last2Index, '}', '{');
		if( apos >= 0 && buffer.positionToLine(apos) == ln && ln > 0 )
		{
			int prls = buffer.lineStartPosition(ln - 1);
			pos_t ls = buffer.lineStartPosition(ln);
			QString firstToken, lastToken;
			indent = getFirstLastToken(typeSettings, buffer, prls, ls, firstToken, lastToken, last2Token, nToken, last2Index);
			if( lastToken != "{" && isCppIndentWord(firstToken) ) {
				text = text0 + getText(buffer, prls, indent);
			}
		}
#endif
	} else {	//	最後がセミコロン（;）でない場合
		wchar_t nxChar = 0;		//	カーソル位置文字
		for(pos_t pos = pos0; isSpaceChar(nxChar = buffer[pos]); ++pos) {}
		if( offset >= indent + firstToken.size()
			&& (isCppIndentWord(firstToken) || firstToken == "case")
			&& nxChar != '{' )		//	{ 直前で改行した場合
		{
			text += "\t";
		}
	}
	return text;
}
