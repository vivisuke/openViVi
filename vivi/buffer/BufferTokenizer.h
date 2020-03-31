//----------------------------------------------------------------------
//
//			File:			"BufferTokenizer.h"
//			Created:		29-9-2013
//			Author:			�Óc�L�G
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
		UNDEF = 0,	//	�����l
		IDENT,		//	�p���Ŏn�܂�p������
		NUMBER,		//	�����Ŏn�܂�p������
		STRING,		//	������
		SYMBOL,		//	���̑��L��
		END_OF_FILE,
	};
public:
	BufferTokenizer(const Buffer &, int, int);
	
private:
	const Buffer	&m_buffer;
	int		m_pos;
	int		m_last;
	bool	m_pushed;
	bool	m_firstToken;	//	�s���̍ŏ��̃g�[�N��
	int		m_lineNum;		//	���ݍs�ԍ� (1..*)
	byte	m_tokenType;	//	�g�[�N���^�C�v
	int		m_tokenFirst;
	int		m_tokenSize;
	int		m_prevTokenFirst;
	int		m_prevTokenSize;
	//QString	m_tokenText;	//	���݃g�[�N��������
	//QString	m_prevText;		//	�ЂƂO�̃g�[�N��������
	int		m_tokenLineNum;
	int		m_tokenPosition;	//	�g�[�N���J�n�I�t�Z�b�g
};

#endif		//_HEADER_BUFFERTOKENIZER_H
