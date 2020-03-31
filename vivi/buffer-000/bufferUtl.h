//----------------------------------------------------------------------
//
//			File:			"bufferUtl.h"
//			Created:		17-9-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_BUFFERUTL_H
#define		_HEADER_BUFFERUTL_H

#include <QString>
//#include <string>

QString getText(const Buffer &, int pos, int sz);
QString getLineText(const Buffer &, int ln);
bool startsWith(const Buffer&, int pos, const QString &);

#if		0
typedef const char cchar;
typedef const wchar_t cwchar;

class Buffer;

//	pos�ʒu�̊��ʂɑΉ����銇�ʈʒu��Ԃ��B�����ꍇ�� -1
int assocParenPosition(const Buffer &, int pos);
int assocParenPositionForward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen);
int assocParenPositionBackward(const Buffer &buffer, int pos,
								wchar_t paren, wchar_t dParen);
#endif

#if 0
//	�I�[�g�C���f���g�e�L�X�g�擾
std::wstring autoIndentText(const Buffer &buffer,
							int pos,				//	���J�[�\���ʒu
							cwchar *newLineText,	//	���s������
							bool nxln = true,		//	���ɍs��}��
							bool bCPP = false);
#endif


#endif		//_HEADER_BUFFERUTL_H
