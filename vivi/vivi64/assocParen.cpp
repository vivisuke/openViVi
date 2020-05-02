//----------------------------------------------------------------------
//
//			File:			"assocParen.cpp"
//			Created:		20-10-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include "assocParen.h"
#include "tokenizer.h"
#include "typeSettings.h"
#include "../buffer/Buffer.h"
#include <QStringList>
#include <QList>
#include <QDebug>

enum {
	FIND_FORWARD,
	FIND_BACKWARD,
};
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	�s�ԍ�

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
inline bool isSpaceOrNewLine(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}
inline bool isAlpha(wchar_t ch)
{
	return ch < 0x100 && isalpha(ch);
}

pos_t assocParenPositionForward(TypeSettings *typeSettings, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen)
{
	//	undone: ��������̊��ʑΉ�
	//pos_t pos0 = pos;
	QString bcBeg = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG);
	QString bcEnd = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END);
	QString lcstr = typeSettings->textValue(TypeSettings::LINE_COMMENT);
	//qDebug() << lcstr;
	const wchar_t *blockCommentBeg = (const wchar_t *)bcBeg.data();
	const wchar_t *blockCommentEnd = (const wchar_t *)bcEnd.data();
	const wchar_t *lineComment = (const wchar_t *)lcstr.data();
	int limit = buffer.size();
	int ln = buffer.positionToLine(pos);
	pos_t first = buffer.lineStartPosition(ln);
	//int last = buffer.lineStartPosition(ln+1);
	bool inBlockComment = (buffer.lineFlags(ln) & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0;
	while( first != pos ) {
		if( inBlockComment ) {
			if( buffer.isMatched(blockCommentEnd, first) ) {
				inBlockComment = false;
				first += bcEnd.size();
			}
		} else {
			if( buffer.isMatched(blockCommentBeg, first) ) {
				inBlockComment = true;
				first += bcBeg.size();
			} else if( buffer.isMatched(lineComment, first) ) {
				//	�J�[�\���ʒu�ȑO�ɍs�R�����g������ꍇ�́A�s���܂łŒT����������߂�
				limit = buffer.lineStartPosition(ln+1);
				break;
			}
		}
		++first;
	}
	for(int lvl = 1;;) {
		if( ++pos >= limit ) return -1;		//	�s��v
		wchar_t ch = buffer.charAt(pos);
		if( ch == dParen && --lvl == 0 )
			return pos;
		else if( ch == paren )
			++lvl;
		else if( ch == '\'' || ch == '\"' ) {
			wchar_t sep = ch;
			while( (ch = buffer.charAt(++pos)) != sep ) {
				if( ch == '\\' )
					++pos;
				else if( isNewLine(ch) )
					break;		//	���N�H�[�g���s���ɖ����ꍇ
			}
		} else if( buffer.isMatched(lineComment, pos) ) {
			//	���s�܂ŃX�L�b�v
			while( pos < limit && !isNewLine(buffer.charAt(pos)) ) ++pos;
		} else {
			if( inBlockComment ) {
				if( buffer.isMatched(blockCommentEnd, pos) )
					return -1;
			} else {
				if( buffer.isMatched(blockCommentBeg, pos) ) {
					pos += bcBeg.size();
					for(;;) {
						if( buffer.isMatched(blockCommentEnd, pos) )
							break;
						if( ++pos >= limit ) return -1;
					}
				}
			}
		}
	}
}
pos_t assocParenPositionBackward(TypeSettings *typeSettings, const Buffer &buffer, pos_t pos,
								wchar_t paren, wchar_t dParen)
{
	//	undone: ��������̊��ʑΉ�
	QString bcBeg = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG);
	QString bcEnd = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END);
	QString lcstr = typeSettings->textValue(TypeSettings::LINE_COMMENT);
	qDebug() << lcstr;
	const wchar_t *blockCommentBeg = (const wchar_t *)bcBeg.data();
	const wchar_t *blockCommentEnd = (const wchar_t *)bcEnd.data();
	const wchar_t *lineComment = (const wchar_t *)lcstr.data();
#if		1
	int lvl = 1;
	int ln = buffer.positionToLine(pos);
	int first = buffer.lineStartPosition(ln);
	bool inBlockComment = (buffer.lineFlags(ln) & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0;
	while( first < pos ) {
		if( inBlockComment ) {
			if( buffer.isMatched(blockCommentEnd, first) ) {
				inBlockComment = false;
				first += bcEnd.size();
			}
		} else {
			if( buffer.isMatched(blockCommentBeg, first) ) {
				inBlockComment = true;
				first += bcBeg.size();
			}
			//else if( buffer.isMatched(lineComment, first) ) {
			//	//	�J�[�\���ʒu�ȑO�ɍs�R�����g������ꍇ�́A�s���܂łŒT����������߂�
			//	limit = buffer.lineStartPosition(ln+1);
			//	break;
			//}
		}
		++first;
	}
	bool init = true;
	while( ln >= 0 ) {
		std::vector<wchar_t> vch;		//	paren or dParen
		std::vector<int> vpos;			//	���ʈʒu
		pos_t first = buffer.lineStartPosition(ln);
		pos_t last = qMin(buffer.lineStartPosition(ln+1), pos);
		uint flags = buffer.lineFlags(ln);
		if( (flags & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0 ) {
			for(;;) {
				if( buffer.isMatched(blockCommentEnd, first) )
					break;
				if( ++first >= last ) break;
			}
		}
		int lcPos = -1;
		while( first < last ) {
			wchar_t ch = buffer.charAt(first++);
			if( ch == paren || ch == dParen ) {
				vch.push_back(ch);
				vpos.push_back(first - 1);
			} else if( ch == '\'' || ch == '\"' ) {
				wchar_t sep = ch;
				while( first < last && (ch = buffer.charAt(first++)) != sep ) {
					if( ch == '\\' )
						++first;
				}
			} else if( buffer.isMatched(lineComment, first - 1) ) {
				if( init )
					lcPos = first - 1;
				else
					break;
			} else if( buffer.isMatched(blockCommentBeg, first - 1) ) {
				first += bcBeg.size();
				for(;;) {
					if( buffer.isMatched(blockCommentEnd, first) )
						break;
					if( ++first >= last ) break;
				}
			}
		}
		for(int i = vch.size(); --i >= 0;) {
			if( vpos[i] < lcPos ) break;
			if( vch[i] == dParen ) {
				if( !--lvl ) return vpos[i];
			} else if( vch[i] == paren ) {
				++lvl;
			}
		}
		if( lcPos >= 0 ) break;
		init = false;
		--ln;
	}
	return -1;		//	not found
#else
	for(int lvl = 1;;) {
		if( --pos < 0 ) return -1;		//	not found
		wchar_t ch = buffer.charAt(pos);
		if( ch == dParen && --lvl == 0 )
			return pos;
		else if( ch == paren )
			++lvl;
		else if( ch == '\'' || ch == '\"' ) {
			wchar_t sep = ch;
			while( --pos >= 0 && !isNewLine(ch = buffer.charAt(pos)) ) {
				if( ch == sep ) {
					if( !pos || buffer.charAt(pos-1) != '\\' )
						break;
					--pos;
				}
			}
		}
	}
#endif
}
//	�p�[�X�͍s�̐擪����s���A�J�n�E�I���^�O�Ƃ��̈ʒu����������X�^�b�N�ɐς�
//	���̌セ����t���ɕ]�����Ă���
struct TagPos
{
	pos_t		m_pos;			//	< �̈ʒu
	QString	m_tag;		//	�I���^�O�̏ꍇ�� "/" + �^�O������
public:
	TagPos() {}
	TagPos(const TagPos &x)
		: m_pos(x.m_pos), m_tag(x.m_tag) {}
	TagPos(pos_t pos, const QString &tag)
		: m_pos(pos), m_tag(tag) {}
};
void parseTags(const Buffer &buffer, Tokenizer &tkn,
						std::vector<TagPos> &vTags2, QString &lastToken)
{
	while( tkn.nextToken() != Tokenizer::END_OF_FILE
				&& tkn.tokenText() != ">" )
	{
		//qDebug() << tkn.tokenText();
		int p2 = tkn.tokenPosition();
		if( tkn.tokenText() == "<"
			&& (isAlpha(buffer[p2+1])
				|| buffer[p2+1] == '/' && isAlpha(buffer[p2+2])) )
		{
			bool endTag = buffer[p2+1] == '/';
			if( endTag ) tkn.nextTokenText();	//	skip /
			QString tag2 = tkn.nextTokenText();
			QString lastToken2;
			while( tkn.nextToken() != Tokenizer::END_OF_FILE
						&& tkn.tokenText() != ">" )
			{
				lastToken2 = tkn.tokenText();
			}
			if( tkn.tokenText() != ">" ) break;		//	�^�O���P�s�ɖ����ꍇ
			if( lastToken != "/" ) {		//		�Ōオ /> �Ŗ����ꍇ
				if( endTag ) tag2 = "/" + tag2;
				vTags2.push_back(TagPos(p2, tag2));
			}
		}
		lastToken = tkn.tokenText();
	}
}
bool searchStartTag(const Buffer &buffer, pos_t &pos)
{
	QStringList tagList;
	int ln = buffer.positionToLine(pos);
	pos_t ls = buffer.lineStartPosition(ln+1);
	while( ln >= 0 ) {
		std::vector<TagPos> vTags;
		bool inBlockComment = (buffer.lineFlags(ln) & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0;
		pos_t nxls = ls;
		ls = buffer.lineStartPosition(ln);
		Tokenizer tkn(buffer, ls, nxls);
		while( !tkn.tokenText().isEmpty() ) {
			int p = tkn.tokenPosition();
			if( p > pos ) break;
			if( inBlockComment ) {
				if( tkn.tokenText() == "-" && buffer[p+1] == '-' && buffer[p+2] == '>' ) {
					inBlockComment = false;
				}
			} else {
				if( tkn.tokenText() == "<" ) {
					if( buffer[p+1] == '!' && buffer[p+2] == '-' && buffer[p+3] == '-' ) {
						tkn.nextToken();		//	!
						tkn.nextToken();		//	-
						tkn.nextToken();		//	-
						inBlockComment = true;
					} else if( isAlpha(buffer[p+1]) ) {		//	<IDENT �̏ꍇ
						std::vector<TagPos> vTags2;
						QString tag = tkn.nextTokenText();
						QString lastToken;
						parseTags(buffer, tkn, vTags2, lastToken);
						if( tkn.tokenText() != ">" ) break;		//	�^�O���P�s�ɖ����ꍇ
						if( lastToken != "/" )		//		�Ōオ /> �Ŗ����ꍇ
							vTags.push_back(TagPos(p, tag));
						if( !vTags2.empty() )
							vTags.insert(vTags.end(), vTags2.begin(), vTags2.end());
					} else if( buffer[p+1] == '/' && isAlpha(buffer[p+2]) ) {		//	</IDENT �̏ꍇ
						tkn.nextToken();	//	skip /
						QString tag = "/" + tkn.nextTokenText();
						vTags.push_back(TagPos(p, tag));
					}
				}
			}
			tkn.nextToken();
		}
		while( !vTags.empty() ) {
			TagPos t(vTags[vTags.size() - 1]);
			if( t.m_tag[0] == '/' )
				tagList.push_back(t.m_tag.mid(1));
			else {
				int ix = tagList.lastIndexOf(t.m_tag);
				if( !ix ) {
					pos = t.m_pos;
					return true;
				}
				if( ix >= 0 ) {
					while( tagList.size() > ix )
						tagList.pop_back();
				}
			}
			vTags.pop_back();
		}
		--ln;
	}
	return false;
}
bool searchEndTag(const Buffer &buffer, pos_t &pos)
{
	bool inBlockComment = false;
	QStringList tagList;
	int ln = buffer.positionToLine(pos);
	pos_t nxls = buffer.lineStartPosition(ln);
	int offset = pos - nxls;
	while( ln < buffer.lineCount() ) {
		pos_t ls = nxls;
		nxls = buffer.lineStartPosition(ln+1);
		Tokenizer tkn(buffer, ls + offset, nxls);
		while( !tkn.tokenText().isEmpty() ) {
			//qDebug() << tkn.tokenText();
			int p = tkn.tokenPosition();
			if( inBlockComment ) {
				if( tkn.tokenText() == "-" && buffer[p+1] == '-' && buffer[p+2] == '>' ) {
					inBlockComment = false;
				}
			} else {
				if( tkn.tokenText() == "<" ) {
					if( buffer[p+1] == '!' && buffer[p+2] == '-' && buffer[p+3] == '-' ) {
						tkn.nextToken();		//	!
						tkn.nextToken();		//	-
						tkn.nextToken();		//	-
						inBlockComment = true;
					} else if( isAlpha(buffer[p+1]) ) {		//	<IDENT �̏ꍇ
						std::vector<TagPos> vTags2;
						QString tag = tkn.nextTokenText();
						QString lastToken;
						parseTags(buffer, tkn, vTags2, lastToken);
						if( tkn.tokenText() != ">" ) break;		//	�^�O���P�s�ɖ����ꍇ
						if( lastToken != "/" ) {
							tagList += tag;
							//if( !vTags2.empty() ) {
							//	for(auto itr = vTags2.begin(); itr != vTags2; ++itr) {
							//	}
							//}
						} else {
							if( tagList.isEmpty() ) {
								return false;
							}
						}
					} else if( buffer[p+1] == '/' && isAlpha(buffer[p+2]) ) {		//	</IDENT �̏ꍇ
						tkn.nextToken();
						QString tag = tkn.nextTokenText();
						int ix = tagList.lastIndexOf(tag);
						if( !ix ) {
							pos = tkn.tokenPosition() - 2;
							return true;
						}
						if( ix >= 0 ) {
							while( tagList.size() > ix )
								tagList.pop_back();
						}
					}
				}
			}
			tkn.nextToken();
		}
		++ln;
		offset = 0;
	}
	return false;
}
int assocSharpLine(const Buffer &buffer, pos_t pos, int ln, int dir)
{
	int lvl = 0;
	for(;;) {
		if( dir == FIND_FORWARD ) {
			if( ++ln >= buffer.lineCount() ) break;
		} else {
			if( --ln < 0 ) break;
		}
		int p = buffer.lineStartPosition(ln);
		while( isSpaceChar(buffer[p]) ) ++p;
		if( buffer[p] == '#' ) {
			if( buffer.isEqual(p+1, L"if") && isSpaceChar(buffer[p+3])
				|| buffer.isEqual(p+1, L"ifdef") && isSpaceChar(buffer[p+6])
				|| buffer.isEqual(p+1, L"region") && isSpaceOrNewLine(buffer[p+7]))
			{
				if( dir == FIND_BACKWARD ) {
					if( --lvl < 0 )
						return buffer.lineStartPosition(ln);
				} else
					++lvl;
			} else if( buffer.isEqual(p+1, L"else")&& isSpaceOrNewLine(buffer[p+5])
						|| buffer.isEqual(p+1, L"elif")&& isSpaceChar(buffer[p+5]) )
			{
				if( !lvl )
					return buffer.lineStartPosition(ln);
			} else if( buffer.isEqual(p+1, L"endif") && isSpaceOrNewLine(buffer[p+6])
							|| buffer.isEqual(p+1, L"endregion") && isSpaceOrNewLine(buffer[p+10]) )
			{
				if( dir == FIND_FORWARD) {
					if( --lvl < 0 )
						return buffer.lineStartPosition(ln);
				} else
					++lvl;
			}
		}
	}
	return pos;
}
//	pos�ʒu�̊��ʂɑΉ����銇�ʈʒu��Ԃ��B�����ꍇ�͂��Ƃ̈ʒu��Ԃ�
pos_t assocParenPosition(TypeSettings *typeSettings, const Buffer &buffer, pos_t pos, bool &bSharp)
{
	pos_t pos0 = pos;
	int ln = buffer.positionToLine(pos);
	//	��������	#if #else #endif �ړ�
	static uchar prev_dir = FIND_FORWARD;
	if( buffer[pos] == '#' && buffer.isSpaces(buffer.lineStartPosition(ln), pos) ) {
		bSharp = true;
		if( buffer.isEqual(pos+1, L"if") && isSpaceChar(buffer[pos+3])
			|| buffer.isEqual(pos+1, L"ifdef") && isSpaceChar(buffer[pos+6])
			|| buffer.isEqual(pos+1, L"region") && isSpaceOrNewLine(buffer[pos+7]) )
		{
			return assocSharpLine(buffer, pos, ln, prev_dir = FIND_FORWARD);
		} else if( buffer.isEqual(pos+1, L"else")&& isSpaceOrNewLine(buffer[pos+5])
					|| buffer.isEqual(pos+1, L"elif")&& isSpaceChar(buffer[pos+5]) )
		{
			return assocSharpLine(buffer, pos, ln, prev_dir);
		} else if( buffer.isEqual(pos+1, L"endif") && isSpaceOrNewLine(buffer[pos+6])
						|| buffer.isEqual(pos+1, L"endregion") && isSpaceOrNewLine(buffer[pos+10]) )
		{
			return assocSharpLine(buffer, pos, ln, prev_dir = FIND_BACKWARD);
		}
	}
	bSharp = false;
	//	�����܂�	#if #else #endif �ړ�
	//	<tag> - </tag> �ړ�
	if( buffer[pos] == '<' && buffer[pos+1] != '!' && buffer[pos+1] != '%' && buffer[pos+1] != '?' ) {
		if( pos + 1 < buffer.size() && buffer[pos + 1] == '/' ) {	//	�I���^�O�̏ꍇ
			if( pos + 2 < buffer.size() && (isAlpha(buffer[pos+2]) ) &&
				searchStartTag(buffer, pos) )
			{
				return pos;
			}
		} else {
			if( pos + 1 < buffer.size() && (isAlpha(buffer[pos+1]) ) &&
				searchEndTag(buffer, pos) )
			{
				return pos;
			}
		}
	}
	//	�����܂� <tag> - </tag> �ړ�
	
	int last = buffer.lineStartPosition(ln + 1);
	wchar_t paren = 0;
	wchar_t dParen = 0;
	bool forward;
	while( pos < last ) {
		switch( (paren = buffer.charAt(pos)) ) {
		case L'�i':	forward = true;		dParen = L'�j';	goto toCont;
		case L'�j':	forward = false;	dParen = L'�i';	goto toCont;
		case L'�u':	forward = true;		dParen = L'�v';	goto toCont;
		case L'�v':	forward = false;	dParen = L'�u';	goto toCont;
		case L'�m':	forward = true;		dParen = L'�n';	goto toCont;
		case L'�n':	forward = false;	dParen = L'�m';	goto toCont;
		case L'�o':	forward = true;		dParen = L'�p';	goto toCont;
		case L'�p':	forward = false;	dParen = L'�o';	goto toCont;
		case L'�w':	forward = true;		dParen = L'�x';	goto toCont;
		case L'�x':	forward = false;	dParen = L'�w';	goto toCont;
		case L'�y':	forward = true;		dParen = L'�z';	goto toCont;
		case L'�z':	forward = false;	dParen = L'�y';	goto toCont;
		case L'�k':	forward = true;		dParen = L'�l';	goto toCont;
		case L'�l':	forward = false;	dParen = L'�k';	goto toCont;
		case L'�q':	forward = true;		dParen = L'�r';	goto toCont;
		case L'�r':	forward = false;	dParen = L'�q';	goto toCont;
		case L'�s':	forward = true;		dParen = L'�t';	goto toCont;
		case L'�t':	forward = false;	dParen = L'�s';	goto toCont;
		case L'�e':	forward = true;		dParen = L'�f';	goto toCont;
		case L'�f':	forward = false;	dParen = L'�e';	goto toCont;
		case L'�g':	forward = true;		dParen = L'�h';	goto toCont;
		case L'�h':	forward = false;	dParen = L'�g';	goto toCont;

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
		pos = assocParenPositionForward(typeSettings, buffer, pos, paren, dParen);
	} else {
		pos = assocParenPositionBackward(typeSettings, buffer, pos, paren, dParen);
	}
	if( pos < 0 ) pos = pos0;
	return pos;
}
