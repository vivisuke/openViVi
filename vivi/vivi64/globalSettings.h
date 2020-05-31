//----------------------------------------------------------------------
//
//			File:			"globalSettings.h"
//			Created:		15-9-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_GLOBALSETTINGS_H
#define		_HEADER_GLOBALSETTINGS_H

#include <QString>
#include <QDebug>

typedef unsigned char uchar;
typedef const char cchar;
typedef const wchar_t cwchar;

class GlobalSettings
{
public:
	enum {
		IGNORE_CASE = 0,
		WHOLE_WORD_ONLY,
		REGEXP,
		LOOP_SEARCH,
		INC_SEARCH,
		GREP_SUB_DIR,
		VIEW_RT_BTM_PICTURE,
		SYNC_VERT_SCROLL,				//	画面分割時：垂直スクロール同期
		SYNC_HORZ_SCROLL,				//	画面分割時：水平スクロール同期
		IGNORE_MOC_FILE,				//	定義位置へジャンプ：moc_*.cpp を無視
		//STATEMENT_COMPLETION,		//	構文自動補完
		//WORD_COMPLETION,				//	単語自動補完
		//KEYWORD_COMPLETION,		//	キーワード自動補完
		WITH_BOM,							//	BOM付き
		MINI_MAP,						//	ミニマップ表示
		VI_COMMAND,					//	vi コマンド有効
		OUTPUT_VIEW,					//	OutputBar/OutputView
		GREP_VIEW,						//	OutputBar/GrepView
		N_BOOL,
		
		OUTPUT_FONT_SIZE = 0,
		PICTURE_OPACITY,
		PICTURE_SCALE,
		N_INT,
		
		//PICTURE1_PATH = 0,
		//PICTURE2_PATH,
		OUTPUT_FONT_NAME = 0,
		//MAIL_ADDRESS,
		//USER_NAME,
		//CERT_CODE,
		//PASS_CODE,
		ZEN_CODING_PATH,
		HTDOCS_ROOT,
		N_TEXT,
		
		CHAR_ENCODING = 0,
		DOC_TYPE,
		N_ENUM,
	};
	enum {
		CHAR_ENC_UTF_8 = 0,
		CHAR_ENC_UTF_16LE,
		CHAR_ENC_UTF_16BE,
		CHAR_ENC_SJIS,
		CHAR_ENC_EUC,
		N_CHAR_ENC,
		
		DOC_TYPE_DEFAULT = 0,
		DOC_TYPE_CPP,
		DOC_TYPE_CS,
		DOC_TYPE_CSS,
		DOC_TYPE_FS,
		DOC_TYPE_HLSL,
		DOC_TYPE_HTML,
		DOC_TYPE_JAVA,
		DOC_TYPE_JS,
		DOC_TYPE_LOG,
		DOC_TYPE_MARKDOWN,
		DOC_TYPE_PASCAL,
		DOC_TYPE_PERL,
		DOC_TYPE_PHP,
		DOC_TYPE_PYTHON,
		DOC_TYPE_RUBY,
		DOC_TYPE_SQL,
		DOC_TYPE_TXT,
		N_DOC_TYPE,
	};
	
public:
	GlobalSettings();
	~GlobalSettings() {};

public:
	void	writeSettings() const;
	bool		boolValue(int ix) const { return m_boolValues[ix]; }
	int		intValue(int ix) const { return m_intValues[ix]; }
	QString	textValue(int ix) const { return m_textValues[ix]; }
	int			enumValue(int ix) const { return m_enumValues[ix]; }
	QByteArray	defaultCodecName() const;
	bool	certified() const;

protected:
	void	init();

public:
	void	readSettings();
	void	setBoolValue(int ix, bool v) { m_boolValues[ix] = v; }
	void	setIntValue(int ix, int v) { m_intValues[ix] = v; }
	void	setTextValue(int ix, const QString &t) { m_textValues[ix] = t; }
	void	setEnumValue(int ix, int v) { m_enumValues[ix] = v; }

private:
	bool	m_boolValues[N_BOOL];
	int		m_intValues[N_INT];
	QString	m_textValues[N_TEXT];
	int			m_enumValues[N_ENUM];
};


#endif		//_HEADER_GLOBALSETTINGS_H
