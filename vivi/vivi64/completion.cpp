//----------------------------------------------------------------------
//
//			File:			"completion.cpp"
//			Created:		01-10-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include <QtGui>
#include <QMessageBox>
#include <QVBoxLayout>
#include "EditView.h"
#include "Document.h"
#include "mainwindow.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "charEncoding.h"
#include "TextCursor.h"
#include "tokenizer.h"
#include "CompletionWidget.h"
#include "AutoCompletionDlg.h"
#include "ViEngine.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"
#include "../buffer/sssearch.h"

QString getLastToken(TypeSettings *typeSettings, const Buffer &buffer, pos_t pos0, pos_t last, pos_t &);
void setupCandidates(QStringList &lst, const QString &key, const QString &type);

inline bool isSpaceChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t';
}
inline bool isSpaceOrNewLineChar(wchar_t ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}
inline bool isLetterOrNumberOrUnderbar(const QChar &ch)
{
	return ch.unicode() < 0x100 && (ch.isLetterOrNumber() || ch == QChar('_'));
}
inline bool isLetterOrNumberOrUnderbarOrMinus(const QChar &ch)
{
	return ch.unicode() < 0x100 && (ch.isLetterOrNumber() || ch == QChar('_') || ch == QChar('-'));
}
inline bool isLetterOrUnderbar(const QChar &ch)
{
	return ch.unicode() < 0x100 && (ch.isLetter() || ch == QChar('_'));
}
inline bool isDigit(wchar_t ch)
{
	return ch < 0x100 && isdigit(ch);
}
void EditView::setupKeywordsCandidates(QStringList &lst, const QString &text, const QList<QString> &kwList)
{
	for(int i = 0; i < kwList.size(); ++i) {
		if( text.isEmpty() || kwList[i].startsWith(text) ) {
			int ix = kwList[i].indexOf('\t');
			if( ix < 0 )
				lst += kwList[i];
			else
				lst += kwList[i].left(ix);		//	Tab 以降は無視;
		}
	}
}
bool EditView::setupKeywordsCandidates(QStringList &lst, const QString &text)
{
	TypeSettings *ts = typeSettings();
	int ln = positionToLine(m_textCursor->position());
	uint flags = buffer()->lineFlags(ln);
	if( (flags & Buffer::LINEFLAG_IN_SCRIPT) != 0 )
		ts = mainWindow()->typeSettingsForType("JS");
	else if( (flags & Buffer::LINEFLAG_IN_PHP) != 0 )
		ts = mainWindow()->typeSettingsForType("PHP");
	setupKeywordsCandidates(lst, text, ts->keywordList1());
	setupKeywordsCandidates(lst, text, ts->keywordList2());
	if( lst.isEmpty() ) return false;
	lst.sort();
	return true;
}
//	return:	行頭空白類を削除するかどうか
bool EditView::setupCompletionCandidatesFollowingSpaces(QStringList &lst)
{
	if( typeSettings()->name() == "CPP" ) {
		lst << "public:" << "protected:" << "private:"
			<< "public slots:" << "protected slots:" << "private slots:"
			<< "signals:";
		return true;
	} else if( typeSettings()->name() == "HTML" ) {
		//	undone: li 補完すべきかどうかのチェック
		lst << "<li></li>";
	}
	return false;
}
bool loadFile(Buffer &buffer, const QString &fileName)
{
	QString errorString;
	QTextCodec *codec = 0;
	int bomLength;
	byte newLineCode;
	if( !::getTextCodec(fileName, errorString, codec, bomLength, newLineCode) ) {
		QMessageBox::warning(0, "SakuSakuEditor",
							 QString("Cannot read file %1:\n%2.")
							 .arg(fileName)
							 .arg(errorString));
		return false;
	}
	QFile file(fileName);
	if( !file.open(QFile::ReadOnly /*| QFile::Text*/) ) {
		errorString = file.errorString();
		return false;
	}
	if( bomLength != 0 )
		file.seek(bomLength);
	buffer.init();
	QByteArray ba;
	int i = 0;
	while( !file.atEnd() ) {
		ba = file.read(1024*1024);	//	最大IM
		QString buf = codec->toUnicode(ba);
		if( !buffer.basicInsertText(buffer.size(), (wchar_t *)buf.data(), buf.size()) ) {
			buffer.init();
			return false;
		}
		++i;
	}
	return true;
}
void EditView::setupCompletionCandidates(const QString &fileName, SSSearch *sss, QStringList &lst, const QString &text)
{
	Buffer buffer;
	if( !QFileInfo(fileName).exists() ) return;
	if( !loadFile(buffer, fileName) ) return;
	pos_t pos = 0;
	QStringList ;
	while( (pos = sss->strstr(buffer, pos)) >= 0 ) {
		if( !isLetterOrNumberOrUnderbar(buffer[pos - 1]) ) {	//	単語直後の場合
			QString t(text);
			pos += text.length();
			while( pos < buffer.size() && isLetterOrNumberOrUnderbar(QChar(buffer[pos])) )
				t += QChar(buffer[pos++]);
			//	カーソル位置より前の場合
			int k = lst.indexOf(t);
			if( k >= 0 )
				lst.removeAt(k);	//	重複はカーソルに近い方優先
			lst.push_back(t);
		} else
			pos += text.length();
	}
}
bool EditView::checkFileNameCompletion(QStringList &lst,
															QString &text,
															QString &postfix,		//	継続文字列
															pos_t &first)				//	置換開始位置？
{
	//qDebug() << "checkFileNameCompletion()";
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	pos_t ls = lineStartPosition(ln);
	//qDebug() << "ls = " << ls << ", pos = " << pos;
	Tokenizer tkn(*buffer(), ls, pos);
	//qDebug() << tkn.tokenText();
	if( tkn.tokenText() != "#" )
		return false;
	if( tkn.nextTokenText() != "include" ) {
		//qDebug() << tkn.tokenText();
		return false;
	}
	//if( tkn.tokenText() != "#" || tkn.nextTokenText() != "include" )
	//	return false;
	//qDebug() << "#include";
	QChar sep = tkn.skipSpace();
	if( (sep != '\"' && sep != '<') || tkn.nextPosition() >= pos )
		return false;
	QChar sep2 = sep != '<' ? sep : '>';
	//qDebug() << "sep2 = " << sep2;
	postfix = sep2;
	//if( tkn.nextPosition() != pos - text.size() - 1 )
	//	return false;
	int begpos = tkn.nextPosition() + 1;
	QString pat;
	for(int i = tkn.nextPosition() + 1; i < pos; ++i) {
		if( charAt(i) == sep2 ) return false;
		pat += QChar(charAt(i));
	}
	//text = pat;
	int ix = pat.lastIndexOf('/');
	if( ix >= 0 ) {
		first = begpos + ix + 1;		//	置換するのは、最後の / の次から
		text = pat.mid(ix+1);
	} else {
		first = begpos;
		text = pat;
	}
	bool rc;
	//qDebug() << "sep = " << sep;
	if( sep == '<' ) {		//	ライブラリ補完
		rc = clibralyCompletion(lst, pat, first);
	} else
		rc = fileNameCompletion(lst, pat, first);
	//if( rc )
	//	text = pat;
	return rc;
}
bool EditView::fileNameCompletion(QStringList &lst, QString &pat, pos_t &first)
{
	//QString dirPath;
	QDir dir;
	if( fullPathName().isEmpty() ) {
		dir = QDir::current();
	} else {
		QDir dir(fullPathName());
		dir.cdUp();
	}
	qDebug() << dir.absolutePath();
	while( pat.startsWith("../") || pat.startsWith("..\\") ) {
		pat = pat.mid(3);
		dir.cdUp();
	}
	int ix = pat.lastIndexOf('/');
	if( ix >= 0 ) {
		dir = QDir(dir.absolutePath() + "/" + pat.left(ix));
		pat = pat.mid(ix+1);
		//first += ix + 1;
	}
	QStringList elst = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
	foreach(const QString fileName, elst) {
		if( pat.isEmpty() || fileName.startsWith(pat, Qt::CaseInsensitive) )
			lst += fileName;
	}
	elst = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	foreach(const QString fileName, elst) {
		if( pat.isEmpty() || fileName.startsWith(pat, Qt::CaseInsensitive) )
			lst += fileName + "/";		//	ディレクトリは / を付加
	}
	return !lst.isEmpty();
	//if( !lst.isEmpty() ) {
	//	return true;
	//} else
	//	return false;
}
//	src=" 補完
bool EditView::checkSrcFileNameCompletion(QStringList &lst,
															QString &text,
															QString &postfix,		//	継続文字列
															pos_t &first)				//	置換開始位置？
{
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	pos_t ls = lineStartPosition(ln);
	Tokenizer tkn(*buffer(), ls, pos);
	QString token1, token2, token3;
	pos_t lastPos;
	while( tkn.tokenType() != Tokenizer::END_OF_FILE
			&& tkn.tokenPosition() < pos )
	{
		lastPos = tkn.tokenPosition();
		token1 = token2;
		token2 = token3;
		token3 = tkn.tokenText();
		//qDebug() << tkn.tokenText();
		tkn.nextToken();
	}
	if( token1.compare("src", Qt::CaseInsensitive) != 0
		|| token2 != "="
		|| token3.isEmpty()
		|| token3[0] != '\"' )
	{
		return false;
	}
	first = lastPos + 1;
	postfix = "\"";
	QString pat= token3.mid(1, pos - first);
	//qDebug() << pat;
	return fileNameCompletion(lst, pat, first);
}
void EditView::setupCloseTags(QStringList &lst)
{
	//lst << "</body>" << "</html>";
	pos_t pos = m_textCursor->position();
	checkEndTagCompletion(lst, pos);
}
//	</endtag 補完
bool EditView::checkEndTagCompletion(QStringList &lst,
														pos_t &first)		//	in:補完開始位置、out:置換開始位置
{
	if( isLetterOrUnderbar(charAt(first)) || charAt(first-1) != '<' )
		return false;
	Tokenizer tkn(*buffer(), 0, first);
	while( tkn.tokenType() != Tokenizer::END_OF_FILE ) {
		const QString text = tkn.tokenText();
		if( text == "<" && isLetterOrUnderbar(charAt(tkn.tokenPosition() + 1)) ) {
			tkn.nextToken();
			QString tagText = "</" + tkn.tokenText() + ">";
			int ix = lst.lastIndexOf(tagText);
			if( ix >= 0 ) {
				while( lst.size() > ix ) lst.pop_back();
			}
			lst.push_back(tagText);
		} else if( text == "<" && charAt(tkn.tokenPosition() + 1) == '/'
					&& isLetterOrUnderbar(charAt(tkn.tokenPosition() + 2))
					&& !lst.isEmpty() )
		{
			//qDebug() << "end tag";
			tkn.nextToken();
			tkn.nextToken();
			QString tagText = "</" + tkn.tokenText() + ">";
			if( lst.last() == tagText )
				lst.pop_back();
		} else if( tkn.tokenType() == Tokenizer::STRING && text[0] != text[text.size()-1] ) {
			qDebug() << "not closed string.";
			//	文字列が閉じていない場合
			Tokenizer tkn2(*buffer(), tkn.tokenPosition()+1, tkn.tokenPosition() + text.size());
			while( tkn2.tokenType() != Tokenizer::END_OF_FILE ) {
				if( tkn2.tokenText() == "<" && isLetterOrUnderbar(charAt(tkn2.tokenPosition() + 1)) ) {
					tkn2.nextToken();
					QString tagText = "</" + tkn2.tokenText() + ">";
					int ix = lst.lastIndexOf(tagText);
					if( ix >= 0 ) {
						while( lst.size() > ix ) lst.pop_back();
					}
					lst.push_back(tagText);
				}
				tkn2.nextToken();
			}
		}
		tkn.nextToken();
	}
	return !lst.isEmpty();
}
//	} 補完
bool EditView::checkEndBraceCompletion(int ln)
{
	if( m_textCursor->hasSelection() )
		return false;
	pos_t pos = m_textCursor->position();
	if( buffer()->charAt(pos - 1) != '{' )
		return false;
	bool im = isModified();
	QString it = indentText(ln);
	QString text = newLineText() + it + "}";
	m_textCursor->insertText(text);
	m_textCursor->setPosition(pos);
	m_textCursor->insertText(newLineText() + it + "\t");
	if( !im )
		emit modifiedChanged();
	update();
	return false;
}
const char *libnames[] = {
	"algorithm", "array", "assert.h", "atomic", "bitset",
	"cassert", "cctype", "cerrno", "cfenv", "cfloat",
	"chrono", "cinttypes", "ciso646", "climits", "clocale",
	"cmath", "codecvt", "complex", "condition_variable", "csetjmp",
	"csignal", "cstdarg", "cstdbool", "cstddef", "cstdint",
	"cstdio", "cstdlib", "cstring", "ctgmath", "ctime",
	"ctype.h", "cuchar", "cwchar", "cwctype", "deque",
	"errno.h", "exception", "fenv.h", "float.h", "forward_list",
	"fstream", "functional", "future", "initializer_list", "inttypes.h",
	"iomanip", "ios", "iosfwd", "iostream", "iso646.h",
	"istream", "iterator", "limits", "limits.h", "list",
	"locale", "locale.h", "map", "math.h", "memory",
	"mutex", "new", "numeric", "ostream", "queue",
	"random", "ratio", "regex", "set", "setjmp.h",
	"signal.h", "sstream", "stack", "stdarg.h", "stdbool.h",
	"stddef.h", "stdexcept", "stdint.h", "stdio.h", "stdlib.h",
	"streambuf", "string", "string.h", "system_error", "tgmath.h",
	"thread", "time.h", "tuple", "type_traits", "typeindex",
	"typeinfo", "uchar.h", "unordered_map", "unordered_set", "utility",
	"valarray", "vector", "wchar.h", "wctype.h", 
	0
};
void setupLibNames(QStringList &lst /*, QString pat*/)
{
	lst.clear();
	for(cchar **pp = &libnames[0]; *pp != 0; ++pp) {
		QString s(*pp);
		//if( pat.isEmpty() || s.startsWith(pat) )
			lst += "<" + s + ">";
	}
}
bool EditView::clibralyCompletion(QStringList &lst, const QString &pat, pos_t &first)
{
	//qDebug() << "clibralyCompletion()"; 
	//first = 0;
	for(cchar **pp = &libnames[0]; *pp != 0; ++pp) {
		QString s(*pp);
		if( pat.isEmpty() || s.startsWith(pat) )
			lst += s;
	}
	return !lst.isEmpty();
}
int EditView::setupCompletionCandidates(QStringList &lst, const QString &text,
															pos_t first)		//	参照単語先頭位置
{
	//lst.clear();
	if( text.isEmpty() ) return 0;
	//if( checkFileNameCompletion(lst, text) )
	//	return;
	std::unique_ptr<SSSearch> sss(new SSSearch());
	//SSSearch srch;
	if( !sss->setup((const wchar_t *)text.data(), text.size()) ) return 0;
	int maxLn = qMin(100, document()->lineCount());
	for(int ln = 0; ln < maxLn; ++ln) {
		Tokenizer tkn(*buffer(), lineStartPosition(ln), lineStartPosition(ln+1));
		if( tkn.tokenText() == "#"
			&& tkn.nextToken() == Tokenizer::IDENT && tkn.tokenText() == "include"
			&& tkn.nextToken() == Tokenizer::STRING )
		{
			QString fileName = tkn.tokenText().mid(1, tkn.tokenText().size() - 2);
			//qDebug() << fileName;
			QDir path(fileName);
			//if( !path.isAbsolute() ) {
				//path.makeAbsolute();
				fileName = path.absolutePath();
			//}
			qDebug() << fileName;
			setupCompletionCandidates(fileName, sss.get(), lst, text);
		}
	}
	int n = lst.size();		//	カーソル以前の候補数
	//const int lastPosition = first + text.length();
	pos_t pos = 0;
	QStringList lstAfter;
	while( (pos = sss->strstr(*buffer(), pos)) >= 0 ) {
		if( pos != 0 && !isLetterOrNumberOrUnderbar(charAt(pos - 1)) ) {	//	単語直後の場合
			pos_t pos0 = pos;		//	マッチ位置
			QString t(text);
			pos += text.length();
			while( pos < bufferSize() && isLetterOrNumberOrUnderbar(QChar(charAt(pos))) )
				t += QChar(charAt(pos++));
			if( pos0 < first ) {
				//	カーソル位置より前の場合
				int k = lst.indexOf(t);
				if( k >= 0 )
					lst.removeAt(k);	//	重複はカーソルに近い方を優先
				else
					++n;
				lst.push_back(t);
			} else if( pos0 > first) {
				//	カーソルより後ろの場合
				int k = lst.indexOf(t);
				if( k < 0 ) {
					k = lstAfter.indexOf(t);
					if( k < 0 )		//	
						lstAfter.push_back(t);
				} else if( lst.size() - k > lstAfter.size() && lstAfter.indexOf(t) < 0 ) {
					lst.removeAt(k);
					lstAfter.push_back(t);
				}
			}
		} else
			pos += text.length();
	}
	lst += lstAfter;
	//if( n < 0 ) n = lst.size() - 1;
	if( n > 0 ) --n;		//	カーソル以前に候補がある場合は、最後の候補
	return n;
}
void EditView::completion()
{
	completion(false);
}
void EditView::kwCompletion()
{
	completion(true);
}
bool EditView::isSpaces(pos_t first, pos_t last) const
{
	while( first < last ) {
		if( !isSpaceChar(charAt(first++)) )
			return false;
	}
	return true;
}
//	動的補?E
void EditView::completion(bool keyword)
{
	bool im = isModified();
	m_textCursor->clearSelection();
	pos_t first = m_textCursor->position();
#if 0
	QRect rect = cursorRect(cur);
	QPoint p(rect.x() + leftMarginWidth(), rect.bottom());		//	EditView 座標系
#endif
	QString text;		//	カーソル直前の参照されるテキスト
	pos_t pos = m_textCursor->position();
	int ln = document()->positionToLine(pos);
	pos_t ls = lineStartPosition(ln);
	int offset = pos - ls;
	int row = 0;
	QStringList lst;	//	補完候補入れ物
	QString postfix;	//	継続文字列
	bool blankText = false;		//	補完元テキストが空
	bool toRemoveSpaces = false;		//	行頭空白削除
	if( !keyword && (pos == ls || isSpaces(ls, pos)) ) {
		// 行頭 or カーソル直前が空白類だけの場合、それ専用の処理
		blankText = true;
		toRemoveSpaces = setupCompletionCandidatesFollowingSpaces(lst);
	} else if( !keyword && checkFileNameCompletion(lst, text, postfix, first) ) {		//	ファイル名補完
		qDebug() << "checkFileNameCompletion() returned True";
	} else if( !keyword && checkSrcFileNameCompletion(lst, text, postfix, first) ) {		//	src=" ファイル名補完
	} else if( !keyword && checkEndBraceCompletion(ln) ) {
		return;
	} else if( !keyword && checkEndTagCompletion(lst, first) ) {		//	</endtag> 補完
		--first;
		row = lst.size() - 1;
	///} else if( !keyword && checkCloseBraceCompletion(lst, first) ) {		//	} 補完
	} else {
		if( offset > 0 && isLetterOrNumberOrUnderbar(QChar(document()->charAt(pos - 1))) ) {
			TextCursor cur(*m_textCursor);
			cur.movePosition(TextCursor::BEG_WORD, TextCursor::KEEP_ANCHOR);
			if( !cur.hasSelection() )
				cur.movePosition(TextCursor::PREV_WORD, TextCursor::KEEP_ANCHOR);
			first = cur.position();
			text = cur.selectedText();
		}
		if( !keyword ) {
			row = setupCompletionCandidates(lst, text, first);		//	補完候補をセットアップ
		} else {
			setupKeywordsCandidates(lst, text);
		}
		if( lst.isEmpty() ) return;		//	補完候補無し
	}
	CompletionWidget cmpl(this, lst, text);
	cmpl.setCurrentRow(row);
	if( !cmpl.count() ) {
		///showMessage(tr("No matched keyword."));
		return;
	}
	QRect rct = rect();
	int vln = m_textCursor->viewLine();
	int x = offsetToPx(vln, pos - viewLineStartPosition(vln));
	int y = (vln - m_scrollY0 + 1) * lineHeight() + 2;
	int ht = cmpl.height();
	if( y + ht > rct.height() ) {
		y -= ht + lineHeight() + 2;
	}
	QPoint p(x + m_lineNumAreaWidth, y);
	cmpl.move(mapToGlobal(p));		//	EditView 座標系 → グローバル座標系変換
	if( cmpl.count() == 1 ) {			//	
		if( blankText ) {
			//	undone: オートインデント対応？
			m_textCursor->insertText(cmpl.text());
			emit textInserted(cmpl.text());
			QRegExp re("<.+></.+>");
			if( re.exactMatch(cmpl.text()) ) {
				//	<li></li> 等の場合は、その間にカーソル移勁
				int ix = cmpl.text().lastIndexOf("<");
				m_textCursor->movePosition(TextCursor::LEFT,
															TextCursor::MOVE_ANCHOR,
															cmpl.text().size() - ix);
			}
			//##updateScrollBarInfo();
			makeCursorInView();
			update();
			if( !im )
				emit modifiedChanged();
			return;
		}
		//return;
	}
	connect(&cmpl, SIGNAL(textChanged(const QString &)),
					this, SLOT(cmplTextChanged(const QString &)));
	const int rc = cmpl.exec();
	switch( rc ) {
		case QDialog::Accepted: {
			wchar_t ch = charAt(m_textCursor->position());
			if( toRemoveSpaces )
				m_textCursor->movePosition(TextCursor::BEG_LINE, TextCursor::KEEP_ANCHOR);
			else
				m_textCursor->setPosition(first, TextCursor::KEEP_ANCHOR);
			QString txt = cmpl.text();		//	差分ではなく、フルテキストが返ってくる
			if( txt[txt.size()-1] != '/' && (!ch || isSpaceOrNewLineChar(ch)) && !postfix.isEmpty() )
				txt += postfix;
			m_textCursor->insertText(txt);
			emit textInserted(cmpl.text());
			if( !im )
				emit modifiedChanged();
			break;
		}
		case '\b':
			m_textCursor->deletePrevChar();
			if( !im )
				emit modifiedChanged();
			break;
	}
}
bool EditView::setupWord(QStringList &lst, QString &key, pos_t &first)
{
	//lst.clear();
	TextCursor cur(*m_textCursor);
	if( typeSettings()->name() == "CSS" ) {
		pos_t pos = cur.position();
		pos_t ls = viewLineStartPosition(cur.viewLine());
		while( pos != ls && isLetterOrNumberOrUnderbarOrMinus(charAt(pos-1)) )
			--pos;
		cur.setPosition(pos, TextCursor::KEEP_ANCHOR);
	} else
		cur.movePosition(TextCursor::BEG_WORD, TextCursor::KEEP_ANCHOR);
	if( !cur.hasSelection() )
		cur.movePosition(TextCursor::PREV_WORD, TextCursor::KEEP_ANCHOR);
	first = cur.position();
	//if( charAt(first - 1) == '\\' )	//	\n のような場合は自動補完しない方がよい？
	//	return false;
	key = cur.selectedText();
	if( key.isEmpty() || !isLetterOrUnderbar(key[0]) )
		return false;
	setupCompletionCandidates(lst, key, first);
	return !lst.isEmpty();
}
void EditView::cmplTextChanged(const QString &text)
{
	//showMessage(text);
}
void EditView::showAutoCompletionDlg(const QStringList &lst, QString ft, bool selTail, bool filterCaseSensitive)
{
	if( lst.isEmpty()
		|| lst.size() == 1 && lst[0] == ft )	//	候補がひとつで、キー文字列と等しい場合
	{
		return;
	}
	m_autoCompletionDlg = new AutoCompletionDlg(lst, ft, selTail, this);
	m_autoCompletionDlg->setFilterCaseSensitive(filterCaseSensitive);
	connect(m_autoCompletionDlg, SIGNAL(keyPressed(QString)),
					this, SLOT(autoCmplKeyPressed(QString)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(backSpace()),
					this, SLOT(autoCmplBackSpace()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(delPressed(bool, bool)),
					this, SLOT(autoCmplDelete(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(leftPressed(bool, bool)),
					this, SLOT(autoCmplLeft(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(rightPressed(bool, bool)),
					this, SLOT(autoCmplRight(bool, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(zenCoding()),
					this, SLOT(autoCmplZenCoding()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(pasted()),
					this, SLOT(autoCmplPasted()),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(decided(QString, bool)),
					this, SLOT(autoCmplDecided(QString, bool)),
					Qt::QueuedConnection);
	connect(m_autoCompletionDlg, SIGNAL(rejected()),
					this, SLOT(autoCmplRejected()),
					Qt::QueuedConnection);
	//this->setFocus();
	m_autoCompletionDlg->show();
	pos_t pos = m_textCursor->position();
	QRect rct = rect();
	int vln = m_textCursor->viewLine();
	int x = offsetToPx(vln, pos - viewLineStartPosition(vln));
	int y = (vln - m_scrollY0 + 1) * lineHeight() + 4;
	int ht = m_autoCompletionDlg->height();
	if( y + ht > rct.height() ) {
		y -= ht + lineHeight() + 2;
	}
	QPoint p(x + m_lineNumAreaWidth, y);
#if	BASE_QWIDGET
	m_autoCompletionDlg->move(p);
#else
	//	EditView 座標系 → グローバル座標系変換
	m_autoCompletionDlg->move(mapToGlobal(p));
#endif
	//m_autoCompletionDlg->setFocus();
	m_autoCmplFilter = ft;
	m_autoCmplTyped.clear();
}
void EditView::closeAutoCompletionDlg()
{
	if( m_autoCompletionDlg == 0 )
		return;
	delete m_autoCompletionDlg;
	m_autoCompletionDlg = 0;
}
//	単語 \s+ 直後か？
bool EditView::isAfter(pos_t &lastTokenPos, const QString &key, pos_t pos)
{
	if( pos < 0 )
		pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	pos_t ls = viewLineStartPosition(vln);
	QString token = getLastToken(typeSettings(), *buffer(), ls, pos, lastTokenPos);
	return token == key;
}
#if	0
//	for \s+ 直後か？
bool EditView::isAfterFor(int &lastTokenPos)
{
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	pos_t ls = viewLineStartPosition(vln);
	QString token = getLastToken(typeSettings(), *buffer(), ls, pos, lastTokenPos);
	return token == "for";
}
#endif
//	#include \s* 直後か？
bool EditView::isAfterInclude()
{
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	pos_t p = viewLineStartPosition(vln);
	while( p <= pos && isSpaceChar(charAt(p)) ) ++p;
	if( !buffer()->isMatched(L"#include", p) ) return false;
	p += strlen("#include");
	while( p <= pos && isSpaceChar(charAt(p)) ) ++p;
	return p == pos;
}
//	#include \s* ".*直後か？
//	p は " 直後位置を返す
bool EditView::isAfterIncludeDQ(pos_t &p)
{
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	p = viewLineStartPosition(vln);
	while( p <= pos && isSpaceChar(charAt(p)) ) ++p;
	if( !buffer()->isMatched(L"#include", p) ) return false;
	p += strlen("#include");
	while( p <= pos && isSpaceChar(charAt(p)) ) ++p;
	if( charAt(p++) != '\"' ) return false;
	return p <= pos;
}
void EditView::autoCmplKeyPressed(QString text)
{
	if( text[0].unicode() >= 0x20 && text[0].unicode() < 0x7f || text == "\t" ) {
		//if( mainWindow()->viEngine()->isRedoRecording() )
		//	mainWindow()->viEngine()->appendInsertedText(text);
		m_autoCmplTyped += text;
		if( !editForVar(text) )
			insertText(text);
		if( m_autoCompletionDlg != 0 ) {
			if( !m_autoCompletionDlg->count() )
				closeAutoCompletionDlg();
			else if( m_autoCompletionDlg->count() == 1 ) {
				QString t = getText(*buffer(), m_autoCmplPos, m_textCursor->position());
				QString t2 = m_autoCompletionDlg->text();
				if( t == t2 )
					closeAutoCompletionDlg();
			}
		}
		if( m_autoCompletionDlg == 0 ) {
			const bool stmntCmpl = typeSettings()->boolValue(TypeSettings::STATEMENT_COMPLETION);
			//const bool stmntCmpl = globSettings()->boolValue(GlobalSettings::STATEMENT_COMPLETION);
			TypeSettings *ts = typeSettings();
			const QString typeName = ts->name();
			const bool isCPPType = typeName == "CPP"
												|| typeName == "C#"
												|| typeName == "HLSL"
												|| typeName == "JAVA"
												|| typeName == "PHP";
			pos_t lastTokenPos;
			pos_t pos = m_textCursor->position() - text.size();
			if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "for", pos)
						&& isCPPType )
			{
				m_autoCmplPos = lastTokenPos;
				QStringList lst;
				setupCandidates(lst, "for", typeName);
				showAutoCompletionDlg(lst, "for (");
			} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "foreach", pos)
						&& typeName ==  "C#" )
			{
				m_autoCmplPos = lastTokenPos;
				QStringList lst;
				setupCandidates(lst, "foreach", typeName);
				showAutoCompletionDlg(lst, "foreach (");
			} else if( stmntCmpl && text == "(" && isAfter(lastTokenPos, "if", pos)
							&& isCPPType )
			{
				m_autoCmplPos = lastTokenPos;
				QStringList lst;
				setupCandidates(lst, "if", "CPP");
				showAutoCompletionDlg(lst, "if (");
			} else if( stmntCmpl && text == "("
							&& isCPPType && isAfter(lastTokenPos, "while", pos) )
			{
				m_autoCmplPos = lastTokenPos;
				QStringList lst;
				setupCandidates(lst, "while", "CPP");
				showAutoCompletionDlg(lst, "while (");
			}
		}
	}
}
void EditView::autoCmplLeft(bool ctrl, bool shift)
{
	closeAutoCompletionDlg();
	curMove(TextCursor::LEFT);
}
void EditView::autoCmplRight(bool ctrl, bool shift)
{
	closeAutoCompletionDlg();
	curMove(TextCursor::RIGHT);
}
void EditView::autoCmplBackSpace()
{
	onBackSpace(false, false, false);
}
void EditView::autoCmplDelete(bool ctrl, bool shift)
{
	onDelete(ctrl, shift, false);
}
void EditView::autoCmplZenCoding()
{
	closeAutoCompletionDlg();
	zenCoding();
}
void EditView::autoCmplPasted()
{
	closeAutoCompletionDlg();
	paste();
}
//	自動補完文字列が確定した場合の処理
//		abc があった場合に a とタイプし、確定した場合：
//			m_autoCmplFilter = "a", text = "abc"
//			ViEngine::insertedText = "a"
//		abc があった場合に abc までタイプした場合：autoClosed = true
//			m_autoCmplFilter = "a", text = "abc"
//			ViEngine::insertedText = "ab"
void EditView::autoCmplDecided(QString text, bool autoClosed)
{
	qDebug() << "m_autoCmplFilter = " << m_autoCmplFilter << ", m_autoCmplTyped = " << m_autoCmplTyped;
	qDebug() << "text = " << text << "ViEngine::insertedText = " << mainWindow()->viEngine()->insertedText();
	//if( autoClosed ) {
	//	const int sz = mainWindow()->viEngine()->insertedText().size();
	//	mainWindow()->viEngine()->appendInsertedText(text.mid(sz));
	//}
	m_textCursor->setPosition(m_autoCmplPos, TextCursor::KEEP_ANCHOR);		//	既に入力済みの部分を選択
	//QString t0 = m_textCursor->selectedText();
	///mainWindow()->viEngine()->removeFromInsertedText(m_autoCmplFilter);
	//if( text.startsWith(t0) )
	//	mainWindow()->viEngine()->appendInsertedText(text.mid(t0.size()));
	QString t = autoIndentText();
	QString dst;
	pos_t pos = -1;
	openUndoBlock();
	for (int i = 0; i < text.size();) {
		QChar ch = text[i++];
		if( ch == '\\' && i < text.size() ) {
			ch = text[i++];
			if( ch == 't' )
				ch = '\t';
			else if( ch == 'n' ) {
				insertText(dst);
				insertText(t);
				dst.clear();
				continue;
			} else if( ch == 'c' ) {
				insertText(dst);
				pos = m_textCursor->position();
				dst.clear();
				continue;
			}
		}
		dst += ch;
	}
	if( !editForVar(dst) ) {
		insertText(dst);
	} if( !m_autoCmplAtBegWord
		&& !autoClosed )		//	自動確定の場合は、削除を行わない
	{
		//	確定後以降の英数字文字を削除
		int p = m_textCursor->position();
		while( isLetterOrNumberOrUnderbar(charAt(p)) ) ++p;
		if( p != m_textCursor->position() ) {
			m_textCursor->setPosition(p, TextCursor::KEEP_ANCHOR);
			m_textCursor->deleteChar();
		}
	}
	m_autoCmplAtBegWord = false;
	closeUndoBlock();
	if( pos >= 0 ) {
		m_textCursor->setPosition(pos);
		if( isLetterOrNumberOrUnderbar(QChar(charAt(pos))) )
			m_textCursor->movePosition(TextCursor::END_WORD, TextCursor::KEEP_ANCHOR);
	}
	closeAutoCompletionDlg();
	setFocus();		//	ダイアログクリックで取られたフォーカスを戻す
	update();
}
void EditView::autoCmplRejected()
{
	closeAutoCompletionDlg();
	setFocus();		//	ダイアログクリックで取られたフォーカスを戻す
}
