//----------------------------------------------------------------------
//
//			File:			"MarkMgr.cpp"
//			Created:		21-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include "MarkMgr.h"

MarkMgr::MarkMgr()
{
	clear();
}
#if 1
void MarkMgr::clear()
{
	m_marks.clear();
}
int MarkMgr::indexOf(char ch) const
{
	for(int i = 0; i < m_marks.size(); ++i) {
		if( m_marks[i].m_ch == ch ) return i;
	}
	return -1;
}
int MarkMgr::indexOf(pos_t pos) const
{
	for(int i = 0; i < m_marks.size(); ++i) {
		if( m_marks[i].m_pos == pos ) return i;
	}
	return -1;
}
pos_t MarkMgr::markPos(char ch) const
{
	if( ch < 'a' || ch > 'z' ) return -1;
	int ix = indexOf(ch);
	if( ix < 0 ) return -1;
	return m_marks[ix].m_pos;
}
//	マークされていない場合は 0 を返す
char MarkMgr::isMarked(pos_t pos) const
{
	int ix = indexOf(pos);
	if( ix >= 0 )
		return m_marks[ix].m_ch;
	return '\0';
}
void MarkMgr::getMarks(std::vector<MarkItem> &lst) const
{
	lst.clear();
	for(int i = 0; i < m_marks.size(); ++i)
		lst.push_back(m_marks[i]);
}
bool MarkMgr::setMark(pos_t pos, char ch)
{
	if( ch == '\0' ) {
		bool v[26];
		for(int i = 0; i < sizeof(v)/sizeof(bool); ++i) v[i] = false;
		for(int i = 0; i < m_marks.size(); ++i)
			v[m_marks[i].m_ch - 'a'] = true;
		for(int i = 0; ; ++i) {
			if( i >= 26 ) return false;		//	空き無し
			if( !v[i] ) {
				ch = 'a' + i;
				break;
			}
		}
	} else {
		if( ch < 'a' || ch > 'z' ) return false;
		int ix = indexOf(ch);
		if( ix >= 0 )		//	マーク済みの場合
			m_marks.erase(ix);
	}
	int ix = 0;
	while( ix < m_marks.size() && m_marks[ix].m_pos < pos )
		++ix;
	m_marks.insert(ix, MarkItem(ch, pos));
	return true;
}
void MarkMgr::clearMark(char ch)
{
	if( ch < 'a' || ch > 'z' ) return;
	int ix = indexOf(ch);
	if( ix >= 0 )		//	マーク済みの場合
		m_marks.erase(ix);
}
void MarkMgr::clearMark(pos_t pos)
{
	int ix = indexOf(pos);
	if( ix >= 0 )		//	マーク済みの場合
		m_marks.erase(ix);
}
void MarkMgr::inserted(pos_t pos, ssize_t sz)
{
	if( m_marks.isEmpty() ) return;
	int i = 0;
	while( i < m_marks.size() && m_marks[i].m_pos < pos )
		++i;
	while( i < m_marks.size() )
		m_marks[i++].m_pos += sz;
}
void MarkMgr::deleted(pos_t pos, ssize_t sz)
{
	if( m_marks.isEmpty() ) return;
	int i = 0;
	while( i < m_marks.size() && m_marks[i].m_pos < pos )
		++i;
	int last = pos + sz;
	while( i < m_marks.size() && m_marks[i].m_pos < last )
		m_marks.erase(i);
	while( i < m_marks.size() )
		m_marks[i++].m_pos -= sz;
}
#else
void MarkMgr::clear()
{
	m_mkpos.clear();
	for(int i = 0; i < N_MARK; ++i)
		m_mkix[i] = -1;
}
int MarkMgr::markPos(char ch) const
{
	if( ch < 'a' || ch > 'z' ) return -1;
	int ix = m_mkix[ch - 'a'];
	if( ix < 0 )
		return -1;
	return m_mkpos[ix];
}
//	ch = ['a', 'z'], pos < 0 for clear Mark
void MarkMgr::setMark(char ch, pos_t pos)
{
	if( ch < 'a' || ch > 'z' ) return;
	clearMark(ch);
	int i = 0;
	while( i < m_mkpos.size() ) {	//	挿入すべき位置を取得
		if( m_mkpos[i] >= pos )
			break;
		++i;
	}
	for(int k = 0; k < N_MARK; ++k) {	//	挿入によりずれるindexを修正
		if( m_mkix[k] >= i )
			m_mkix[k] += 1;
	}
	m_mkpos.insert(i, pos);		//	挿入
	m_mkix[ch - 'a'] = i;
}
void MarkMgr::clearMark(char ch)
{
	if( ch < 'a' || ch > 'z' ) return;
	int ix = m_mkix[ch - 'a'];
	if( ix >= 0 ) {
		for(int k = 0; k < N_MARK; ++k) {
			if( m_mkix[k] >= ix )
				m_mkix[k] -= 1;
		}
		m_mkpos.erase(ix);
	}
}
void MarkMgr::inserted(pos_t pos, ssize_t sz)
{
	for(int i = 0; i < m_mkpos.size(); ++i) {
		if( m_mkpos[i] >= pos )
			m_mkpos[i] += sz;
	}
}
void MarkMgr::deleted(pos_t pos, ssize_t sz)
{
	if( m_mkpos.isEmpty() ) return;
	int last = pos + sz;
	for(int i = 0; i < m_mkix.size(); ++i) {
		int ix = m_mkix[i];
		if( ix >= 0 ) {
			if( m_mkpos[ix] >= last )
				m_mkpos[ix] -= sz;
			else if( m_mkpos[ix] >= pos )
		}
	}
}
#endif
