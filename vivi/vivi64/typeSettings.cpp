//----------------------------------------------------------------------
//
//			File:			"typeSettings.cpp"
//			Created:		17-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include <QtGui>
#include <QSettings>
#include <QString>
#include "typeSettings.h"
#include <QDebug>

typedef const wchar_t cwchar;

struct KeyColor
{
	cchar	*m_key;			//	色タイプ名
	cchar	*m_color;		//	デフォルト色名
} keyColorTable[] = {
	{"Text",		"black"},
	{"BackGround",	"cornsilk"},
	{"SelText",		"black"},
	{"SelBG",		"#98FB98"},
	{"PreEditBG",	"#e0e0e0"},
	{"DelText",		"gray"},
	{"String",		"purple"},
	{"Digits",		"chocolate"},
	{"Comment",		"#00c000"},
	{"HTML",		"brown"},
	{"URL",		"brue"},
	{"KeyWord1",	"blue"},
	{"KeyWord2",	"darkcyan"},
	{"KeyWord3",	"darkred"},
	{"Tab",			"lightblue"},
	{"ZenSpace",	"lightblue"},
	{"NewLine",		"lightblue"},
	{"EOF",			"blue"},
	{"MarkBG",		"lightblue"},
	{"MatchedBG",	"gold"},
	{"MatchedText",	"orange"},
	{"CurWordBG",	"white"},
	{"MatchParenBG",	"lightgray"},
	{"Cursor",		"red"},
	{"LineCursor",		"red"},
	{"LineNum",			"black"},
	{"LineNumBG",		"#e0e0e0"},
	{"LineModified",	"yellow"},
	{"LineSaved",	"lightgreen"},
	{0, 0},
};
struct KeyBoolItem
{
	cchar	*m_key;			//	キー名
	bool	m_value;	//	
} keyBoolTable[] = {
	{"Tab",			true},
	{"ZenSpace",	true},
	{"NewLine",		true},
	{"LineNum",		true},
	{"MarkBG",		true},
	{"HTMLTag",		false},
	{"CurWordBG",	true},
	{"LineCursor",	true},
	{"JumpEOF",		false},
	{"IgnoreCaseKW1",	false},
	{"IgnoreCaseKW2",	false},
	{"KeyWord1Bold", true},
	{"KeyWord2Bold", false},
	{"KeyWord3Bold", false},
	{"LineBreakWinWidth", false},
	{"statementCompletion",	true},
	{"wordCompletion",	true},
	{"keywordCompletion",	true},
	{0, false}
};
struct KeyIntItem
{
	cchar	*m_key;		//	キー名
	int		m_value;	//	
} keyIntTable[] = {
	{"FontSize",	12},
	{"TabWidth",	4},
	{0, 0}
};
struct KeyTextItem
{
	cchar	*m_key;		//	キー名
	cwchar	*m_value;	//	
} keyTextTable[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L""},
	{"BlockCommentBeg",	L""},
	{"BlockCommentEnd",	L""},
	{"KeyWord1File",	L""},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableCPP[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"cpp.txt"},
	{"KeyWord2File",	L"Qt5.txt"},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L"/*,*/"},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableCS[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"cs.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L"/*,*/"},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableCSS[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L""},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"css.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L"/*,*/"},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableJAVA[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"java.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableRUBY[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"#"},
	{"BlockCommentBeg",	L"=begin"},
	{"BlockCommentEnd",	L"=end"},
	{"KeyWord1File",	L"ruby.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableFS[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"(*"},
	{"BlockCommentEnd",	L"*)"},
	{"KeyWord1File",	L"fs.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTablePYTHON[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"#"},
	{"BlockCommentBeg",	L""},
	{"BlockCommentEnd",	L""},
	{"KeyWord1File",	L"python.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableJS[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"JavaScript.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableHLSL[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"HLSL.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTablePASCAL[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"{"},
	{"BlockCommentEnd",	L"}"},
	{"KeyWord1File",	L"Pascal.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTablePERL[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"#"},
	{"BlockCommentBeg",	L"=pod"},
	{"BlockCommentEnd",	L"=cut"},
	{"KeyWord1File",	L"perl.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTablePHP[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"//"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"php.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableSQL[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L"--"},
	{"BlockCommentBeg",	L"/*"},
	{"BlockCommentEnd",	L"*/"},
	{"KeyWord1File",	L"SQL.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableHTML[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L""},
	{"BlockCommentBeg",	L"<!--"},
	{"BlockCommentEnd",	L"-->"},
	{"KeyWord1File",	L"html.txt"},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L"<p>"},
	{"ShortText1",		L"<br>"},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L"<a href=\"\">,</a>"},
	{"ShortTextSel1",	L"<b>,</b>"},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
struct KeyTextItem keyTextTableLOG[] = {
	{"FontName",	L"メイリオ"},
	{"LineComment",	L""},
	{"BlockCommentBeg",	L""},
	{"BlockCommentEnd",	L""},
	{"KeyWord1File",	L""},
	{"KeyWord2File",	L""},
	{"KeyWord3File",	L""},
	{"ShortText0",		L""},
	{"ShortText1",		L""},
	{"ShortText2",		L""},
	{"ShortText3",		L""},
	{"ShortText4",		L""},
	{"ShortText5",		L""},
	{"ShortText6",		L""},
	{"ShortText7",		L""},
	{"ShortText8",		L""},
	{"ShortText9",		L""},
	{"ShortTextSel0",	L""},
	{"ShortTextSel1",	L""},
	{"ShortTextSel2",	L""},
	{"ShortTextSel3",	L""},
	{"ShortTextSel4",	L""},
	{"ShortTextSel5",	L""},
	{"ShortTextSel6",	L""},
	{"ShortTextSel7",	L""},
	{"ShortTextSel8",	L""},
	{"ShortTextSel9",	L""},
	{0, false}
};
const KeyTextItem *tableForType(const QString &typeName)
{
	//const KeyTextItem *table = keyTextTable;
	if( typeName == "CPP" )
		return keyTextTableCPP;
	else if( typeName == "C#" )
		return keyTextTableCS;
	else if( typeName == "F#" )
		return keyTextTableFS;
	else if( typeName == "CSS" )
		return keyTextTableCSS;
	else if( typeName == "RUBY" )
		return keyTextTableRUBY;
	else if( typeName == "JAVA" )
		return keyTextTableJAVA;
	else if( typeName == "JS" )
		return keyTextTableJS;
	else if( typeName == "PERL" )
		return keyTextTablePERL;
	else if( typeName == "PASCAL" )
		return keyTextTablePASCAL;
	else if( typeName == "PHP" )
		return keyTextTablePHP;
	else if( typeName == "PYTHON" )
		return keyTextTablePYTHON;
	else if( typeName == "SQL" ) {
		//m_boolValues[IGNORE_CASE_KW1] = true;
		//m_boolValues[IGNORE_CASE_KW2] = true;
		return keyTextTableSQL;
	} else if( typeName == "HLSL" ) {
		return keyTextTableHLSL;
	} else if( typeName == "HTML" )
		return keyTextTableHTML;
	else if( typeName == "LOG" ) {
		//m_boolValues[JUMP_EOF_AT_OPEN] = true;
		return keyTextTableLOG;
	}
	return keyTextTable;
}

TypeSettings::TypeSettings(QString typeName, QString defaultExt)
	: m_defaultExt(defaultExt)
	//: m_typeName(typeName)
{
	//m_textValues.resize(N_TEXT);
	//for(int i = 0; i < N_TEXT; ++i)
	//	m_textValues += QString();
	//while( m_textValues.size() < N_TEXT ) m_textValues += QString();
	if( typeName.isEmpty() ) typeName = "Default";
	m_typeName = typeName;
	if (m_defaultExt.isEmpty()) {
		m_defaultExt = "txt";
	}
	init();
	if( !m_typeName.isEmpty() )
		readSettings();
}
void TypeSettings::init()
{
	for(int i = 0; i < N_COLOR; ++i)
		m_colors[i] = QColor(keyColorTable[i].m_color);
	for(int i = 0; i < N_BOOL; ++i)
		m_boolValues[i] = keyBoolTable[i].m_value;
	for(int i = 0; i < N_INT; ++i)
		m_intValues[i] = keyIntTable[i].m_value;
#if		1
	const KeyTextItem *table = tableForType(m_typeName);
	if( m_typeName == "SQL" ) {
		m_boolValues[IGNORE_CASE_KW1] = true;
		m_boolValues[IGNORE_CASE_KW2] = true;
	} else if( m_typeName == "LOG" ) {
		m_boolValues[JUMP_EOF_AT_OPEN] = true;
	}
#else
	//const KeyTextItem *table = keyTextTable;
	//if( m_typeName == "CPP" )
	//	table = keyTextTableCPP;
	//else if( m_typeName == "C#" )
	//	table = keyTextTableCS;
	//else if( m_typeName == "CSS" )
	//	table = keyTextTableCSS;
	//else if( m_typeName == "RUBY" )
	//	table = keyTextTableRUBY;
	//else if( m_typeName == "JAVA" )
	//	table = keyTextTableJAVA;
	//else if( m_typeName == "JS" )
	//	table = keyTextTableJS;
	//else if( m_typeName == "PHP" )
	//	table = keyTextTablePHP;
	//else if( m_typeName == "PYTHON" )
	//	table = keyTextTablePYTHON;
	//else if( m_typeName == "SQL" ) {
	//	m_boolValues[IGNORE_CASE_KW1] = true;
	//	m_boolValues[IGNORE_CASE_KW2] = true;
	//	table = keyTextTableSQL;
	//} else if( m_typeName == "HLSL" ) {
	//	table = keyTextTableHLSL;
	//} else if( m_typeName == "HTML" )
	//	table = keyTextTableHTML;
	//else if( m_typeName == "LOG" ) {
	//	m_boolValues[JUMP_EOF_AT_OPEN] = true;
	//	table = keyTextTableLOG;
	//}
#endif
	for(int i = 0; i < N_TEXT; ++i)
		m_textValues[i] = QString((QChar *)table[i].m_value);
}
int TypeSettings::colorKeyIndex(const QString &key) const
{
	for(int i = 0; i < N_COLOR; ++i) {
		if( key == QString(keyColorTable[i].m_key) )
			return i;
	}
	return -1;
}
int TypeSettings::indexOfBool(const QString &key) const
{
	for(int i = 0; i < N_BOOL; ++i) {
		if( key == QString(keyBoolTable[i].m_key) )
			return i;
	}
	return -1;
}
int TypeSettings::indexOfInt(const QString &key) const
{
	for(int i = 0; i < N_INT; ++i) {
		if( key == QString(keyIntTable[i].m_key) )
			return i;
	}
	return -1;
}
int TypeSettings::indexOfText(const QString &key) const
{
	for(int i = 0; i < N_TEXT; ++i) {
		if( key == QString(keyTextTable[i].m_key) )
			return i;
	}
	return -1;
}
cchar *TypeSettings::colorKey(int ix) const
{
	if( ix < 0 || ix >= N_COLOR )
		return 0;
	return keyColorTable[ix].m_key;
}
cchar *TypeSettings::viewItemKey(int ix) const
{
	if( ix < 0 || ix >= N_BOOL )
		return 0;
	return keyBoolTable[ix].m_key;
}
bool TypeSettings::save(const QString &fileName) const
{
	QFile file(fileName);
	if( !file.open(QFile::WriteOnly) )
		return false;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	for(int i = 0; i < N_COLOR; ++i) {
		const QString key = QString("color") + QString(keyColorTable[i].m_key);
		QByteArray ba = codec->fromUnicode(key + " = " + color(i).name() + "\n");
		file.write(ba);
	}
	for(int i = 0; i < N_BOOL; ++i) {
		const QString key = QString("bool") + QString(keyBoolTable[i].m_key);
		const QString vi = m_boolValues[i] ? "on" : "off";
		QByteArray ba = codec->fromUnicode(key + " = " + vi + "\n");
		file.write(ba);
	}
	for(int i = 0; i < N_INT; ++i) {
		const QString key = QString("int") + QString(keyIntTable[i].m_key);
		const QString vi = QString::number(m_intValues[i]);
		QByteArray ba = codec->fromUnicode(key + " = " + vi + "\n");
		file.write(ba);
	}
	for(int i = 0; i < N_TEXT; ++i) {
		const QString key = QString("text") + QString(keyTextTable[i].m_key);
		QByteArray ba = codec->fromUnicode(key + " = " + m_textValues[i] + "\n");
		file.write(ba);
	}
	return true;
}
void TypeSettings::writeSettings() const
{
	QSettings settings;
	for(int i = 0; i < N_COLOR; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/color") + QString(keyColorTable[i].m_key);
		settings.setValue(key, color(i).name());
	}
	for(int i = 0; i < N_BOOL; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/bool") + QString(keyBoolTable[i].m_key);
		settings.setValue(key, m_boolValues[i]);
	}
	for(int i = 0; i < N_INT; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/int") + QString(keyIntTable[i].m_key);
		settings.setValue(key, m_intValues[i]);
	}
	for(int i = 0; i < N_TEXT; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/text") + QString(keyTextTable[i].m_key);
		settings.setValue(key, m_textValues[i]);
	}
}
bool TypeSettings::load(const QString &fileName, bool onlyColor)
{
	QFile file(fileName);
	if( !file.open(QFile::ReadOnly) )
		return false;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	while( !file.atEnd() ) {
		QString txt = codec->toUnicode(file.readLine()).trimmed();
		int ix = txt.indexOf(" ");
		if( ix < 0 ) continue;
		if( txt.startsWith("color") ) {
			QString key = txt.mid(5, ix - 5);
			QString val = txt.mid(ix + 3);		//	skip " = ";
			//qDebug() << key << val;
			if( (ix = colorKeyIndex(key)) >= 0 )
				m_colors[ix] = QColor(val);
		} else if( !onlyColor ) {
			if( txt.startsWith("bool") ) {
				QString key = txt.mid(4, ix - 4);
				QString val = txt.mid(ix + 3);		//	skip " = ";
				if( (ix = indexOfBool(key)) >= 0 )
					m_boolValues[ix] = val == "on";
			} else if( txt.startsWith("int") ) {
				QString key = txt.mid(3, ix - 3);
				QString val = txt.mid(ix + 3);		//	skip " = ";
				if( (ix = indexOfInt(key)) >= 0 )
					m_intValues[ix] = val.toInt();
			} else if( txt.startsWith("text") ) {
				QString key = txt.mid(4, ix - 4);
				QString val = txt.mid(ix + 3);		//	skip " = ";
				if( (ix = indexOfText(key)) >= 0 )
					m_textValues[ix] = val;
			}
		}
	}
	file.close();
	loadKeyWords();
	return true;
}
void TypeSettings::readSettings()
{
	QSettings settings;
	for(int i = 0; i < N_COLOR; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/color") + QString(keyColorTable[i].m_key);
		const QString name = settings.value(key, QString(keyColorTable[i].m_color)).toString();
		m_colors[i] = QColor(name);
	}
	for(int i = 0; i < N_BOOL; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/bool") + QString(keyBoolTable[i].m_key);
		const bool b = name() == "HTML" && i == VIEW_HTMLTAG 
							|| name() == "LOG" && i == JUMP_EOF_AT_OPEN
							|| name() == "SQL" && (i == IGNORE_CASE_KW1 || i == IGNORE_CASE_KW2)
								? true : keyBoolTable[i].m_value;
		const bool vi = settings.value(key, b).toBool();
		m_boolValues[i] = vi;
	}
	for(int i = 0; i < N_INT; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/int") + QString(keyIntTable[i].m_key);
		const int v = settings.value(key, keyIntTable[i].m_value).toInt();
		m_intValues[i] = v;
	}
#if		1
	const KeyTextItem *table = tableForType(m_typeName);
#else
	//const KeyTextItem *table = keyTextTable;
	//if( m_typeName == "CPP" )
	//	table = keyTextTableCPP;
	//else if( m_typeName == "CSS" )
	//	table = keyTextTableCSS;
	//else if( m_typeName == "C#" )
	//	table = keyTextTableCS;
	//else if( m_typeName == "RUBY" )
	//	table = keyTextTableRUBY;
	//else if( m_typeName == "JAVA" )
	//	table = keyTextTableJAVA;
	//else if( m_typeName == "JS" )
	//	table = keyTextTableJS;
	//else if( m_typeName == "PHP" )
	//	table = keyTextTablePHP;
	//else if( m_typeName == "PYTHON" )
	//	table = keyTextTablePYTHON;
	//else if( m_typeName == "SQL" )
	//	table = keyTextTableSQL;
	//else if( m_typeName == "HLSL" )
	//	table = keyTextTableHLSL;
	//else if( m_typeName == "HTML" )
	//	table = keyTextTableHTML;
	//else if( m_typeName == "LOG" )
	//	table = keyTextTableLOG;
#endif
	qDebug() << N_TEXT;
	for(int i = 0; i < N_TEXT; ++i) {
		const QString key = QString("type/") + m_typeName + QString("/text") + QString(keyTextTable[i].m_key);
		const QString t = settings.value(key, QString((QChar *)table[i].m_value)).toString();
		m_textValues[i] = t;
	}
	loadKeyWords();
}
bool TypeSettings::loadKeyWords()
{
	bool rc1 = loadKeyWords(KEYWORD1_FILE, m_keyWord1Set, boolValue(IGNORE_CASE_KW1));
	bool rc2 = loadKeyWords(KEYWORD2_FILE, m_keyWord2Set, boolValue(IGNORE_CASE_KW2));
	return rc1 && rc2;
}
bool TypeSettings::loadKeyWords(int ix, QSet<QString> &kwSet, bool ic)
{
	kwSet.clear();
	QString fileName = textValue(ix);
	if( fileName.isEmpty() ) return false;
#ifdef	_DEBUG
	QDir dir("C:/user/sse.bin/keywords/");
#else
	QDir dir(qApp->applicationDirPath() + "/keywords/");
#endif
	QString fullPath = dir.absoluteFilePath(fileName);
	QFile file(fullPath);
	if( !file.open(QFile::ReadOnly) ) return false;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	while( !file.atEnd() ) {
		QString txt = codec->toUnicode(file.readLine()).trimmed();
		if( !txt.isEmpty() &&
			(txt[0].isLetter() || txt[0] == '_'
			 || txt.size() >= 2 && txt[0] == '#' && txt[1].isLetter()) )
		{
			if( ic ) txt = txt.toLower();
			kwSet += txt;
		}
	}
	return true;
}
bool TypeSettings::isKeyWord1(const QString &token) const
{
	if( !boolValue(IGNORE_CASE_KW1) )
		return m_keyWord1Set.contains(token);
	else
		return m_keyWord1Set.contains(token.toLower());
}
bool TypeSettings::isKeyWord2(const QString &token) const
{
	if( !boolValue(IGNORE_CASE_KW2) )
		return m_keyWord2Set.contains(token);
	else
		return m_keyWord2Set.contains(token.toLower());
}
QList<QString> TypeSettings::keywordList1() const
{
	return m_keyWord1Set.values();
}
QList<QString> TypeSettings::keywordList2() const
{
	return m_keyWord2Set.values();
}
