//----------------------------------------------------------------------
//
//			File:			"globalSettings.cpp"
//			Created:		15-9-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#include "globalSettings.h"
#include <QSettings>

struct KeyBoolItem
{
	cchar	*m_key;		//	�L�[��
	bool		m_value;	//	
};
static KeyBoolItem keyBoolTable[] = {
	{"IgnoreCase",	true},
	{"MatchWholeWordOnly",	false},
	{"RegExp",			false},
	{"LoopSearch",	true},
	{"IncrementalSearch",	true},
	{"GrepSubDir",	true},
	{"ViewRtBtmPicture",	true},
	{"SyncVertScroll",	false},
	{"SyncHorzScroll",	false},
	{"ignoreMocFile",	true},
	//{"statementCompletion",	true},
	//{"wordCompletion",	true},
	//{"keywordCompletion",	true},
	{"withBOM",	true},
	{"wholeMap",	true},
	{"viCommand",	false},
	{"grepView",	false},
	{"outputView",	false},
	{0, 0}
};
struct KeyTextItem
{
	cchar	*m_key;		//	�L�[��
	cwchar	*m_value;	//	
};
static KeyTextItem keyTextTable[] = {
	{"pict1Path",	L""},
	{"pict2Path",	L""},
	{"OutputFontName",	L"���C���I"},
	{"MailAddress", L""},
	{"UserName", L""},
	{"CertCode", L""},
	{"PassCode", L""},
	{"ZenCodingPath",	L""},
	{"htdocsRoot",	L""},
	{0, 0}
};
struct KeyEnumItem
{
	cchar	*m_key;		//	�L�[��
	int		m_value;	//	
} keyEnumTable[] = {
	{"CharEncoding",	0},
	{"DocType",		0},
	{0, 0}
};
struct KeyIntItem
{
	cchar	*m_key;		//	�L�[��
	int		m_value;	//	
};
static KeyIntItem keyIntTable[] = {
	{"OutputFontSize",	10},
	{"PictureOpacity",	100},
	{"PictureScale",	50},			//	%
	{0, 0}
};

GlobalSettings::GlobalSettings()
{
	init();
	readSettings();
}
void GlobalSettings::init()
{
	//for(int i = 0; i < N_COLOR; ++i)
	//	m_colors[i] = QColor(keyColorTable[i].m_color);
	//for(int i = 0; i < N_BOOL; ++i)
	//	m_boolValues[i] = keyBoolTable[i].m_value;
	for(int i = 0; i < N_INT; ++i)
		m_intValues[i] = keyIntTable[i].m_value;
	for(int i = 0; i < N_TEXT; ++i)
		m_textValues[i] = QString((QChar *)keyTextTable[i].m_value);
	for(int i = 0; i < N_ENUM; ++i)
		m_enumValues[i] = keyEnumTable[i].m_value;
}
void GlobalSettings::writeSettings() const
{
	QSettings settings;
	for(int i = 0; i < N_BOOL; ++i) {
		const QString key = QString("global/") + QString("/bool") + QString(keyBoolTable[i].m_key);
		settings.setValue(key, m_boolValues[i]);
	}
	for(int i = 0; i < N_INT; ++i) {
		const QString key = QString("global/") + QString("/int") + QString(keyIntTable[i].m_key);
		settings.setValue(key, m_intValues[i]);
	}
	for(int i = 0; i < N_TEXT; ++i) {
		const QString key = QString("global/") + QString("/text") + QString(keyTextTable[i].m_key);
		settings.setValue(key, m_textValues[i]);
	}
	for(int i = 0; i < N_ENUM; ++i) {
		const QString key = QString("global/") + QString("/enum") + QString(keyEnumTable[i].m_key);
		settings.setValue(key, m_enumValues[i]);
	}
}
void GlobalSettings::readSettings()
{
	QSettings settings;
	for(int i = 0; i < N_BOOL; ++i) {
		const QString key = QString("global/") + QString("/bool") + QString(keyBoolTable[i].m_key);
		const bool v = settings.value(key, keyBoolTable[i].m_value).toBool();
		m_boolValues[i] = v;
	}
	for(int i = 0; i < N_INT; ++i) {
		const QString key = QString("global/") + QString("/int") + QString(keyIntTable[i].m_key);
		const int v = settings.value(key, keyIntTable[i].m_value).toInt();
		m_intValues[i] = v;
	}
	for(int i = 0; i < N_TEXT; ++i) {
		const QString key = QString("global/") + QString("/text") + QString(keyTextTable[i].m_key);
		const QString t = settings.value(key, QString((QChar *)keyTextTable[i].m_value)).toString();
		m_textValues[i] = t;
	}
	for(int i = 0; i < N_ENUM; ++i) {
		const QString key = QString("global/") + QString("/enum") + QString(keyEnumTable[i].m_key);
		const int v = settings.value(key, keyEnumTable[i].m_value).toInt();
		m_enumValues[i] = v;
	}
}
QByteArray GlobalSettings::defaultCodecName() const
{
	switch( enumValue(CHAR_ENCODING) ) {
		default:
		case CHAR_ENC_UTF_8:	return QByteArray("UTF-8");
		case CHAR_ENC_UTF_16LE:	return QByteArray("UTF-16LE");
		case CHAR_ENC_UTF_16BE:	return QByteArray("UTF-16BE");
		case CHAR_ENC_SJIS:	return QByteArray("Shift_JIS");
		case CHAR_ENC_EUC:	return QByteArray("EUC");
	}
}
