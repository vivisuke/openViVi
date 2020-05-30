//----------------------------------------------------------------------
//
//			File:			"zenCoding.h"
//			Created:		10-8-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_ZENCODING_H
#define		_HEADER_ZENCODING_H

class TypeSettings;

void expandZenCodingHTML(
		//const QString	&indent,
		//int column,			//	�J�����ʒu
		QString &text,
		int &��Line,		//	�擪�ʒu����̍s����Ԃ�
		int &dOffset);		//	�擪�ʒu����̃I�t�Z�b�g��Ԃ�
bool expandZenCodingCPP(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	���s�e�L�X�g
		const QString	&indent,		//	�C���f���g�e�L�X�g
		QString &text,			//	�Q�ƃe�L�X�g
		int &��Line,		//	�擪�ʒu����̍s����Ԃ�
		int &dOffset);		//	�擪�ʒu����̃I�t�Z�b�g��Ԃ�
bool expandZenCodingPython(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	���s�e�L�X�g
		const QString	&indent,		//	�C���f���g�e�L�X�g
		QString &text,			//	�Q�ƃe�L�X�g
		int &��Line,		//	�擪�ʒu����̍s����Ԃ�
		int &dOffset);		//	�擪�ʒu����̃I�t�Z�b�g��Ԃ�
bool expandZenCodingRuby(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	���s�e�L�X�g
		const QString	&indent,		//	�C���f���g�e�L�X�g
		QString &text,			//	�Q�ƃe�L�X�g
		int &��Line,		//	�擪�ʒu����̍s����Ԃ�
		int &dOffset);		//	�擪�ʒu����̃I�t�Z�b�g��Ԃ�

//	�t�@�C������ǂݍ��� Zen-Coding ���́A
//	QMap<QString, QString> �ŊǗ������
//	---QMap<QString, ZenCodingItem> �ŊǗ������---
#if	0
struct ZenCodingItem {
	///QString	m_key;		//	�L�[�e�L�X�g
	QString	m_text;		//	�W�J�e�L�X�g
public:
	ZenCodingItem();
	ZenCodingItem(const ZenCodingItem &);
};
#endif

#endif		//_HEADER_ZENCODING_H
