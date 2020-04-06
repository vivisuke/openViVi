//----------------------------------------------------------------------
//
//			File:			"ViewTokenizer.cpp"
//			Created:		30-7-2013
//			Author:			津田伸秀
//			Description:	表示単位を切り分けるクラス
//
//----------------------------------------------------------------------


//#include <QtGui>
#include "typeSettings.h"

#include "ViewTokenizer.h"
#include "../buffer/Buffer.h"

typedef const wchar_t cwchar;

ViewTokenizer::ViewTokenizer(const TypeSettings *typeSettings, const Buffer *buffer, int first, int sz, int last)
	: m_buffer(buffer)
	, m_typeSettings(typeSettings)
	, m_ix(first)
	, m_tokenix(first)
	, m_lastBuffer(first + sz)
	, m_lastBuffer2(last)
	, m_tokenType(UNKNOWN)
	, m_inLineComment(false)
	, m_inBlockComment(false)
	//, m_inScript(false)
	, m_inScriptStartEnd(false)
	, m_inPHP(false)
	, m_isTokenInComment(false)
	, m_inHTMLTag(false)
	, m_htmlTagLvl(0)
	, m_isTokenInHTMLTag(false)
	, m_isTokenNewLine(false)
	//, m_inString(false)
	, m_quoteChar('\0')
	, m_cursorLine(false)
{
	m_lineCommentText = typeSettings->textValue(TypeSettings::LINE_COMMENT);
	m_begBlockCommentText = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_BEG);
	m_endBlockCommentText = typeSettings->textValue(TypeSettings::BLOCK_COMMENT_END);
}

void ViewTokenizer::setBlockCommentText(const QString &beg, const QString &end)
{
	m_begBlockCommentText = beg;
	m_endBlockCommentText = end;
}
void ViewTokenizer::setQuotedText(const QString &text)
{
	m_quotedText = text;
	m_quotedTextix = m_ix;
}
void ViewTokenizer::setLineCommentText(const QString &lineCommentText)
{
	m_lineCommentText = lineCommentText;
}
inline bool isAlpha(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && isalpha(v);
}
inline bool isDigit(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && isdigit(v);
}
inline bool isAlphaOrUnderbar(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && isalpha(v) || v == '_';
}
inline bool isAlnum(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && isalnum(v);
}
inline bool isAlnumOrUnderbar(QChar qch)
{
	const ushort v = qch.unicode();
	return v < 0x80 && (isalnum(v) || v == '_');
}
int indexOfTabOrZenSpace(const QString &text)
{
	for (int i = 0; i < text.size(); ++i) {
		if( text[i] == '\t' || text[i] == L'　' )
			return i;
	}
	return -1;
}
static bool safeChar[256] = {
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	0, 1, 0, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,	//	  ! " # $ % & ' ( ) * + , - . /
/* 3 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 0, 1, 0, 1,	//	0 1 2 3 4 5 6 7 8 9 : ; < = > ?
/* 4 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,	//	@ A B C D E F G H I J K L M N O
/* 5 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 0, 0, 0, 0, 1,	//	P Q R S T U V W X Y Z [ \ ] ^ _
/* 6 */	0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,	//	` a b c d e f g h i j k l m n o
/* 7 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 0, 1, 0, 1, 0,	//	p q r s t u v w x y z { | } ~
/* 8 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 9 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* a */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* b */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* c */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* d */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* e */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* f */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
};

bool isSafeChar(wchar_t ch)
{
	return ch < 0x100 && safeChar[ch];
}
QString ViewTokenizer::nextToken()
{
	QString lastToke = m_tokenText;
	m_isTokenNewLine = false;
	//m_inString = false;
	//if( m_ix >= m_lastBuffer ) return QString();
	if( !m_quotedText.isEmpty() ) {
		//	文字列の途中に タブ、全角空白があった場合は、いったんリターンしている
		//	文字列、位置は m_quotedText、 m_quotedTextix に格納されている
		m_tokenix = m_quotedTextix;
		if( m_tokenix >= m_lastBuffer )
			return QString();
		if( m_quotedText[0] == '\t' || m_quotedText[0] == L'　') {
			QChar ch = m_quotedText[0];
			m_quotedText = m_quotedText.mid(1);
			++m_quotedTextix;
			return m_tokenText = QString(ch);
		}
		m_tokenType = QUOTED;
		//int ix = m_quotedText.indexOf(QRegExp("[\t　]"));
		int ix = indexOfTabOrZenSpace(m_quotedText);
		if( ix < 0 ) {
			if( m_tokenix + m_quotedText.size() > m_lastBuffer )
				ix = m_lastBuffer - m_tokenix;
		} else
			ix = qMin(m_lastBuffer - m_tokenix, ix);
		if( ix > 0 ) {
			//	m_quotedText が タブ、全角空白を含む場合 または m_lastBuffer を超えている場合
			QString txt = m_quotedText.left(ix);
			m_quotedText = m_quotedText.mid(ix);
			m_quotedTextix += ix;
			return m_tokenText = txt;
		} else {
			m_ix = m_quotedTextix + m_quotedText.size();
			QString txt = m_quotedText;
			m_quotedText.clear();
			return m_tokenText = txt;
		}
	}
	m_tokenType = UNKNOWN;
	m_tokenix = m_ix;
	if( m_ix >= m_lastBuffer ) return m_tokenText = QString();
	m_isTokenInComment = m_inBlockComment || m_inLineComment;
	m_isTokenInHTMLTag = m_inHTMLTag;
	if( m_inBlockComment ) {
		if( !m_endBlockCommentText.isEmpty()
			&& m_buffer->isMatched((cwchar *)m_endBlockCommentText.data(),
									m_endBlockCommentText.size(), m_ix) )
		{
			m_tokenType = COMMENT;
			m_inBlockComment = false;
			m_ix += m_endBlockCommentText.size();
			return m_tokenText = m_endBlockCommentText;
		}
	} else if( !m_inLineComment ) {
		if( !m_lineCommentText.isEmpty()
			&& m_buffer->isMatched((cwchar *)m_lineCommentText.data(),
									m_lineCommentText.size(), m_ix) )
		{
			m_tokenType = COMMENT;
			m_inLineComment = true;
			m_isTokenInComment = true;
			m_ix += m_lineCommentText.size();
			return m_tokenText = m_lineCommentText;
		}
		if( !m_begBlockCommentText.isEmpty()
			&& m_buffer->isMatched((cwchar *)m_begBlockCommentText.data(),
									m_begBlockCommentText.size(), m_ix) )
		{
			m_tokenType = COMMENT;
			m_inBlockComment = true;
			m_isTokenInComment = true;
			m_ix += m_begBlockCommentText.size();
			return m_tokenText = m_begBlockCommentText;
		}
	}
	int limit = qMin(m_ix + 256, m_lastBuffer2);		//	最大256文字
	QChar qch = m_buffer->charAt(m_ix++);
	//qDebug() << qch;
	const bool isCSS = m_typeSettings->name() == "CSS";
	if( isAlphaOrUnderbar(qch) || qch == '#' && isAlpha(m_buffer->charAt(m_ix))
		||  isCSS && qch == '-' && isAlpha(m_buffer->charAt(m_ix)) )
	{
		QString txt(qch);
		while( m_ix < m_lastBuffer ) {
			qch = m_buffer->charAt(m_ix);
			if( !(isAlnumOrUnderbar(qch) || isCSS && qch == '-') )
				break;
			txt += qch;
			++m_ix;
		}
		m_fullText = txt;
		for(int i =  m_ix; i < limit; ++i) {
			QChar ch = m_buffer->charAt(i);
			if( !(isAlnumOrUnderbar(ch) || isCSS && ch == '-') )
				break;
			m_fullText += ch;
		}
		if( (txt == "http" || txt == "https")
			&& qch == ':'
			&& m_buffer->charAt(m_ix+1) == '/'
			&& m_buffer->charAt(m_ix+2) == '/' )
		{
			txt += "://";
			m_ix += 3;
			while( m_ix < limit && isSafeChar(m_buffer->charAt(m_ix)) ) {
				txt += QChar(m_buffer->charAt(m_ix++));
			}
			m_tokenType = URL;
			return m_tokenText = txt;
		}
		m_tokenType = ALNUM;
		if( m_inHTMLTag && m_startTag && m_tagText.isEmpty() ) {
			m_tagText = txt.toLower();
			m_inScriptStartEnd = m_tagText == "script";
		}
		return m_tokenText = txt;
	}
	if( isDigit(qch) ) {
		QString txt(qch);
		while( m_ix < m_lastBuffer ) {
			qch = m_buffer->charAt(m_ix);
			if( !isAlnum(qch) )
				break;
			txt += qch;
			++m_ix;
		}
		m_tokenType = DIGITS;
		return m_tokenText = txt;
	}
#if 1
	if( qch == '\r' ) {
		m_tokenType = NEWLINE;
		m_isTokenNewLine = true;
		if( m_buffer->charAt(m_ix) == '\n' ) {
			++m_ix;
			return m_tokenText = QString(QChar(L'⏎'));
		} else
			return m_tokenText = QString(QChar(L'←'));
	}
	if( qch == '\n' ) {
		m_tokenType = NEWLINE;
		m_isTokenNewLine = true;
		return m_tokenText = QString(QChar(L'↓'));
	}
#else
	if( (qch == '\r' || qch == '\n' ) ) {
		return m_tokenText = QString();
	}
#endif
	QString txt(qch);
	if( qch.unicode() < 0x20 ) {
		m_tokenType = CTRL;
		return m_tokenText = txt;
	}
	if( qch == L'　' ) {
		m_tokenType = ZEN_SPACE;
		return m_tokenText = txt;
	}
	const bool bHTMLTag = m_typeSettings->boolValue(TypeSettings::VIEW_HTMLTAG);
	if( bHTMLTag /*&& !m_inHTMLTag*/ && qch == '<' && m_ix < m_lastBuffer ) {
		QChar qch2 = m_buffer->charAt(m_ix);
		if( !m_inLineComment && isAlpha(qch2) && !m_inScriptStartEnd ) {
			++m_htmlTagLvl;
			m_tagText.clear();
			//m_inScriptStartEnd = false;
			m_inHTMLTag = true;
			m_isTokenInHTMLTag = true;
			m_startTag = true;
			m_tokenType = HTMLTAG;
			return m_tokenText = txt;
		}
		if( qch2 == '/' && m_ix + 1 < m_lastBuffer ) {		//	</ の場合
			txt += qch2;
			qch2 = m_buffer->charAt(++m_ix);
			if( isAlpha(qch2) ) {
				++m_htmlTagLvl;
				m_tagText.clear();
				if( m_inScriptStartEnd && m_buffer->isMatchedIC(L"script>", m_ix) ) {
					m_inScriptStartEnd = false;
				}
				m_inHTMLTag = true;
				m_isTokenInHTMLTag = true;
				m_startTag = false;
				m_tokenType = HTMLTAG;
				return m_tokenText = txt;
			}
		}
		if( qch2 == '?' ) {
			m_inPHP = true;
			++m_ix;
			txt += qch2;
			return m_tokenText = txt;
		}
	} else if( bHTMLTag && m_inPHP && qch == '?' && m_ix < m_lastBuffer ) {
		QChar qch2 = m_buffer->charAt(m_ix);
		if( qch2 == '>' ) {
			m_inPHP = false;
			++m_ix;
			txt += qch2;
			return m_tokenText = txt;
		}
	} else if( m_inHTMLTag && qch == '>' && --m_htmlTagLvl <= 0 ) {
		m_htmlTagLvl = 0;
		m_tokenType = HTMLTAG;
		m_inHTMLTag = false;
		return m_tokenText = txt;
	}
	//bool preAtMark = false;
	bool backSlashEscape = true;
	if( m_typeSettings->name() == "C#"
		&& qch == '@'
		&& m_buffer->charAt(m_ix) == '\"' )
	{
		qch = m_buffer->charAt(m_ix++);
		txt += qch;
		//preAtMark = true;
		backSlashEscape = false;
	}
	if( qch == '\"' || qch == '\'' ) {		//	ダブル・シングルクォート発見
		if( lastToke.toLower() == "r" && m_typeSettings->name() == "PYTHON" ) {
			//preAtMark = true;
			backSlashEscape = false;
		}
		const QString txt0 = txt;
		int ix = m_ix;
		int tsix = -1;		//	tab or 全角空白 index
		while( ix < limit ) {
			QChar ch = m_buffer->charAt(ix++);
			if( ch == '\r' || ch == '\n' ) break;
			if( tsix < 0 && (ch == '\t' || ch == L'　') )
				tsix = ix - m_ix;
			txt += ch;
			if( ch == qch ) {				//	ちゃんとバランスしている場合のみ文字列とみなす
				m_tokenType = QUOTED;
				m_ix = ix;
				if( m_ix > m_lastBuffer ) {		//	最終位置を超えている場合
					//txt = txt.left(m_lastBuffer - m_tokenix);
					tsix = m_lastBuffer - m_tokenix;
				}
				//	done: TAB、全角空白 を含む場合
				//if( (ix = txt.indexOf(QRegExp("[\t\\u3000]"))) >= 0 )		//	メモ：最初はクォート文字なので ix > 0
				if (tsix >= 0) {
					m_quotedText = txt.mid(tsix);
					m_quotedTextix = m_tokenix + tsix;
					txt = txt.left(tsix);
				}
				//m_inString = true;
				return m_tokenText = txt;
			}
			if( backSlashEscape && ch == '\\' && ix < m_lastBuffer ) {
				txt += QChar(m_buffer->charAt(ix++));
			}
		}
		txt = txt0;		//	バランスしていない場合は次に進む
		//return txt;
	}
	bool isHTML = m_typeSettings->name() == "HTML";
#if	1
	if( !m_cursorLine && isHTML && qch == '&' ) {
		if( m_buffer->isMatched(L"gt;", 3, m_ix) ) {
			m_orgText = "&gt;";
			m_ix += 3;
			m_tokenType =  HTML_SPECIAL_CHARS;
			return m_tokenText = ">";
		}
		if( m_buffer->isMatched(L"lt;", 3, m_ix) ) {
			m_orgText = "&lt;";
			m_ix += 3;
			m_tokenType =  HTML_SPECIAL_CHARS;
			return m_tokenText = "<";
		}
		if( m_buffer->isMatched(L"amp;", 4, m_ix) ) {
			m_orgText = "&amp;";
			m_ix += 4;
			m_tokenType =  HTML_SPECIAL_CHARS;
			return m_tokenText = "&";
		}
		if( m_buffer->isMatched(L"nbsp;", 5, m_ix) ) {
			m_orgText = "&nbsp;";
			m_ix += 5;
			m_tokenType =  HTML_SPECIAL_CHARS;
			return m_tokenText = " ";
		}
	}
#endif
	while( m_ix < m_lastBuffer ) {
		qch = m_buffer->charAt(m_ix);
		if( qch.unicode() < 0x20
			|| qch == '\"' || qch == '\'' || qch == '@'
			|| isAlnum(qch)
			|| bHTMLTag /*&& !m_inHTMLTag*/ && qch == '<'
			|| !m_inLineComment && !m_lineCommentText.isEmpty()
				&& m_buffer->isMatched((cwchar *)m_lineCommentText.data(),
										m_lineCommentText.size(), m_ix)
			|| !m_inBlockComment && !m_begBlockCommentText.isEmpty()
				&& m_buffer->isMatched((cwchar *)m_begBlockCommentText.data(),
										m_begBlockCommentText.size(), m_ix)
			|| m_inBlockComment && !m_endBlockCommentText.isEmpty()
				&& m_buffer->isMatched((cwchar *)m_endBlockCommentText.data(),
										m_endBlockCommentText.size(), m_ix)
			|| qch == L'　'
			|| !m_cursorLine && isHTML && qch == '&' )
		{
			break;
		}
		txt += qch;
		++m_ix;
		if( m_inHTMLTag && qch == '>' ) {
			m_inHTMLTag = false;
			break;
		}
	}
	m_tokenType = OTHER;
	return m_tokenText = txt;
}
