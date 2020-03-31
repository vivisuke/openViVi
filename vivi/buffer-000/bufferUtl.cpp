//----------------------------------------------------------------------
//
//			File:			"bufferUtl.cpp"
//			Created:		17-9-2013
//			Author:			’Ã“cLG
//			Description:
//
//----------------------------------------------------------------------

#include "Buffer.h"
#include "bufferUtl.h"

#include <QString>

#if		0
inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isAlpha(wchar_t ch)
{
	return isalpha(ch);
}

int assocParenPositionForward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen)
{
	//	undone: •¶š—ñ“à‚ÌŠ‡ŒÊ‘Î‰
	//int pos0 = pos;
	const int sz = buffer.size();
	for(int lvl = 1;;) {
		if( ++pos >= sz ) return -1;		//	•sˆê’v
		wchar_t ch = buffer.charAt(pos);
		if( ch == dParen && --lvl == 0 )
			return pos;
		else if( ch == paren )
			++lvl;
	}
}
int assocParenPositionBackward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen)
{
	//	undone: •¶š—ñ“à‚ÌŠ‡ŒÊ‘Î‰
	for(int lvl = 1;;) {
		if( --pos < 0 ) return -1;
		wchar_t ch = buffer.charAt(pos);
		if( ch == dParen && --lvl == 0 )
			return pos;
		else if( ch == paren )
			++lvl;
	}
}
bool searchStartTag(const Buffer &buffer, int pos)
{
	return false;
}
bool searchEndTag(const Buffer &buffer, int pos)
{
	int ln = buffer.positionToLine(pos);
	int nxls = buffer.lineStartPosition(ln);
	int offset = pos - nxls;
	while( ln < buffer.lineCount() ) {
		int ls = nxls;
		nxls = buffer.lineStartPosition(ln+1);
		//Tokenizer tkn();
		
		offset = 0;
	}
	return false;
}
//	posˆÊ’u‚ÌŠ‡ŒÊ‚É‘Î‰‚·‚éŠ‡ŒÊˆÊ’u‚ğ•Ô‚·B–³‚¢ê‡‚Í‚à‚Æ‚ÌˆÊ’u‚ğ•Ô‚·
int assocParenPosition(const Buffer &buffer, int pos)
{
	if( buffer[pos] == '<' && buffer[pos+1] != '!' && buffer[pos+1] != '%' && buffer[pos+1] != '?' ) {
		if( pos + 1 < buffer.size() && buffer[pos + 1] == '/' ) {	//	I—¹ƒ^ƒO‚Ìê‡
			if( pos + 2 < buffer.size() && (isAlpha(buffer[pos+2]) ) &&
				searchStartTag(buffer, pos + 1) )
			{
				return true;
			}
		} else {
			if( pos + 1 < buffer.size() && (isAlpha(buffer[pos+1]) ) &&
				searchEndTag(buffer, pos) )
			{
				return true;
			}
		}
	}
	int pos0 = pos;
	int ln = buffer.positionToLine(pos);
	int last = buffer.lineStartPosition(ln + 1);
	wchar_t paren = 0;
	wchar_t dParen = 0;
	bool forward;
	while( pos < last ) {
		switch( (paren = buffer.charAt(pos)) ) {
		case L'i':	forward = true;		dParen = L'j';	goto toCont;
		case L'j':	forward = false;	dParen = L'i';	goto toCont;
		case L'u':	forward = true;		dParen = L'v';	goto toCont;
		case L'v':	forward = false;	dParen = L'u';	goto toCont;
		case L'm':	forward = true;		dParen = L'n';	goto toCont;
		case L'n':	forward = false;	dParen = L'm';	goto toCont;
		case L'o':	forward = true;		dParen = L'p';	goto toCont;
		case L'p':	forward = false;	dParen = L'o';	goto toCont;
		case L'w':	forward = true;		dParen = L'x';	goto toCont;
		case L'x':	forward = false;	dParen = L'w';	goto toCont;
		case L'y':	forward = true;		dParen = L'z';	goto toCont;
		case L'z':	forward = false;	dParen = L'y';	goto toCont;
		case L'k':	forward = true;		dParen = L'l';	goto toCont;
		case L'l':	forward = false;	dParen = L'k';	goto toCont;
		case L'q':	forward = true;		dParen = L'r';	goto toCont;
		case L'r':	forward = false;	dParen = L'q';	goto toCont;
		case L's':	forward = true;		dParen = L't';	goto toCont;
		case L't':	forward = false;	dParen = L's';	goto toCont;
		case L'e':	forward = true;		dParen = L'f';	goto toCont;
		case L'f':	forward = false;	dParen = L'e';	goto toCont;
		case L'g':	forward = true;		dParen = L'h';	goto toCont;
		case L'h':	forward = false;	dParen = L'g';	goto toCont;

		case '(':	forward = true;		dParen = ')';	goto toCont;
		case ')':	forward = false;	dParen = '(';	goto toCont;
		case '{':	forward = true;		dParen = '}';	goto toCont;
		case '}':	forward = false;	dParen = '{';	goto toCont;
		case '[':	forward = true;		dParen = ']';	goto toCont;
		case ']':	forward = false;	dParen = '[';	goto toCont;
		}
		++pos;
	}
	return pos0;
toCont:
	if( forward ) {
		pos = assocParenPositionForward(buffer, pos, paren, dParen);
	} else {
		pos = assocParenPositionBackward(buffer, pos, paren, dParen);
	}
	if( pos < 0 ) pos = pos0;
	return pos;
}
#endif
QString getText(const Buffer &buffer, int pos, int sz)
{
	QString text;
	while( --sz >= 0 )
		text += QChar(buffer.charAt(pos++));
	return text;
}
QString getLineText(const Buffer &buffer, int ln)
{
	int ls = buffer.lineStartPosition(ln);
	int nxls = buffer.lineStartPosition(ln+1);
	return getText(buffer, ls, nxls - ls);
}
bool startsWith(const Buffer &buffer, int pos, const QString &pat)
{
	for (int i = 0; i < pat.size(); ++i) {
		if( buffer.charAt(pos+i) != pat[i].unicode() )
			return false;
	}
	return true;
}
