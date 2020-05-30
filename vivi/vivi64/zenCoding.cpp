#include <QLocale>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QTextCodec>

#include "typeSettings.h"
#include "zenCoding.h"
#include "tokenizer.h"
#include "EditView.h"
#include "Document.h"
#include "mainwindow.h"
#include "TextCursor.h"
#include "globalSettings.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isAlpha(wchar_t ch)
{
	return ch < 0x100 && isalpha(ch);
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
bool isAlnum(const QString &text)
{
	for (int i = 0; i < text.size(); ++i) {
		if( !isAlnum(text[i]) )
			return false;
	}
	return true;
}
/**
*/
int parseRepeatCount(QString &text)
{
	QString::const_iterator ibegin = text.begin();
	QString::const_iterator itr = text.end();
	if( ibegin == itr || !(*--itr).isNumber() ) return 1;
	while( ibegin != itr && (*--itr).isNumber() ) NULL;
	if( ibegin == itr || (*itr) != '*' ) return 1;
	const int n = text.mid(itr - ibegin + 1).toInt();
	text = text.left(itr - ibegin);
	return n;
#if 0
	cchar *ptr = (cchar *)text;
	cchar *endptr = ptr + text.GetLength();
	if( ptr == endptr || !isdigit(*--endptr) ) return 1;
	while( ptr < endptr && isdigit(*--endptr) ) NULL;
	if( ptr == endptr || *endptr != '*' ) return 1;
	const int n = atoi(endptr+1);
	text = text.Left(endptr - ptr);
	return n;
#endif
}
/**
	#hoge → id="hoge"
	.hoge → class="hoge"
	.one.two → class="one two"
	#one.two → id="one" class="two"
*/
QString parseIdClass(QString &text,
						int &offset)	//	展開後のカーソル変移
{
	offset = 0;
	QString open, close;
	QString::const_iterator iend = text.end();
	QRegExp sorpExp("[#\\.]");
	int ix = sorpExp.indexIn(text);	//	# or . 位置
	if( ix < 0 ) {
		open = close = text;
	} else {
		open = close = text.left(ix);
		//bool withClass = false;
		QChar sep = ' ';
		while( ix < text.length() ) {
			int ix2 = sorpExp.indexIn(text, ix + 1);
			if( ix2 < 0 ) ix2 = text.length();
			if( text[ix] != sep ) {
				if( sep != ' ' )
					open += "\"";
				open += (ix >= 0 && text[ix] == '#') ? " id=\"" : " class=\"";
				sep = text[ix];
			} else
				open += " ";
			open += text.mid(ix + 1, ix2 - ix - 1);
			ix = ix2;
		}
		if( sep != ' ' )
			open += '"';
		text = close;
	}
	if( close.isEmpty() && !open.isEmpty() && open[0] == ' ' ) {
		open = "div" + open;
		text = "div";
	}
	if( close.toLower() == "a" ) {
		offset = -2;			//	末尾からの変位
		open += " href=\"\"";
	}
	return open;
}

bool isInlineTag(const QString &tag)
{
	return !tag.compare("a", Qt::CaseInsensitive)
			|| !tag.compare("b", Qt::CaseInsensitive)
			|| !tag.compare("i", Qt::CaseInsensitive)
			|| !tag.compare("u", Qt::CaseInsensitive)
			|| !tag.compare("font", Qt::CaseInsensitive)
			//|| !tag.compare("li", Qt::CaseInsensitive)
			|| !tag.compare("span", Qt::CaseInsensitive);
}

QString expandAtr(const QString atr)
{
	if( atr.isEmpty() ) return QString();
	QString text;
	QStringList lst = atr.split(' ');
	for (int i = 0; i < lst.size(); ++i) {
		text += " ";
		int ix = lst[i].indexOf('=');
		if( ix < 0 )
			text += lst[i] + "=\"\"";
		else {
			text += lst[i].left(ix) + "=";
			QString v = lst[i].mid(ix+1);
			if( v.isEmpty() || v == "\"" || v == "\"\"" )
				text += "\"\"";
			else {
				if( v[0] != '\"' )
					v = "\"" + v;
				if( v[v.size() - 1] != '\"' )
					v += "\"";
				text += v;
			}
		}
	}
	return text;
}
/*
		<ZenCoding> ::= <Element> | <Element> ">" <ZenCoding>
		<Element> ::= <Fuctor> | <Fuctor> "+" <Element>
		<Fuctor> ::= <Term> | <Term> "*" <digits> 
		<Term> ::= <Tag> | "ul+" | "(" <ZenCoding> ")"
		<Tag> ::= <TagText> { "#" <id> | "." <class> } { "[" <atr> "]" }
*/
//----------------------------------------------------------------------
bool szhTagPlus(QString &text,
		const QString &item1,
		//const QString &indent,	//	インデントテキスト
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	if( item1 == "ul+" ) {
		text += "<ul>\n\t<li></li>\n</ul>";
		dLine += 1;
		dOffset += /*indent.length() +*/ 5;
		return true;
	}
	if( item1 == "ol+" ) {
		text += "<ol>\n\t<li></li>\n</ol>";
		dLine += 1;
		dOffset += /*indent.length() +*/ 5;
		return true;
	}
	if( item1 == "dl+" ) {
		text += "<dl>\n\t<dt></dt>\n\t<dd></dd>\n</dl>";
		dLine += 1;
		dOffset += /*indent.length() +*/ 5;
		return true;
	}
	if( item1 == "table+" ) {
		text += "<table>\n\t<tr>\n\t\t<td></td>\n\t</tr>\n</table>";
		dLine += 2;
		dOffset += /*indent.length() +*/ 6;
		return true;
	}
	return false;
}
bool zchHTML(
		QString &text,
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	QString locale = QLocale::system().name();
	int ix = locale.indexOf('_');
	QString lang = ix >= 0 ? locale.left(ix) : locale;
	if( text == "html:5" ) {
		text = "<!DOCTYPE HTML>\n"
					"<html lang=\"" + lang + "\">\n"
					"<head>\n"
					"\t<meta charset=\"UTF-8\">\n"
					"\t<title></title>\n"
					"</head>\n"
					"<body>\n"
					"\t\n"
					"</body>\n</html>";
		dLine = 7;
		dOffset = 1;
		return true;
	}
	if( text == "html:4s" ) {
		text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"
					"<html lang=\"" + lang + "\">\n"
					"<head>\n"
					"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
					"\t<title></title>\n"
					"</head>\n"
					"<body>\n"
					"\t\n"
					"</body>\n</html>";
		dLine = 7;
		dOffset = 1;
		return true;
	}
	if( text == "html:4t" ) {
		text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
					"<html lang=\"" + lang + "\">\n"
					"<head>\n"
					"\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
					"\t<title></title>\n"
					"</head>\n"
					"<body>\n"
					"\t\n"
					"</body>\n</html>";
		dLine = 7;
		dOffset = 1;
		return true;
	}
	return false;
}
QString zchTerm(int k,
		QString item2,
		QString &subText,
		bool	&inlineTag,		//	in：subText がインラインかどうか、out：返すテキストがインラインかどうか
		//const QString &indent,	//	インデントテキスト
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	if( item2 == "bq" ) item2 = "backquote";
	else if( item2 == "BQ" ) item2 = "BACKQUOTE";
	QString atr;		//	[ ] の中身
	int ix = item2.indexOf('[');
	if( ix >= 0 && item2[item2.size() - 1] == ']' ) {
		atr = item2.mid(ix+1, item2.size() - ix - 2);
		item2 = item2.left(ix);
	}
	QString body;		//	{ } の中身
	ix = item2.indexOf('{');
	if( ix >= 0 && item2[item2.size() - 1] == '}' ) {
		body = item2.mid(ix+1, item2.size() - ix - 2);
		item2 = item2.left(ix);
	}
	QString term;
	int n = parseRepeatCount(item2);		//	繰り返し回数
	int offset = 0;
	QString openItem = parseIdClass(item2, offset);
	const bool isInline = isInlineTag(item2);
	QString atxt = expandAtr(atr);
	//if( openItem.isEmpty() && item2.isEmpty() && !atxt.isEmpty() ) {
	//	openItem = item2 = "div";	//	タグ名が省略された場合
	//}
	QString lw = item2.toLower();
	if( lw == "br" || lw == "hr" ) {
		term = "<" + item2 + " />";
		if( !k ) {
			dLine = 0;
			dOffset = term.length();
		}
	} else if( lw == "an" ) {
		term = "<a name=\"\">" + body + "</a>";
		if( !k ) {
			dLine = 0;
			dOffset = strlen("<a name=\"");
		}
	} else if( lw == "img" ) {
		term = "<img src=\"\" alt=\"\" />";
		if( !k ) {
			dLine = 0;
			dOffset = term.length() - 11;
		}
	} else if( (subText.isEmpty() || inlineTag) && lw != "pre" ) {
		term = "<" + openItem + atxt + ">" + body + subText + "</" + item2 + ">";
		if( !k ) {
			dLine = 0;
			if( subText.isEmpty() ) {
				if( offset >= 0 )
					dOffset = openItem.length() + atxt.length() + offset + 2;
				else
					dOffset = openItem.length() + atxt.length() + subText.length() + offset + 2;
			} else {
				dOffset += openItem.length() + atxt.length() + 2;
			}
			//dOffset += body.size();
		}
	} else {
		QString t = lw == "pre" ? "" : "\t";
		subText.replace("\n", "\n" + t);
		term = "<" + openItem + atxt + ">" + "\n"
				/*+ indent*/ + t + subText + "\n"
				/*+ indent*/ + "</" + item2 + ">";
		dLine += 1;
		dOffset += t.length();
		//if( !k )
		//	dOffset += indent.length();
	}
	//isInline = isInlineTag(item);
	QString term2;
	for(int i = 1; i <= n; ++i) {
		QString t = term;
		QString nstr = QString::number(i);
		t.replace("$", nstr);
		if( term2.isEmpty() )
			term2 = t;
		else
			term2 += "\n" /*+ indent */+ t;
	}
	inlineTag = isInline;
	return term2;
}
//----------------------------------------------------------------------
//	丸括弧に対応しつつ '>' で分割
QStringList splitGt(const QString &text)
{
	QStringList lst;
	int nest = 0;		//	'(' のネスト
	int first = 0;
	for (int i = 0; i < text.size(); ++i) {
		switch( text[i].unicode() ) {
			case '>':
				if( !nest ) {
					lst += text.mid(first, i - first);
					first = i + 1;
				}
				break;
			case '(':
				++nest;
				break;
			case ')':
				if( nest != 0 ) --nest;
				break;
		}
	}
	if( first != text.size() )
		lst += text.mid(first);
	return lst;
}
void expandZenCodingHTML(
		//const QString &indent,	//	インデントテキスト
		QString &text,
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	dLine = 0;
	dOffset = 0;
	if( zchHTML(text, dLine, dOffset) ) return;
	//	undone: (x>y)+z 対応
	QStringList vs = splitGt(text);	//	丸括弧に対応しつつ '>' で分割
	//QStringList vs = text.split(">");
	text.clear();
	bool inlineTag = false;
	for(int i = vs.size(); --i >= 0;) {
		QString item1 = vs[i];
		if( szhTagPlus(text, item1, /*indent,*/ dLine, dOffset) )
			continue;		//	ul+, ol+, dd+, table+ 展開した場合
		QStringList vs2 = item1.split("+");
		QString subText = text;
		text.clear();
		for(int k = vs2.size(); --k >= 0;) {
			QString item2 = vs2[k];
			if( item2.isEmpty() ) continue;
			QString term2;
			if( item2[0] == '(' && item2[item2.size()-1] == ')' ) {
				term2 = item2.mid(1, item2.size()-2);
				expandZenCodingHTML(/*indent,*/ term2, dLine, dOffset);
			} else
				term2 = zchTerm(k, item2, subText, inlineTag, /*indent,*/ dLine, dOffset);
			if( text.isEmpty() )
				text = term2;
			else
				text = term2 + "\n" + text;
			subText.clear();	//	次の+ループのため
		}
	}
}
bool expandZenCodingPython(
		const TypeSettings *typeSettings,
		const QString &newLine,		//	改行テキスト
		const QString &indent,	//	インデントテキスト
		QString &text,
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	if( text.isEmpty() ) return false;
	QStringList lst = text.split(' ');
	dLine = dOffset = 0;
	if( lst[0] == "f" ) {
		if( lst.size() == 1 ) {
			text = "for i in range(N):" + newLine;
		} else {
			lst.pop_front();
			QString var = "i";
			QString list = "N";
			if( !lst.isEmpty() && lst[0][0].isLower() && isAlnum(lst[0]) )
			{
				var = lst[0];
				lst.pop_front();
			}
			if( !lst.isEmpty() ) {
				list = lst.join(" ");
			}
			text = "for " + var + " in range(" + list + "):" + newLine;
		}
		text += indent + "\t";
		dLine = 1;
		dOffset = indent.size() + 1;
		return true;
	}
	return false;
}
bool expandZenCodingRuby(
		const TypeSettings *typeSettings,
		const QString &newLine,		//	改行テキスト
		const QString &indent,	//	インデントテキスト
		QString &text,
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	if( text.isEmpty() ) return false;
	QStringList lst = text.split(' ');
	dLine = dOffset = 0;
	if( lst[0] == "f" ) {
		if( lst.size() == 1 ) {
			text = "for i in 1..N do" + newLine;
		} else {
			lst.pop_front();
			QString var = "i";
			QString list = "1..N";
			if( !lst.isEmpty() && lst[0][0].isLower() && isAlnum(lst[0]) )
			{
				var = lst[0];
				lst.pop_front();
			}
			if( lst.size() == 1
				&& (lst[0][0].isUpper() && isAlnum(lst[0])
						|| lst[0][0].isNumber()) )
			{
				list = "1.." + lst[0];
			} else if( !lst.isEmpty() ) {
				list = lst.join(" ");
			}
			text = "for " + var + " in " + list + newLine;
		}
		text += indent + "\t" + newLine;
		text += "end";
		dLine = 1;
		dOffset = indent.size() + 1;
		return true;
	}
	return false;
}
bool expandZenCodingCPP(
		const TypeSettings *typeSettings,
		const QString &newLine,		//	改行テキスト
		const QString &indent,	//	インデントテキスト
		QString &text,
		int &dLine,		//	先頭位置からの行数
		int &dOffset)		//	先頭位置からのオフセットを返す
{
	if( text.isEmpty() ) return false;
	QStringList lst = text.split(' ');
	dLine = dOffset = 0;
	const QString typeName = typeSettings->name();
	QString typeStr = typeName == "JS" ? "var " :
								typeName == "PHP" ? "" :
								"int ";		//	C/C++, C#, Java
	if( lst[0] == "f" || lst[0] == "for" ) {
		if( lst.size() == 1 ) {
			if( typeName == "PHP" )
				text = "for ($i = 0; $i < N; ++$i) {" + newLine;
			else
				text = "for (" + typeStr + "i = 0; i < N; ++i) {" + newLine;
		} else {
			bool binit = false;		//	初期値指定有り
			bool blast = false;		//	最終値指定有り
			bool bop = false;		//	演算子指定有り
			bool inc = true;
			if( lst[lst.size() - 1] == "-" ) {		//	最後がマイナスだった場合
				inc = false;
				lst.pop_back();
			}
			QString var = typeName == "PHP" ? "$i" : "i";
			int ix = 1;
			if( ix < lst.size()
				&& (lst[ix][0].isLower() && isAlnum(lst[ix])
						|| typeName == "PHP" && lst[ix][0] == '$') )
			{
				var = lst[ix++];
				if( typeName == "PHP" && var[0] != '$' )
					var = '$' + var;
			}
			QString init = inc ? "0" : "N";
			QString last = inc ? "N" : "0";
			if( ix < lst.size()
				&& (lst[ix][0].isLower() || !inc && lst[ix][0].isUpper()|| lst[ix][0].isDigit()) )
			{
				//	初期値指定
				init = lst[ix++];
				binit = true;
			}
			QString op;
			if( ix < lst.size() &&
				(lst[ix] == "<=" || lst[ix] == "<"
				|| lst[ix] == ">=" || lst[ix] == ">"
				|| lst[ix] == "!=" || lst[ix] == "==") )
			{
				op = lst[ix++];
				bop = true;
			}
			if( ix < lst.size() && (isAlphaOrUnderbar(lst[ix][0]) || lst[ix][0].isDigit()) ) {
				last = lst[ix++];
				blast = true;
			}
			if( init[0].isDigit() && last[0].isDigit() && init.toInt() > last.toInt() ) {
				inc = false;
			}
			if( !bop && binit && !blast && inc ) {
				last = init;
				init = "0";
			}
			if( op.isEmpty() ) {
				if (inc)
					op = "<";
				else
					op = ">=";
			}
			text = "for (" + typeStr + " " + var + " = " + init + "; ";
			if (!inc) {
				text += "--";
			}
			text += var + " " + op + " " + last + "; ";
			if (inc) {
				text += "++" + var;
			}
			text += ") {" + newLine;
		}
		text += indent + "\t" + newLine;
		text += indent + "}";
		dLine = 1;
		dOffset = indent.size() + 1;
		return true;
	}
	if( lst.size() == 2 && lst[0] == "}" && lst[1] == "e" ) {		//	} e の場合
		text = "} else {" + newLine;
		text += indent + "\t" + newLine;
		text += indent + "}";
		dLine = 1;
		dOffset = indent.size() + 1;
		return true;
	}
	if( lst[0] == "ei" || lst.size() >= 2 && lst[0] == "}" && lst[1] == "ei" ) {
		if (lst[0] == "}") {
			lst.pop_front();
			text = "} ";
		}
		if (lst.size() == 1) {
			text += "else if (true) {" + newLine;
		} else {
			lst.pop_front();
			text += "else if (" + lst.join(" ") + ") {" + newLine;
		}
		text += indent + "\t" + newLine;
		text += indent + "}";
		dLine = 1;
		dOffset = indent.size() + 1;
		return true;
	}
	if( lst[0] == "r" ) {
		text = "return";
		if( lst.size() != 1 ) {
			lst.pop_front();
			text += " " + lst.join(" ");
		}
		text += ";";
		dOffset = text.size();
		return true;
	}
	return false;
}
bool EditView::zenCodingFromFile(const QString &indent,
													const QString &keyText,
													const pos_t pos)		//	keyText 開始位置
{
	QString fileName = globSettings()->textValue(GlobalSettings::ZEN_CODING_PATH);
	if( !fileName.isEmpty() ) {
		if (zenCodingFromFile(fileName, indent, keyText, pos)) {
			return true;
		}
	}
#if	_DEBUG
	fileName = "G:/bin/sse64/zenCoding.txt";
#else
	fileName = qApp->applicationDirPath() + "/zenCoding.txt";
#endif
	return zenCodingFromFile(fileName, indent, keyText, pos);
}
bool EditView::zenCodingFromFile(const QString &fileName,
													const QString &indent,
													const QString &keyText,
													const pos_t pos)		//	keyText 開始位置
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
#if	0
	QMap<QString, QString> zcMap;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	while( !file.atEnd() ) {
		QString buf = codec->toUnicode(file.readLine());
		int ix = buf.indexOf('\t');
		if( ix < 0 ) continue;
		QString key = buf.left(ix);
		buf = buf.mid(ix+1);
		zcMap[key] = buf;
	}
	if( !zcMap.contains(keyText) )
		return false;
	QString text = zcMap[keyText];
#else
	QStringList lst = keyText.split(' ');
	QString text;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	for (;;) {
		if( file.atEnd() )
			return false;
		QString buf = codec->toUnicode(file.readLine());
		if( buf.isEmpty() || buf[0].isSpace() )
			continue;	//	空行 or 先頭が空白類の場合はスキップ
		int ix = buf.indexOf('\t');
		if( ix < 0 ) continue;
		if( buf.left(ix) == lst[0] ) {
			while( buf[ix].isSpace() ) ++ix;
			text = buf.mid(ix).trimmed();
			//int ix2 = text.size();
			//while( ix2 > 0 && text[ix2-1].isSpace() ) --ix2;
			//text = text.left(ix2);
			break;
		}
	}
	lst.pop_front();
#endif
	//	\n を改行に変換
	//	\c は展開後のカーソル位置
	//	\{default\} を展開、\{ \} の数が引数より少ない場合は、残り引数全部で置き換える
	QString text2;
	text2.reserve(text.size());
	int spos = 0;		//	行頭位置
	int dLine = -1;		//	展開行からの行数
	int dOffset = -1;		//	行頭からのカーソル位置
	int nNewLine = 0;		//	改行数
	//int ixlst = 0;
	for (int i = 0; i < text.size(); ) {
		QChar ch = text[i++];
		if( ch != '\\' ) {
			text2 += ch;
		} else {
			QChar ch = text[i++];
			if( ch == 'n' ) {
				text2 += newLineText();
				spos = text2.size();
				text2 += indent;
				++nNewLine;
			} else if( ch == 't' ) {
				text2 += "\t";
			} else if( ch == 's' ) {
				text2 += " ";
			} else if (ch == 'c') {
				dLine = nNewLine;
				dOffset = text2.size() - spos;
			} else if( ch == '{' ) {
				int ix = text.indexOf("\\}", i);
				QString deftext;
				if( ix < 0 ) {	//	\} が無い場合
					deftext = text.mid(i);
					text.clear();
				} else {
					deftext = text.mid(i, ix - i);
					text = text.mid(ix+2);
					i = 0;
				}
				if( !lst.isEmpty() ) {
					if (text.indexOf("\\{") < 0) {	//	\{ \} がもう無い場合
						text2 += lst.join(" ");
					} else {
						text2 += lst[0];
						lst.pop_front();		//	引数先頭削除
					}
				} else
					text2 += deftext;
			} else
				text2 += ch;
		}
	}
	//text.replace("\\n", newLineText() + indent);
	m_textCursor->setPosition(pos, TextCursor::KEEP_ANCHOR);
	m_textCursor->insertText(text2);
	if( dLine >= 0 ) {
		m_textCursor->setPosition(pos);	//	最初の位置
		m_textCursor->movePosition(TextCursor::DOWN, TextCursor::MOVE_ANCHOR, dLine);
		m_textCursor->movePosition(TextCursor::BEG_LINE);
	}
	if( dOffset < 0 )
		m_textCursor->movePosition(TextCursor::END_LINE);
	else
		m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::MOVE_ANCHOR, dOffset);
	makeCursorInView();
	return true;
}
QChar checkStatement(QString &keyText, int &ix)
{
	int i = keyText.lastIndexOf(QRegExp("[\\)\\{]"));
	if( i < 0 ) return QChar(0);
	QChar ch = keyText[i];
	++i;
	while( i < keyText.size() && (keyText[i] == ' ' || keyText[i] == '\t') )
		++i;
	if( i < keyText.size() && isAlpha(keyText[i].unicode()) ) {
		keyText = keyText.mid(i);
		ix += i;
	}
	return ch;
}
//	x y[a b] の様な場合、有効な範囲を切り出す
//	tag{text} もサポート
int checkRange(QString &text, int &i)
{
	for (int k = text.size(); --k >= 0; ) {
		if( text[k] == ' ' || text[k].unicode() >= 0x80 ) {
			i += k + 1;
			text = text.mid(k + 1);
			return k+1;
		}
		if( text[k] == ']' ) {
			k = text.lastIndexOf('[', k);
			if( k < 0 ) return 0;
		}
		if( text[k] == '}' ) {
			k = text.lastIndexOf('{', k);
			if( k < 0 ) return 0;
		}
	}
	return 0;
}
void EditView::zenCoding()
{
	closeAutoCompletionDlg();
	const pos_t pos = m_textCursor->position();
	const int ln = positionToLine(pos);
	const pos_t ls = lineStartPosition(ln);
	int ix = ls;
	while( ix < pos && isSpaceChar(charAt(ix)) ) {
		++ix;
		//indentText += QString(QChar(charAt(i++)));
	}
	if( ix >= pos ) {	//	行頭の場合
		if( ln != 0 && typeSettings()->name() == "HTML" ) {
			QString text;
			int l = ln;
			do {
				if( --l < 0 ) return;
				text = document()->lineText(l).trimmed();
			} while( text.isEmpty() );
			int k;
			if( /*!text.isEmpty() &&*/ text[text.size() - 1] == '>'
				&& (k = text.lastIndexOf('<')) >= 0 )
			{
				QString tag = text.mid(k+1, text.size() - k - 2);
				if( !tag.isEmpty() ) {
					bool upper = false;
					if( tag[0].isUpper() )
						tag = tag.toLower();
					if( tag == "table" || tag == "/tr" ) {
						text = "<tr></tr>";
						if( upper )
							text = text.toUpper();
						m_textCursor->insertText(text);
						m_textCursor->setPosition(pos + 4);
					} else if( tag == "/tr" ) {
						text = "<tr></tr>";
						if( upper )
							text = text.toUpper();
						m_textCursor->insertText(text);
						m_textCursor->setPosition(pos + 4);
					} else if( tag == "/th" ) {
						text = "<th></th>";
						if( upper )
							text = text.toUpper();
						m_textCursor->insertText(text);
						m_textCursor->setPosition(pos + 4);
					} else if( tag == "tr" || tag == "/td" ) {
						text = "<td></td>";
						if( upper )
							text = text.toUpper();
						m_textCursor->insertText(text);
						m_textCursor->setPosition(pos + 4);
					} else if( tag == "ol" || tag == "ul" || tag == "/li" ) {
						text = "<li></li>";
						if( upper )
							text = text.toUpper();
						m_textCursor->insertText(text);
						m_textCursor->setPosition(pos + 4);
					}
				}
			}
		}
		return;
	}
	QString indentText = getText(*buffer(), ls, ix - ls);
	QString keyText = getText(*buffer(), ix, pos - ix);
	if( typeSettings()->name() != "HTML" ) {
		QChar ch;
		if( isCppType() )
			ch = checkStatement(keyText, ix);		//	{ がある場合は、それ以前を消去
		if( zenCodingFromFile(indentText, keyText, ix) )
			return;
		if( zenCodingFor(indentText, keyText, ix, ch) )
			return;
	}
	if( typeSettings()->name() != "HTML" )
		return;
	int ixl = m_textCursor->positionInLine();
	//pos = m_textCursor->position();
	//int ln = dock->positionToLine(pos);
	QString blockText = document()->lineText(ln);
#if	1
	int i = 0;
	for (;;++i) {
		if( i == ixl ) return;
		if( blockText[i] != ' ' && blockText[i] != '\t' )
			break;
	}
#else
	int i = ixl;
	while( i > 0 && blockText[i - 1] != ' ' && blockText[i - 1] != '\t'
		&& blockText[i - 1] != '\"' && blockText[i - 1] != '\'')
	{
		--i;
	}
	if( i == ixl ) return;
#endif
	QString indent = blockText.left(i);
	for(int k = 0; k < indent.length(); ++k)
		if( indent[k] != ' ' && indent[k] != '\t' ) indent[k] = ' ';
	QString text = blockText.mid(i, ixl - i);
	int j;
	int t1 = -1, t2 = -1;		//	< > 位置
	int d = 0;		//	text 内の有効位置（i は行内の有効位置）
	//	タグ部分を削除
	while( (j = text.indexOf('<', d)) >= 0 ) {
		t1 = j + 1;
		d = j + 1;
		if( (j = text.indexOf('>', d)) >= 0 ) {
			t2 = j;
			d = j + 1;
		}
	}
	QString lastTag;
	bool upper = false;
	if( d != 0 ) {
		if( t1 >= 0 && t2 >= 0 ) {
			lastTag = text.mid(t1, t2 - t1);
			int ix = lastTag.indexOf(' ');
			if( ix >= 0 )
				lastTag = lastTag.left(ix);
		}
		text = text.mid(d);
		i += d;
	}
	d += checkRange(text, i);		//	x y[a b] の様な場合、有効な範囲を切り出す
	if( !lastTag.isEmpty() && lastTag[0].isUpper() ) {
		lastTag = lastTag.toLower();
		upper = true;
	}
	//ixl += d;		//	ixl は text 先頭文字の行内位置
	int dLine = 0, dOffset = 0;
	if( text.isEmpty() ) {
		if( lastTag == "tr" || lastTag == "/td" ) {
			text = "<td></td>";
			if( upper )
				text = text.toUpper();
			dOffset = 4;
		} else if( lastTag == "/th" ) {
			text = "<th></th>";
			if( upper )
				text = text.toUpper();
			dOffset = 4;
		} else if( lastTag == "table" || lastTag == "/tr" ) {
			text = "<tr></tr>";
			if( upper )
				text = text.toUpper();
			dOffset = 4;
		} else if( lastTag == "ul" || lastTag == "ol" || lastTag == "/li" ) {
			text = "<li></li>";
			if( upper )
				text = text.toUpper();
			dOffset = 4;
		} else {
			return;
		}
	} else
		expandZenCodingHTML(/*indent,*/ text, dLine, dOffset);
	//
	if( !indent.isEmpty() ) {
		dOffset += indent.size();
		text.replace("\n", "\n" + indent);
	}
	pos_t pos0 = pos - (ixl - i);
	m_textCursor->setPosition(pos0, TextCursor::KEEP_ANCHOR);
	m_textCursor->insertText(text);
	m_textCursor->setPosition(pos0);	//	最初の位置
	if( dLine )
		m_textCursor->movePosition(TextCursor::DOWN, TextCursor::MOVE_ANCHOR, dLine);
	else
		dOffset += d;
	m_textCursor->movePosition(TextCursor::BEG_LINE);
	m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::MOVE_ANCHOR, dOffset);
	//onCursorPosChanged();
	clearOpenCloseParenPos();
	//m_openParenPos = m_closeParenPos = -1;
	//update();
	makeCursorInView();
}
bool EditView::zenCodingFor(const QString &indentText, const QString &keyText, int ix, QChar prch)
{
	int dLine, dOffset;
	QString text = keyText;
	if( typeSettings()->name() == "PYTHON" ) {
		if( !expandZenCodingPython(typeSettings(), newLineText(), indentText, text, dLine, dOffset) )
			return false;
	} else if( typeSettings()->name() == "RUBY" ) {
		if( !expandZenCodingRuby(typeSettings(), newLineText(), indentText, text, dLine, dOffset) )
			return false;
	} else if( typeSettings()->name() == "CPP"
		|| typeSettings()->name() == "C#"
		|| typeSettings()->name() == "JAVA"
		|| typeSettings()->name() == "JS"
		|| typeSettings()->name() == "PHP" )		//	HTML中のPHPブロック未対応
	{
		//QChar ch = checkStatement(text, ix);		//	if ( ) r のような場合は r のみに変更
		if( !expandZenCodingCPP(typeSettings(), newLineText(), indentText, text, dLine, dOffset) )
			return false;
		if( prch == '{' ) {			text += " }";
			dOffset += 2;
		}
	} else {
		return false;
	}
	m_textCursor->setPosition(ix, TextCursor::KEEP_ANCHOR);
	m_textCursor->insertText(text);
	m_textCursor->setPosition(ix);
	if( dLine ) {
		m_textCursor->movePosition(TextCursor::DOWN, TextCursor::MOVE_ANCHOR, dLine);
		m_textCursor->movePosition(TextCursor::BEG_LINE);
	}
	m_textCursor->movePosition(TextCursor::RIGHT, TextCursor::MOVE_ANCHOR, dOffset);
	clearOpenCloseParenPos();
	makeCursorInView();
	return true;
}
