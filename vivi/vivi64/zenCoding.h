//----------------------------------------------------------------------
//
//			File:			"zenCoding.h"
//			Created:		10-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_ZENCODING_H
#define		_HEADER_ZENCODING_H

class TypeSettings;

void expandZenCodingHTML(
		//const QString	&indent,
		//int column,			//	カラム位置
		QString &text,
		int &ｄLine,		//	先頭位置からの行数を返す
		int &dOffset);		//	先頭位置からのオフセットを返す
bool expandZenCodingCPP(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	改行テキスト
		const QString	&indent,		//	インデントテキスト
		QString &text,			//	参照テキスト
		int &ｄLine,		//	先頭位置からの行数を返す
		int &dOffset);		//	先頭位置からのオフセットを返す
bool expandZenCodingPython(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	改行テキスト
		const QString	&indent,		//	インデントテキスト
		QString &text,			//	参照テキスト
		int &ｄLine,		//	先頭位置からの行数を返す
		int &dOffset);		//	先頭位置からのオフセットを返す
bool expandZenCodingRuby(
		const TypeSettings *typeSettings,
		const QString	&newLine,		//	改行テキスト
		const QString	&indent,		//	インデントテキスト
		QString &text,			//	参照テキスト
		int &ｄLine,		//	先頭位置からの行数を返す
		int &dOffset);		//	先頭位置からのオフセットを返す

//	ファイルから読み込んだ Zen-Coding 情報は、
//	QMap<QString, QString> で管理される
//	---QMap<QString, ZenCodingItem> で管理される---
#if	0
struct ZenCodingItem {
	///QString	m_key;		//	キーテキスト
	QString	m_text;		//	展開テキスト
public:
	ZenCodingItem();
	ZenCodingItem(const ZenCodingItem &);
};
#endif

#endif		//_HEADER_ZENCODING_H
