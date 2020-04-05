//----------------------------------------------------------------------
//
//			File:			"typeSettings.h"
//			Created:		17-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_TYPESTG_H
#define		_HEADER_TYPESTG_H

//#include <QtGui>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>

typedef unsigned char uchar;
typedef const char cchar;

class TypeSettings
{
public:
	enum {
		TEXT = 0,
		BACK_GROUND,
		SEL_TEXT,
		SEL_BG,
		PREEDIT_BG,
		DEL_TEXT,
		STRING,
		DIGITS,
		COMMENT,
		HTML,
		URL,
		KEYWORD1,
		KEYWORD2,
		KEYWORD3,
		TAB,
		ZEN_SPACE,
		NEWLINE,
		EOF_MARK,
		MARK_BG,
		MATCHED_BG,
		MATCHED_TEXT,
		CUR_WORD_BG,
		MATCHE_PAREN_BG,
		CURSOR,
		LINE_CURSOR,
		LINENUM,
		LINENUM_BG,
		LINENUM_MODIFIED,
		LINENUM_SAVED,
		N_COLOR,

		VIEW_TAB = 0,
		VIEW_ZEN_SPACE,
		VIEW_NEWLINE,
		VIEW_LINENUM,
		VIEW_MARK_BG,
		VIEW_HTMLTAG,		//	HTML タグ強調
		VIEW_CUR_WORD_BG,
		VIEW_LINE_CURSOR,
		JUMP_EOF_AT_OPEN,		//	オープン時、EOFにジャンプ
		IGNORE_CASE_KW1,
		IGNORE_CASE_KW2,
		KEYWORD1_BOLD,
		KEYWORD2_BOLD,
		KEYWORD3_BOLD,
		LINE_BREAK_WIN_WIDTH,		//	画面右端で折り返し
		STATEMENT_COMPLETION,		//	構文自動補完
		WORD_COMPLETION,				//	単語自動補完
		KEYWORD_COMPLETION,		//	キーワード自動補完
		N_BOOL,

		FONT_NAME = 0,
		LINE_COMMENT,
		BLOCK_COMMENT_BEG,
		BLOCK_COMMENT_END,
		KEYWORD1_FILE,
		KEYWORD2_FILE,
		KEYWORD3_FILE,
		SHORT_TEXT_0,
		SHORT_TEXT_1,
		SHORT_TEXT_2,
		SHORT_TEXT_3,
		SHORT_TEXT_4,
		SHORT_TEXT_5,
		SHORT_TEXT_6,
		SHORT_TEXT_7,
		SHORT_TEXT_8,
		SHORT_TEXT_9,
		SHORT_TEXT_SEL_0,
		SHORT_TEXT_SEL_1,
		SHORT_TEXT_SEL_2,
		SHORT_TEXT_SEL_3,
		SHORT_TEXT_SEL_4,
		SHORT_TEXT_SEL_5,
		SHORT_TEXT_SEL_6,
		SHORT_TEXT_SEL_7,
		SHORT_TEXT_SEL_8,
		SHORT_TEXT_SEL_9,
		N_TEXT,

		FONT_SIZE = 0,
		TAB_WIDTH,
		N_INT,
	};

public:
	TypeSettings(QString typeName = QString(), QString defaultExt = QString());
	~TypeSettings() {};

public:
	QString	name() const { return m_typeName; }
	int		nColor() const { return N_COLOR; }
	int		nBool() const { return N_BOOL; }
	int		colorKeyIndex(const QString &) const;
	cchar	*colorKey(int ix) const;
	QColor	color(int ix) const { return m_colors[ix]; }
	cchar	*viewItemKey(int ix) const;
	bool	boolValue(int ix) const { return m_boolValues[ix]; }
	int		intValue(int ix) const { return m_intValues[ix]; }
	QString	textValue(int ix) const { return m_textValues[ix]; }
	int		indexOfBool(const QString &) const;
	int		indexOfInt(const QString &) const;
	int		indexOfText(const QString &) const;
	bool	isKeyWord1(const QString &) const;
	bool	isKeyWord2(const QString &) const;
	void	writeSettings() const;
	bool	save(const QString &) const;
	QList<QString>	keywordList1() const;
	QList<QString>	keywordList2() const;
	QString	defaultExt() const { return m_defaultExt; }		//	デフォルト拡張子

public:
	void	reset() { init(); }
	void	setColor(int ix, const QColor &col) { m_colors[ix] = col; }
	void	setBoolValue(int ix, bool b) { m_boolValues[ix] = b; }
	void	setIntValue(int ix, int v) { m_intValues[ix] = v; }
	void	setTextValue(int ix, const QString &t) { m_textValues[ix] = t; }
	void	readSettings();
	bool	load(const QString &, bool onlyColor = false);
	bool	loadKeyWords();

protected:
	void	init();
	bool	loadKeyWords(int ix, QSet<QString> &, bool);

private:
	QString	m_typeName;
	QString	m_defaultExt;
	QColor	m_colors[N_COLOR];
	bool	m_boolValues[N_BOOL];
	int		m_intValues[N_INT];
	//QStringList		m_textValues;
	QString	m_textValues[N_TEXT];
	QSet<QString>	m_keyWord1Set;
	QSet<QString>	m_keyWord2Set;
};

#endif		//_HEADER_TYPESTG_H
