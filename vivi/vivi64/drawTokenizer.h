//----------------------------------------------------------------------
//
//			File:			"drawTokenizer.h"
//			Created:		30-7-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_DRAWTOKENIZER_H
#define		_HEADER_DRAWTOKENIZER_H

#include <QString>

typedef unsigned char byte;

class Buffer;
class TypeSettings;

class DrawTokenizer
{
public:
	enum {
		UNKNOWN = 0,
		STRING,
		ALNUM,		//	�p���Ŏn�܂�p������
		DIGITS,		//	10�i��
		QUOTED,		//	�V���O���A�_�u���N�H�[�g���ꂽ������
		SYMBOL,		//	!"# �Ȃǂ̋L���ށi���p�󔒂��܂ށj
		ZEN_SPACE,	//	�S�p��
		HTMLTAG,	//	<...> ���e�L�X�g
		COMMENT,	//	�R�����g
		CTRL,		//	0x20 ����
		NEWLINE,	//	CR/LF/CRLF
		URL,
		HTML_SPECIAL_CHARS,		//	&lt; ����HTML���ꕶ��
		OTHER,		//	���̑��i�}���`�o�C�g�����Ȃǁj
	};
public:
	DrawTokenizer(const TypeSettings *typeSettings, const Buffer *buffer, int first, int sz, int last);
	~DrawTokenizer() {}

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
	QString m_fullText;		//	�J�[�\���ʒu�ȍ~���܂߂��e�L�X�g
	int		m_ix;		//	���̕���
	int		m_tokenix;	//	�g�[�N���ŏ��̕����̃o�b�t�@�������ʒu
	int		m_quotedTextix;
	int		m_lastBuffer;
	int		m_lastBuffer2;			//	�z���g�̃o�b�t�@����
	byte	m_tokenType;				//	�g�[�N�����
	bool	m_inLineComment;
	bool	m_inBlockComment;
	//bool	m_inScript;
	bool	m_inPHP;
	bool	m_isTokenInComment;
	//bool	m_inString;
	bool	m_inHTMLTag;						//	"<" ... ">" �̒�
	int		m_htmlTagLvl;						//	"<" �̃l�X�g��
	bool	m_inScriptStartEnd;				//	"<script> ... </script> �̒�
	bool	m_isTokenInHTMLTag;
	bool	m_isTokenNewLine;
	bool	m_startTag;								//	true for "<", false for "</"
	bool	m_cursorLine;
	char	m_quoteChar;							//	�_�u���E�V���O���N�H�[�g���̏ꍇ�ɗL��
	const TypeSettings	*m_typeSettings;
	const Buffer	*m_buffer;
	QString	m_orgText;							//	&lt; ����HTML���ꕶ��
	QString	m_tagText;							//	"<" �^�O�e�L�X�g
	QString	m_quotedText;						//	������e�L�X�g
	QString	m_lineCommentText;
	QString	m_begBlockCommentText;
	QString	m_endBlockCommentText;
};

#endif		//_HEADER_DRAWTOKENIZER_H
