//----------------------------------------------------------------------
//
//			File:			"tokenizer.cpp"
//			Created:		17-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include "tokenizer.h"
#include "typeSettings.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isSpaceOrNewLineChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}
inline bool isNewLineChar(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
inline bool isNumber(wchar_t ch)
{
	return ch >= '0' && ch <= '9';
}
inline bool isLetterOrNumberOrUnderbar(const QChar &ch)
{
	return ch.isLetterOrNumber() || ch == QChar('_');
}
inline bool isLetterOrNumber(const QChar &ch)
{
	return ch.isLetterOrNumber();
}
#if 0
QString getText(const Buffer &buffer, int pos, int sz)
{
	QString text;
	while( --sz >= 0 )
		text += QChar(buffer.charAt(pos++));
	return text;
}
#endif

Tokenizer::Tokenizer(const Buffer *buffer, pos_t pos, pos_t last, bool bString, const TypeSettings *typeSetting)
	: m_buffer(*buffer)
	, m_pos(pos)
	, m_last(last)
	, m_pushed(false)
	, m_bString(bString)
	, m_typeSettings(typeSetting)
{
	init();
}
Tokenizer::Tokenizer(const Buffer &buffer, pos_t pos, pos_t last, bool bString, const TypeSettings *typeSetting)
	: m_buffer(buffer)
	, m_pos(pos)
	, m_last(last)
	, m_pushed(false)
	, m_bString(bString)
	, m_typeSettings(typeSetting)
{
	init();
}
Tokenizer::Tokenizer(const Tokenizer &x)
	: m_buffer(x.m_buffer)
	, m_pos(x.m_pos)
	, m_last(x.m_last)
	, m_pushed(false)
	, m_bString(x.m_bString)
{
	//m_lineNum = 1;
	//m_tokenType = UNDEF;
	init();
}
void Tokenizer::init()
{
	if( m_pos >= m_last ) {
		m_tokenType = END_OF_FILE;
	} else {
		m_lineNum = 1;
		m_tokenType = UNDEF;
		if( m_typeSettings != 0 )
			m_lineComment = m_typeSettings->textValue(TypeSettings::LINE_COMMENT);
		nextToken();
	}
}


Tokenizer::~Tokenizer(void)
{
}
#if		0
QChar Tokenizer::nextChar()
{
	if( m_pos < m_last )
		return QChar(m_buffer.charAt(m_pos));
	else
		return QChar();
}
#endif
#if 0
bool Tokenizer::nextBlock()
{
	m_block = m_block.next();
	if( !m_block.isValid() ) return false;
	m_buffer = m_block.text();
	if( m_buffer == "__END__" ) return false;	//	__END__ 以降は無視
	m_pos = 0;
	m_lineNum += 1;
	return true;
}
#endif
QChar Tokenizer::skipSpace()
{
	wchar_t ch = 0;
	while( m_pos < m_last && isSpaceChar(ch = m_buffer.charAt(m_pos)) )
		++m_pos;
	return QChar(ch);
}
QChar Tokenizer::skipSpaceOrNewLine()
{
	wchar_t ch = 0;
	while( m_pos < m_last && isSpaceOrNewLineChar(ch = m_buffer.charAt(m_pos)) ) {
		++m_pos;
		if( ch == '\n' )
			++m_lineNum;
		else if( ch == '\r' ) {
			if( m_buffer.charAt(m_pos) == '\n' )
				++m_pos;
			++m_lineNum;
		}
	}
	return QChar(ch);
}
//	次の行の最初のトークンまでスキップ
void Tokenizer::skipLine()
{
	while( m_pos < m_last && !isNewLineChar(m_buffer.charAt(m_pos)) )
		++m_pos;
	//while( m_pos < m_last && isNewLineChar(m_buffer.charAt(m_pos)) )
	//	++m_pos;
	wchar_t ch = m_buffer.charAt(m_pos);
	if( ch == '\n' ) {
		++m_pos;
		++m_lineNum;
	} else if( ch == '\r' ) {
		++m_pos;
		if( m_buffer.charAt(m_pos) == '\n' )
			++m_pos;
		++m_lineNum;
	}
}
wchar_t Tokenizer::nextChar() const
{
	pos_t pos = m_pos;
	wchar_t ch;
	while( isSpaceChar(ch = m_buffer.charAt(pos++)) ) {};
	return ch;
	///return m_buffer.charAt(m_pos);
}
wchar_t Tokenizer::nextNextChar() const
{
	pos_t pos = m_pos;
	wchar_t ch;
	while( isSpaceChar(ch = m_buffer.charAt(pos++)) ) {};
	return m_buffer.charAt(pos);
}
int Tokenizer::nextLineToken()			//	次の行の最初のトークンまで読み進める
{
	return nextToken();
}
int Tokenizer::nextToken()
{
	if( m_tokenType == END_OF_FILE ) return END_OF_FILE;
	if( m_pushed ) {
		m_pushed = false;
		return m_tokenType;
	}
	m_prevText = m_tokenText;
	for(;;) {
		m_firstToken = m_pos /*== 0*/;
		skipSpaceOrNewLine();
#if 0
		if( m_pos == 0 && m_buffer.startsWith("=begin") ) {
			for(;;) {
				if( !nextBlock() ) return m_tokenType = END_OF_FILE;
				if( m_buffer.startsWith("=end") ) {
					if( !nextBlock() ) return m_tokenType = END_OF_FILE;
					break;
				}
			}
			continue;
		}
#endif
		if( !m_lineComment.isEmpty() && ::startsWith(m_buffer, m_pos, m_lineComment) ) {
			skipLine();
			continue;
		}
		m_tokenLineNum = m_lineNum;
		m_tokenPosition = m_pos;
		m_tokenText.clear();
		if( m_pos >= m_last /*|| isNewLineChar(m_buffer.charAt(m_pos))*/ )
			return m_tokenType = END_OF_FILE;
		if( isNumber(m_buffer.charAt(m_pos)) ) {
			while( ++m_pos < m_last && isLetterOrNumber(m_buffer.charAt(m_pos)) ) {}
			m_tokenText = getText(m_buffer, m_tokenPosition, m_pos - m_tokenPosition);
			return m_tokenType = NUMBER;
		}
		if( isLetterOrNumberOrUnderbar(m_buffer.charAt(m_pos)) ) {
			while( ++m_pos < m_last && isLetterOrNumberOrUnderbar(m_buffer.charAt(m_pos)) ) {}
			m_tokenText = getText(m_buffer, m_tokenPosition, m_pos - m_tokenPosition);
			return m_tokenType = IDENT;
		}
		if( m_bString && (m_buffer.charAt(m_pos) == '\"' || m_buffer.charAt(m_pos) == '\'') ) {
			QChar sep = m_buffer.charAt(m_pos++);
			while( m_pos < m_last ) {
				QChar c = m_buffer.charAt(m_pos++);
				if( c == sep )
					break;
				if( c == '\\' && m_pos < m_last )
					++m_pos;
			}
			m_tokenText = getText(m_buffer, m_tokenPosition, m_pos - m_tokenPosition);
			return m_tokenType = STRING;
		}
		wchar_t ch = '\0';
		m_tokenText = QChar(m_buffer.charAt(m_pos++));
		if( m_tokenText == "<"
			&& m_pos < m_last
			&& m_buffer.charAt(m_pos) == '<' )	//	<< はひとつのトークンにする
		{
			m_tokenText += QChar('<');
			if( ++m_pos < m_last
				&& m_buffer.charAt(m_pos) == '=' )	//	<<= はひとつのトークンにする
			{
				m_tokenText += QChar('=');
				++m_pos;
			}
		} else if( m_tokenText == ">"
			&& m_pos < m_last
			&& m_buffer.charAt(m_pos) == '>' )	//	>> はひとつのトークンにする
		{
			m_tokenText += QChar('<');
			if( ++m_pos < m_last
				&& m_buffer.charAt(m_pos) == '=' )	//	>>= はひとつのトークンにする
			{
				m_tokenText += QChar('=');
				++m_pos;
			}
		} else if( (m_tokenText == "!" || m_tokenText == "=" ||
			m_tokenText == ">" || m_tokenText == "<")
			&& m_pos < m_last
			&& m_buffer.charAt(m_pos) == '=' )	//	== または != または >= または <= はひとつのトークンにする
		{
			m_tokenText += "=";
			++m_pos;
		} else if( m_tokenText == "/" 
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '/' || ch == '*' || ch == '=') )	//	//、/* /= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "*"
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '/' || ch == '=') )	//	*/ *= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "-"
			&& m_pos + 1< m_last
			&& m_buffer.charAt(m_pos) == '>'
			&&  m_buffer.charAt(m_pos+1) == '*' )	//	->* はひとつのトークンにする
		{
			m_tokenText += ">*";
			m_pos += 2;
		} else if( m_tokenText == "-"
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '>' || ch == '-' || ch == '=') )	//	->, -- -= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "+"
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '+' || ch == '=') )	//	++ += はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "&"
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '&' || ch == '=') )	//	&& &= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "|"
			&& m_pos < m_last
			&& ((ch = m_buffer.charAt(m_pos)) == '|' || ch == '=') )	//	|| |= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( (m_tokenText == "%" || m_tokenText == "^")
			&& m_pos < m_last
			&& (ch = m_buffer.charAt(m_pos)) == '=' )	//	%= ^= はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == ":"
			&& m_pos < m_last
			&& (ch = m_buffer.charAt(m_pos)) == ':' )	//	:: はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "#"
			&& m_pos < m_last
			&& (ch = m_buffer.charAt(m_pos)) == '#' )	//	## はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "."
			&& m_pos < m_last
			&& (ch = m_buffer.charAt(m_pos)) == '*' )	//	.* はひとつのトークンにする
		{
			m_tokenText += QChar(ch);
			++m_pos;
		} else if( m_tokenText == "."
			&& m_pos + 1< m_last
			&& m_buffer.charAt(m_pos) == '.'
			&&  m_buffer.charAt(m_pos+1) == '.' )	//	... はひとつのトークンにする
		{
			m_tokenText += "..";
			m_pos += 2;
		}
		return m_tokenType = SYMBOL;
	}
	return m_tokenType = END_OF_FILE;
}
