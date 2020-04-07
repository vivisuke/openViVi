//----------------------------------------------------------------------
//
//			File:			"Buffer.cpp"
//			Created:		09-Jun-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include "Buffer.h"
#include "gap_buffer.h"
#include "LineMgr.h"
#include "undoMgr.h"
#include "MarkMgr.h"
#include "sssearch.h"
#include "UTF16.h"
//#include "viewLineMgr.h"
#include <iostream>
#include <string>

inline bool isDigit(wchar_t ch)
{
	return ch < 0x100 && isdigit(ch);
}

//--------------------------------------------------------------------
Buffer::Buffer()
	: m_modified(false)
	, m_lastModifiedPos(0)
	, m_seqNumber(0)
{
#if		USE_GAP_DEQUE
	m_buffer = new gap_deque<wchar_t>();
#else
	m_buffer = new gap_buffer<wchar_t>();
#endif
	m_lineMgr = new LineMgr(this);
	m_undoMgr = new UndoMgr(this);
	m_markMgr = new MarkMgr();
}
Buffer::Buffer(const Buffer &x)
	: m_modified(false)
	, m_lastModifiedPos(0)
	, m_seqNumber(x.m_seqNumber)
{
#if		USE_GAP_DEQUE
	m_buffer = new gap_deque<wchar_t>(*x.m_buffer);
#else
	m_buffer = new gap_buffer<wchar_t>(*x.m_buffer);
#endif
	m_lineMgr = new LineMgr(*x.m_lineMgr);
	m_undoMgr = new UndoMgr(this);		//	Undo情報はコピーされないものとする
	m_markMgr = new MarkMgr();			//	マークもコピーされないよん
}
Buffer::~Buffer()
{
	delete m_buffer;
	delete m_lineMgr;
	delete m_undoMgr;
	delete m_markMgr;
}
void Buffer::init()
{
	m_buffer->clear();
	m_lineMgr->init();
	m_undoMgr->init();
	m_markMgr->clear();
	setModified(false);
}
void Buffer::setModified(bool b)
{
	//if( b != m_modified )
		m_modified = b;
}
//	ファイル保存した時にコールされる
void Buffer::onSaved()
{
	m_modified = false;
	m_undoMgr->onSaved();
	m_lineMgr->setSavedFlag();
}
bool Buffer::isEmpty() const
{
	return m_buffer->empty();
}
bool Buffer::isBlankEOFLine() const
{
	if( isEmpty() ) return true;
	//pos_t last = m_buffer->size() - 1;
	//wchar_t ch = charAt(last);
	wchar_t ch = m_buffer->back();
	return ch == '\r' || ch == '\n';
}
bool Buffer::isSpaces(pos_t first, pos_t last) const
{
	while( first != last ) {
		wchar_t ch = charAt(first++);
		if( ch != ' ' && ch != '\t' ) return false;
	}
	return true;
}
bool Buffer::isEqual(pos_t pos, const wchar_t *ptr) const
{
	while( *ptr != '\0' ) {
		if( operator[](pos++) != *ptr++ )
			return false;
	}
	return true;
}
ssize_t Buffer::size() const
{
	return m_buffer->size();
}
wchar_t Buffer::charAt(pos_t ix) const
{
	return m_buffer->at(ix);
}
uint Buffer::lineFlags(line_t ln) const
{
	return m_lineMgr->lineFlags(ln);
}
void Buffer::setLineFlag(line_t ln, uint flag)
{
	m_lineMgr->setLineFlag(ln, flag);
}
void Buffer::resetLineFlag(line_t ln, uint flag)
{
	m_lineMgr->resetLineFlag(ln, flag);
}
void Buffer::clearLineFlags()
{
	for (line_t ln = 0; ln < lineCount(); ++ln) {
		m_lineMgr->clearLineFlag(ln);
	}
}
//	行の LINEFLAG_GLOBAL をセット/アンセット
bool Buffer::setGlobalFlag(const wchar_t *pat, ssize_t sz)
{
	m_lineMgr->clearGlobalFlag();
	SSSearch sss;
	if( !sss.setup(pat, sz, 0, SSSearch::STD_REGEX) ) return false;
	for (line_t ln = 0; ln < m_lineMgr->lineCount(); ++ln) {
		pos_t ls = m_lineMgr->lineStartPosition(ln);
		pos_t last = m_lineMgr->lineStartPosition(ln+1);
		if( sss.strstr(*this, ls, last) >= 0 )
			m_lineMgr->setLineFlag(ln, LINEFLAG_GLOBAL);
	}
	return true;
}
void Buffer::setCanCollapse(line_t ln, bool b)
{
	if( b )
		m_lineMgr->setLineFlag(ln, LINEFLAG_CAN_COLLAPSE);
	else
		m_lineMgr->resetLineFlag(ln, LINEFLAG_CAN_COLLAPSE);
}
bool Buffer::canCollapse(line_t ln) const
{
	return (m_lineMgr->lineFlags(ln) & LINEFLAG_CAN_COLLAPSE) != 0;
}
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}
//	行管理対応、undo/redo 非対象
bool Buffer::basicInsertText(pos_t pos, cwchar_t ch)
{
	return basicInsertText(pos, &ch, &ch + 1);
}
bool Buffer::basicInsertText(pos_t pos, cwchar_t *first, cwchar_t *last, line_t ln)
{
	//m_buffer->insert(pos, first, last);
	return basicInsertText(pos, first, last - first, ln);
}
bool Buffer::basicInsertText(pos_t pos, cwchar_t *first, ssize_t sz, line_t ln)
{
	if( !sz ) return true;
	if( pos < 0 ) pos = 0;
	else if( pos > size() ) pos = size();
	bool wasBlankEOFLine = isBlankEOFLine();
	const bool atEOF = pos == size();
	cwchar_t *last = first + sz;
	if( ln < 0 ) ln = positionToLine(pos);		//	Buffer::positionToLine() は最後の改行有無を考慮
	const line_t ln0 = ln;		//	挿入位置
	const line_t lineCount0 = m_lineMgr->lineCount();		//	挿入前行数
	int prevChar = !pos ? (-1) : m_buffer->operator[](pos-1);
	int nextChar = atEOF ? (-1) : m_buffer->operator[](pos);
	if( !m_buffer->insert(pos, first, sz) )
		return false;
	//	挿入文字列に改行（CR/LF/CRLF）があった場合は、行を作成
	pos_t p = pos;
	if( prevChar == '\r' && nextChar == '\n' )		//	CRLF の間に挿入
		m_lineMgr->addLine(ln++, p);
	while( first != last ) {
		wchar_t ch = *first++;
		if( ch == '\r' ) {
			if( atEOF && prevChar >= 0 && !isNewLine(prevChar) )
				m_lineMgr->addAt(++ln, 1);
			else
				m_lineMgr->addLine(ln++, p+1);
		} else if( ch == '\n' ) {
			if( prevChar == '\r' /*|| atEOF && prevChar != '\n'*/ ) {
				m_lineMgr->addAt(ln, 1);
			} else if( atEOF && prevChar >= 0 && prevChar != '\n' ) {
				m_lineMgr->addAt(++ln, 1);
			} else
				m_lineMgr->addLine(ln++, p+1);
		} else {
			if( atEOF ) {
				if( prevChar < 0 || isNewLine(prevChar) )
					m_lineMgr->addLine(ln, p+1);
				else
					m_lineMgr->addAt(ln+1, 1);
			}
		}
		++p;
		prevChar = (uint)ch;
	}
	if( prevChar == '\r' && !atEOF && m_buffer->operator[](p) == '\n' )
		m_lineMgr->eraseLine(ln--);
	if( !atEOF )
		m_lineMgr->textInserted(ln, sz);
	m_markMgr->inserted(pos, sz);
	emit onInserted(ln0, m_lineMgr->lineCount() - lineCount0);
	//if( m_lineMgr->lineCount() != lineCount0 ) {
		//for(auto itr = m_viewLineMgrs.begin(); itr != m_viewLineMgrs.end(); ++itr)
		//	(*itr)->inserted(ln0, m_lineMgr->lineCount() - lineCount0);
	//}
	m_lastModifiedPos = pos + sz;
	if( !m_emphasizedRanges.empty() )
		inserted(pos, sz);
	return true;
}
void Buffer::basicDeleteText(pos_t pos)
{
	if( pos < 0 || pos >= size() ) return;
	basicDeleteText(pos, 1);
}
void Buffer::basicDeleteText(pos_t first, ssize_t sz, line_t line)
{
	pos_t last = first + sz;
	if( first < 0 ) first = 0;
	if( last > size() ) last = size();
	if( first >= last ) return;
	if( first == 0 && last == size() ) {	//	全削除の場合
		line_t lineCount0 = m_lineMgr->lineCount();
		m_lineMgr->init();
		m_buffer->clear();
		m_lastModifiedPos = 0;
		emit  onDeleted(first, lineCount0);
		return;
	}
	wchar_t prevChar = m_buffer->operator[](first-1);	//	範囲外は '\0' を返す仕様なので範囲チェック不要
	if( line < 0 ) line = m_lineMgr->positionToLine(first);
	const line_t line0 = line;
	const line_t lineCount0 = m_lineMgr->lineCount();
	bool bBegLine = m_lineMgr->lineStartPosition(line) == first;	//	行頭から削除
	//	lineStart が削除範囲の場合は、行削除
	line_t ln0 = line + 1;
	if( prevChar == '\r' && charAt(first) == '\n' ) {		//	\r\n の間から別の行の\r\n の間まで削除される場合
		m_lineMgr->setLineStartPosition(++line, first);
		++ln0;
	}
	line_t ln = ln0;
	while( ln < m_lineMgr->lineCount() && m_lineMgr->lineStartPosition(ln) <= last )
		++ln;
	m_lineMgr->eraseLine(ln0, ln - ln0);
	if( prevChar == '\r' ) {
		if( last != size() && m_buffer->operator[](last) == '\n') {
			m_lineMgr->eraseLine(line--);
		}
	} else if( !bBegLine && last == size() /*&& prevChar != '\r'*/ ) {
		//	行の途中からバッファ末尾まで削除した場合は、ダミー行が必要
		m_lineMgr->addLine(line, last);
	}
	m_lineMgr->textInserted(line, -sz);
	m_buffer->eraseFL(first, last);
	m_markMgr->deleted(first, sz);
	emit onDeleted(line0, lineCount0 - m_lineMgr->lineCount());
	m_lastModifiedPos = first;
	if( !m_emphasizedRanges.empty() )
		deleted(first, sz);
}
void Buffer::basicReplaceText(pos_t pos, ssize_t dsz, cwchar_t *first, ssize_t isz, line_t ln)
{
	basicDeleteText(pos, dsz, ln);
	basicInsertText(pos, first, isz, ln);
}
void Buffer::inserted(pos_t pos, ssize_t sz)
{
	int i = 0;
	while( i < m_emphasizedRanges.size() && m_emphasizedRanges[i] <= pos )
		++i;
	while( i < m_emphasizedRanges.size() )
		m_emphasizedRanges[i++] += sz;
}
void Buffer::deleted(pos_t pos, ssize_t sz)
{
	int i = 0;
	while( i < m_emphasizedRanges.size() && m_emphasizedRanges[i] <= pos )
		++i;
	while( i < m_emphasizedRanges.size() )
		m_emphasizedRanges[i++] -= sz;
}
void Buffer::rebuildLineMgr()
{
	m_lineMgr->init();
	pos_t pos = 0;
	while( pos != size() ) {
		wchar_t ch = charAt(pos++);
		if( ch == '\r' && pos < size() && charAt(pos) == '\n' )
			++pos;
		if( ch == '\r' || ch == '\n' )
			m_lineMgr->push_back(pos);
	}
	if( !isBlankEOFLine() )
		m_lineMgr->push_back(pos);		//	最終行作成
}
wchar_t *Buffer::data()
{
	return m_buffer->data();
}
const wchar_t *Buffer::raw_data(pos_t pos) const
{
	return m_buffer->raw_data(pos);
}
bool Buffer::getText(pos_t pos, wchar_t *buf, int length) const
{
	if( length <= 0 ) return false;
	//Q_ASSERT( length > 0 );
	int n = m_buffer->get_data(pos, buf, length);
	return n == length;
}
bool Buffer::getText(pos_t pos, ssize_t sz, std::vector<wchar_t> &v) const
{
	v.resize(sz);
	int n = m_buffer->get_data(pos, &v[0], sz);
	return n == sz;
}
bool Buffer::insertText(pos_t pos, cwchar_t *first, ssize_t sz, line_t ln)
{
	if( ln < 0 ) ln = m_lineMgr->positionToLine(pos);
	if( m_editedPos.size() >= EDIT_POS_SIZE )
		m_editedPos.pop_front();
	if( !basicInsertText(pos, first, sz, ln) )
		return false;		//	メモリ不足の場合
	for (auto itr = m_editedPos.begin(); itr != m_editedPos.end(); ++itr) {
		if( *itr > pos )
			*itr += sz;
	}
	m_undoMgr->push_back_insText(pos, sz, ln);
	m_modified = true;
	++m_seqNumber;
	if( !m_editedPos.empty() && m_editedPos.back() == pos )	//	連続挿入の場合
		m_editedPos[m_editedPos.size() - 1] = pos + sz;
	else
		m_editedPos.push_back(pos + sz);
	m_epix = m_editedPos.size() - 1;
	return true;
}
bool Buffer::deleteText(pos_t pos, ssize_t sz, bool BS, line_t ln)
{
	if( ln < 0 ) ln = m_lineMgr->positionToLine(pos);
	if( m_editedPos.size() >= EDIT_POS_SIZE )
		m_editedPos.pop_front();
	m_undoMgr->push_back_delText(pos, sz, BS, ln);
	basicDeleteText(pos, sz, ln);
	m_lineMgr->setLineFlag(ln, LINEFLAG_MODIFIED);		//	???
	m_lineMgr->resetLineFlag(ln, LINEFLAG_SAVED);
	m_modified = true;
	++m_seqNumber;
	for (auto itr = m_editedPos.begin(); itr != m_editedPos.end(); ++itr) {
		if( *itr >= pos )
			*itr -= sz;
	}
	if( !m_editedPos.empty() && m_editedPos.back() != pos )	//	連続削除でない場合
		m_editedPos.push_back(pos);
	m_epix = m_editedPos.size() - 1;
	return true;
}
//	置換、undo 対応
bool Buffer::replaceText(pos_t pos, ssize_t dsz, cwchar_t *first, int isz, line_t ln,
											bool updateEditedPos)
{
	if( ln < 0 ) ln = m_lineMgr->positionToLine(pos);
#if	1
	m_undoMgr->push_back_delText(pos, dsz, false, ln);
	basicDeleteText(pos, dsz, ln);
	m_lineMgr->setLineFlag(ln, LINEFLAG_MODIFIED);		//	???
	m_lineMgr->resetLineFlag(ln, LINEFLAG_SAVED);
	if( !basicInsertText(pos, first, isz, ln) )
		return false;		//	メモリ不足の場合
	m_undoMgr->push_back_insText(pos, isz, ln);
	m_modified = true;
	++m_seqNumber;
	if( updateEditedPos ) {
		m_editedPos.push_back(pos);
		if( m_editedPos.size() > EDIT_POS_SIZE )
			m_editedPos.pop_front();
		m_epix = m_editedPos.size() - 1;
	}
#else
	UndoActionReplace *ptr = m_undoMgr->push_back_repText(pos, dsz, isz, ln);
	if( ptr == 0 ) return false;
	basicReplaceText(pos, dsz, first, isz, ln);
	line_t ln2 = positionToLine(pos + isz);
	ptr->m_lcIns = ln2 - ln + 1;
	m_modified = true;
	++m_seqNumber;
#endif
	return true;
}
//----------------------------------------------------------------------
//	行管理関連
int Buffer::lineCount() const		//	空のEOF行を含まない行数
{
	return m_lineMgr->lineCount() - 1;
}
int Buffer::EOFLine() const
{
	line_t ln = lineCount();
	if( !isBlankEOFLine() ) --ln;
	return ln;
}
pos_t Buffer::lineStartPosition(line_t line) const
{
	return m_lineMgr->lineStartPosition(line);
}
ssize_t Buffer::lineSize(line_t line) const
{
	return m_lineMgr->lineSize(line);
}
line_t Buffer::positionToLine(pos_t pos) const
{
	if( pos >= size() ) {
		if( isBlankEOFLine() )
			return m_lineMgr->lineCount() - 1;
		else
			return m_lineMgr->lineCount() - 2;
	}
	return m_lineMgr->positionToLine(pos);
}
//----------------------------------------------------------------------
//		undo/redo 関連
bool Buffer::canUndo() const
{
	return m_undoMgr->canUndo();
}
bool Buffer::canRedo() const
{
	return m_undoMgr->canRedo();
}
int Buffer::undo()
{
	++m_seqNumber;
	return m_undoMgr->undo();
}
int Buffer::redo()
{
	++m_seqNumber;
	return m_undoMgr->redo();
}
void Buffer::openUndoBlock()
{
	m_undoMgr->openBlock();
}
void Buffer::closeUndoBlock()
{
	m_undoMgr->closeBlock();
}
void Buffer::closeAllUndoBlock()
{
	m_undoMgr->closeAllBlock();
}
void Buffer::clearUndoMgr()
{
	m_undoMgr->init();
}
void Buffer::prohibitMergeUndo()			//	挿入マージ禁止
{
	m_undoMgr->prohibitMergeUndo();
}
//----------------------------------------------------------------------
//
void Buffer::print() const
{
	std::cout << "size = " << size() << "\n";
	std::wstring txt;
	for(int i = 0; i < size(); ++i) {
		wchar_t ch = charAt(i);
		switch( ch ) {
		case '\r':	txt += L"\\r";	break;
		case '\n':	txt += L"\\n";	break;
		default:	txt += ch;		break;
		}
	}
	std::wcout << txt << "\n";
	for(line_t ln = 0; ln < m_lineMgr->lineCount(); ++ln) {
		std::cout << ln << ":" << m_lineMgr->lineStartPosition(ln) << "\n";
	}
}
wchar_t Buffer::operator[](pos_t pos) const
{
	return m_buffer->operator[](pos);
}
bool Buffer::operator==(const Buffer &x) const
{
	if( *m_buffer != *x.m_buffer ) {
		std::cout << "not equal buffers\n";
		return false;
	}
	if( m_lineMgr->lineCount() != x.m_lineMgr->lineCount() ) {
		std::cout << "not equal lineMgr.size(), "
					<< m_lineMgr->lineCount() << x.m_lineMgr->lineCount() << "\n";
		return false;
	}
	for(int i = 0; i != m_lineMgr->lineCount(); ++i) {
		if( m_lineMgr->lineStartPosition(i) != x.m_lineMgr->lineStartPosition(i) ) {
			std::cout << "not equal lineStart at" << i
						<< "(" << m_lineMgr->lineStartPosition(i)
						<< x.m_lineMgr->lineStartPosition(i) << ")" << "\n";
			return false;
		}
	}
	return true;
}
bool Buffer::isMatched(cwchar_t *pat, ssize_t sz, pos_t pos) const
{
	while( pos < size() ) {
		if( *pat++ != m_buffer->operator[](pos++) ) break;
		if( --sz <= 0 ) return true;
	}
	return false;
}
bool Buffer::isMatchedIC(cwchar_t *pat, ssize_t sz, pos_t pos) const
{
	while( pos < size() ) {
		if( tolower(*pat++) != tolower(m_buffer->operator[](pos++)) ) break;
		if( --sz <= 0 ) return true;
	}
	return false;
}
bool Buffer::isMatched(cwchar_t *pat, pos_t pos) const
{
	while( pos < size() ) {
		if( *pat++ != m_buffer->operator[](pos++) ) break;
		if( *pat == '\0' ) return true;
	}
	return false;
}
bool Buffer::isMatchedIC(cwchar_t *pat, pos_t pos) const
{
	while( pos < size() ) {
		if( tolower(*pat++) != tolower(m_buffer->operator[](pos++)) ) break;
		if( *pat == '\0' ) return true;
	}
	return false;
}
pos_t Buffer::strstr(cwchar_t *pat, ssize_t sz, pos_t pos, pos_t last, bool ic) const
{
	if( last < 0 )
		last = size();
	if( !ic ) {
		while( pos <= last - sz ) {
			if( isMatched(pat, sz, pos) ) return pos;
			++pos;
		}
	} else {
		while( pos <= last - sz ) {
			if( isMatchedIC(pat, sz, pos) ) return pos;
			++pos;
		}
	}
	return -1;
}
pos_t Buffer::strrstr(cwchar_t *pat, ssize_t sz, pos_t pos, pos_t last, bool ic) const
{
	if( pos < 0 || pos > size() )
		pos = size();
	if( !ic ) {
		while( --pos >= last ) {
			if( isMatched(pat, pos) ) return pos;
		}
	} else {
		while( --pos >= last ) {
			if( isMatchedIC(pat, pos) ) return pos;
		}
	}
	return -1;
}
pos_t Buffer::indexOf(SSSearch &sss, cwchar_t *pat, ssize_t sz, pos_t pos, uint opt, pos_t last, byte algorithm) const
{
	//uint opt = 0;
	//if( ic ) opt |= SSSearch::IGNORE_CASE;
	if( !sss.setup(pat, sz, opt, algorithm) ) return -1;
	return sss.strstr(*this, pos, last);
}
pos_t Buffer::rIndexOf(SSSearch &sss, cwchar_t *pat, ssize_t sz, pos_t from, uint opt, pos_t last, byte algorithm) const
{
	//uint opt = 0;
	//if( ic ) opt |= SSSearch::IGNORE_CASE;
	if( !sss.setup(pat, sz, opt, algorithm) ) return -1;
	return sss.strrstr(*this, from, last /*, algorithm*/);
}

int Buffer::replaceAll(cwchar_t *before, ssize_t bsz, cwchar_t *after, ssize_t asz, uint opt, byte algorithm)
{
	pos_t first = 0;
	pos_t last = -1;
	pos_t lastPos = -1;
	return replaceAll(before, bsz, after, asz, opt, algorithm, first, last, lastPos);
}
bool includeSubMatch(const wchar_t *ptr, ssize_t sz)
{
	const wchar_t *last = ptr + sz;
	while( ptr != last ) {
		if( *ptr == '\\' && ptr + 1 != last && isDigit(ptr[1]) )
			return true;
		++ptr;
	}
	return false;
}
std::wstring escapeText(const std::wstring &text)
{
	std::wstring text2;
	text2.reserve(text.size());
	const wchar_t *ptr = text.c_str();
	const wchar_t *last = ptr + text.size();
	while( ptr < last ) {
		wchar_t ch = *ptr++;
		if( ch == '\\' && ptr < last ) {
			switch( (ch = *ptr++) ) {
				case 't':	ch = '\t';		break;
				case 'n':	ch = '\n';	break;
				case 'r':	ch = '\r';		break;
			}
			text2.push_back(ch);
		} else
			text2.push_back(ch);
	}
	return text2;
}
std::wstring replaceText(const std::wstring &text,
								const std::match_results<const wchar_t *> &mr)
{
	std::wstring text2;
	text2.reserve(text.size());
	const wchar_t *ptr = text.c_str();
	const wchar_t *last = ptr + text.size();
	while( ptr < last ) {
		wchar_t ch = *ptr++;
		if( ch == '\\' && ptr < last ) {
			if( isDigit(*ptr) ) {
				int n = *ptr++ - '0';
				if( n < mr.size() ) {
					std::wstring str = mr[n].str();
					text2.insert(text2.end(), str.begin(), str.end());
				}
			} else {
				switch( (ch = *ptr++) ) {
					case 't':	ch = '\t';		break;
					case 'n':	ch = '\n';	break;
					case 'r':	ch = '\r';		break;
				}
				text2.push_back(ch);
			}
		} else
			text2.push_back(ch);
	}
	return text2;
}
int Buffer::replaceAll(cwchar_t *before, ssize_t bsz, cwchar_t *after, ssize_t asz0, uint opt, byte algorithm,
						pos_t first, pos_t &last, pos_t &lastPos, bool global)
{
	m_editedPos.clear();
	lastPos = -1;
	int cnt = 0;
	pos_t pos = first;
	if( last < 0 ) last = size();
	std::auto_ptr<SSSearch> sss(new SSSearch());
	sss->setup(before, bsz, opt, algorithm);
	openUndoBlock();
	std::wstring atext0(after, asz0);
	//bool submatch = algorithm == SSSearch::STD_REGEX
	//						&& includeSubMatch(after, asz0);
	std::wstring atext;
		atext = atext0;
	int asz = atext.size();
	while( (pos = sss->strstr(*this, pos, last)) >= 0 ) {
		if( algorithm == SSSearch::STD_REGEX ) {
			//	部分参照処理＆エスケープ処理
			atext = ::replaceText(atext0, sss->mresult());
			asz = atext.size();
		}
		lastPos = pos;
		line_t ln = positionToLine(pos);		//??? パフォーマンス
#if 0
		//	13/08/17 行フラグを保存しない場合で、2.04秒/100万行
		const int bits = sizeof(wchar_t) * 8 / 2;
		line_t ln2 = positionToLine(pos + bsz);
		int lcDel = ln2 - ln + 1;
		int msz = (lcDel + bits - 1) / bits;
		int ixDel = m_undoMgr->m_delText.size();
		m_undoMgr->m_delText.resize(ixDel + bsz + msz);
		getText(pos, &m_undoMgr->m_delText[ixDel], bsz);
		basicDeleteText(pos, bsz, ln);
		basicInsertText(pos, after, asz, ln);
		ln2 = positionToLine(pos + asz);
		int lcIns = ln2 - ln + 1;
		msz = (lcIns + bits - 1) / bits;
		UndoActionReplace *act = m_undoMgr->m_actRepPool.construct();
		act->m_pos = pos;
		act->m_ln = ln;
		act->m_sizeDel = bsz;
		act->m_ixDel = ixDel;
		act->m_lcDel = lcDel;
		act->m_sizeIns = asz;
		//act->m_ixIns = ixIns;
		act->m_lcIns = lcIns;
		m_undoMgr->push_back(act);
#else
		replaceText(pos, sss->matchLength(), atext.c_str(), asz, ln, false);
		//deleteText(pos, sss->matchLength(), false, ln);
		//insertText(pos, atext.c_str(), asz, ln);
#endif
		pos += asz;
		last += asz - sss->matchLength();
		if( !global ) {
			while( pos < last ) {
				wchar_t ch = charAt(pos++);
				if( ch == '\r' ) {
					if( charAt(pos) == '\n' ) ++pos;
					break;
				} else if( ch == '\n' )
					break;
			}
		}
		++cnt;
	}
	closeUndoBlock();
	return cnt;
}
int Buffer::markPos(char ch) const
{
	return m_markMgr->markPos(ch);
}
char Buffer::isMarked(pos_t pos) const
{
	return m_markMgr->isMarked(pos);
}
void Buffer::getMarks(std::vector<MarkItem> &lst) const
{
	m_markMgr->getMarks(lst);
}
//	ch = ['a', 'z'], pos < 0 for clear Mark
void Buffer::setMark(pos_t pos, char ch)
{
	m_markMgr->setMark(pos, ch);
}
void Buffer::clearMark(char ch)
{
	m_markMgr->clearMark(ch);
}
void Buffer::clearMark(pos_t pos)
{
	m_markMgr->clearMark(pos);
}
bool Buffer::isWordBegin(pos_t pos) const
{
	if( !pos ) return true;
	wchar_t pch;
	if( (pch = charAt(pos-1)) <= ' ' ) return true;
	byte p = UTF16CharType(pch);
	byte t = UTF16CharType(charAt(pos));
	return p != t;
}
bool Buffer::isWordEnd(pos_t pos) const
{
	if( pos >= size() ) return true;
	///if( pos == 1 ) return false;
	wchar_t ch;
	if( (ch = charAt(pos)) <= ' ' ) return true;
	byte p = UTF16CharType(charAt(pos - 1));
	byte t = UTF16CharType(ch);
	return p != t;
}
int Buffer::gapIndex() const
{
	return m_buffer->gapIndex();
}
void Buffer::setGapIndex(int ix) const
{
	m_buffer->setGapIndex(ix);
}
int Buffer::undoIndex() const
{
	return m_undoMgr->undoIndex();
}
bool Buffer::isUndoBlockOpened() const
{
	return m_undoMgr->isBlockOpened();
}
#if		0
ViewLineMgr *Buffer::createViewLineMgr()
{
	ViewLineMgr *ptr = new ViewLineMgr(this);
	m_viewLineMgrs.push_back(ptr);
	return ptr;
}
#endif
int Buffer::nextEditedPos()
{
	if( m_editedPos.empty() )
		return -1;
	if( m_epix < m_editedPos.size() - 1)
		++m_epix;
	return m_editedPos[m_epix];
}
int Buffer::prevEditedPos()
{
	if( m_editedPos.empty() )
		return -1;
	if( m_epix != 0 )
		--m_epix;
	return m_editedPos[m_epix];
}
