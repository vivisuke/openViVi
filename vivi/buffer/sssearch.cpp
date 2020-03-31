//----------------------------------------------------------------------
//
//			File:			"sssearch.cpp"
//			Created:		06-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include "sssearch.h"
#include "Buffer.h"
#include <algorithm>    // std::search

inline bool isNewLineChar(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}

SSSearch::SSSearch()
	: m_algorithm(STRSTR)
	, m_opt(0)
	, m_ignoreCase(false)
	, m_CV(0)
	, m_strstrAFunc(0)
	, m_strstrWFunc(0)
{
}
SSSearch::~SSSearch()
{
	delete m_CV;
}
void SSSearch::clear()
{
	delete m_CV;
	m_CV = 0;
	m_algorithm = 0;
	m_opt = 0;
	m_ignoreCase = false;
	m_plen = 0;
	m_strstrAFunc = 0;
	m_strstrWFunc = 0;
}
/*

	逆方向検索の場合
                              ↓pos
	┌─────────┬─┬───────────────┐
    │                  │ch│                              │
	└─────────┴─┴───────────────┘
*/
bool SSSearch::setup(const wchar_t *pat, int plen, uint opt, uchar algorithm)
{
	//if( algorithm == SAKUSAKU ) algorithm = QUICK_SEARCH;		//	暫定処理
	delete m_CV;
	m_CV = 0;
	m_opt = opt;
	if( (opt & REGEXP) != 0 )
		m_algorithm = STD_REGEX;
	else
		m_algorithm = algorithm;
	m_ignoreCase = (opt & IGNORE_CASE) != 0;
	m_wholeWordOnly = (opt & WHOLE_WORD_ONLY) != 0;
	m_plen = m_matchLength = plen;
	m_strstrAFunc = 0;
	m_strstrWFunc = 0;
	if( !plen ) return false;
	m_wpat = std::wstring(pat, pat + plen);
	if( plen == 1 && m_algorithm != STD_REGEX ) {
		m_strstrWFunc = &SSSearch::w_strchr;
		return true;
	}
	const int WORD_SIZE = 0x10000;
	RegType mask;
	switch( m_algorithm ) {
	case QUICK_SEARCH:
QS:
		if( plen > 0xffff ) return false;
		for(int i = 0; i < 0x100; ++i)
			m_skipTable[i] = m_rskipTable[i] = plen + 1;
		if( !m_ignoreCase ) {
			for(int i = 0; i < plen; ++i)					//	for 順方向検索
				m_skipTable[(uchar)m_wpat[i]] = plen - i;	//	下位バイトのみ参照
			for(int i = plen - 1; i >= 0; --i)				//	for 逆方向検索
				m_rskipTable[(uchar)m_wpat[i]] = i + 1;		//	下位バイトのみ参照
			m_strstrWFunc = &SSSearch::w_quick_search;
		} else {
			for(int i = 0; i < plen; ++i) {
				if( m_wpat[i] < 0x80 ) {
					m_skipTable[(uchar)toupper(m_wpat[i])] = plen - i;	//	下位バイトのみ参照
					m_skipTable[(uchar)(m_wpat[i] = tolower(m_wpat[i]))] = plen - i;	//	下位バイトのみ参照
				} else
					m_skipTable[(uchar)m_wpat[i]] = plen - i;	//	下位バイトのみ参照
			}
			for(int i = plen - 1; i >= 0; --i) {				//	for 逆方向検索
				if( m_wpat[i] < 0x80 ) {
					m_rskipTable[(uchar)toupper(m_wpat[i])] = i + 1;	//	下位バイトのみ参照
					m_rskipTable[(uchar)(m_wpat[i])] = i + 1;		//	下位バイトのみ参照、小文字化済み
				} else
					m_rskipTable[(uchar)m_wpat[i]] = i + 1;	//	下位バイトのみ参照
			}
			m_strstrWFunc = &SSSearch::w_quick_search_ic;
		}
		break;
	case SHIFT_AND:
	case SAKUSAKU:
		if( plen > 32 ) goto QS;
		for(int i = 0; i < 0x100; ++i)					//	skip テーブル初期化
			m_skipTable[i] = m_rskipTable[i] = plen;
		m_CV = new RegType[WORD_SIZE]();		//	() for 0 で初期化
		if( !m_ignoreCase ) {
			for(int i = 0, mask = 1; i < plen; ++i, mask<<=1)
				m_CV[pat[i]] |= mask;
			for(int i = 0; i < plen; ++i)					//	for 順方向検索
				m_skipTable[(uchar)pat[i]] = plen - i - 1;
		} else {
			for(int i = 0, mask = 1; i < plen; ++i, mask<<=1) {
				m_CV[tolower(pat[i])] |= mask;
				m_CV[toupper(pat[i])] |= mask;
			}
			for(int i = 0; i < plen; ++i) {					//	for 順方向検索
				m_skipTable[(uchar)tolower(pat[i])] = plen - i - 1;
				m_skipTable[(uchar)toupper(pat[i])] = plen - i - 1;
			}
		}
		if( algorithm == SHIFT_AND )
			m_strstrWFunc = &SSSearch::w_bitmap_strstr;
		else
			m_strstrWFunc = &SSSearch::w_sakusaku_strstr;
		break;
	case BNDM: 
		if( plen > 32 ) goto QS;
		m_CV = new RegType[WORD_SIZE]();		//	() for 0 で初期化
		if( !m_ignoreCase ) {
			for(int i = plen - 1, mask = 1; i >= 0; --i, mask<<=1)
				m_CV[pat[i]] |= mask;
		} else {
			for(int i = plen - 1, mask = 1; i >= 0; --i, mask<<=1) {
				m_CV[tolower(pat[i])] |= mask;
				m_CV[toupper(pat[i])] |= mask;
			}
		}
		m_strstrWFunc = &SSSearch::w_BNDM;
		break;
	case FORWARD_SBNDM: 
		if( plen > 31 ) goto QS;
		m_strstrWFunc = &SSSearch::w_FORWARD_SBNDM;
		m_CV = new RegType[WORD_SIZE];
		for(int i = 0; i < WORD_SIZE; ++i)
			m_CV[i] = 1;
		if( !m_ignoreCase ) {
			for(int i = plen, mask = 2; --i >= 0; mask<<=1)
				m_CV[pat[i]] |= mask;
		} else {
			for(int i = plen, mask = 2; --i >= 0; mask<<=1) {
				m_CV[tolower(pat[i])] |= mask;
				m_CV[toupper(pat[i])] |= mask;
			}
		}
		break;
	case STD_REGEX:
		try {
			if( !m_ignoreCase )
				m_wregex.assign(pat, plen);
			else
				m_wregex.assign(pat, plen, std::regex::ECMAScript | std::regex::icase);
		} catch(...) {
			return false;
		}
		m_strstrWFunc = &SSSearch::w_STD_REGEX;
		break;
	case STRSTR:
	default:
		m_strstrWFunc = &SSSearch::w_strstr;
		break;
	}
#if 0
	else if( !(opt & SHIFT_AND) /*|| plen > 32*/ ) {
		//	QuickSearch
	} else {	//	ビットマップアルゴリズム
		const int WORD_SIZE = 0x10000;
		if( plen <= sizeof(RegType)*8 ) {
			m_nreg = 1;
			if( m_CV == 0 )
				m_CV = new RegType[WORD_SIZE]();		//	0 initialized
			else
				memset(m_CV, 0, WORD_SIZE*sizeof(RegType));
			for(int i = 0; i < 0x100; ++i)					//	skip テーブル初期化
				m_skipTable[i] = m_rskipTable[i] = plen;
			if( !m_ignoreCase ) {
				RegType mask = 1;
				for(int i = 0; i < plen; ++i, mask<<=1)
					m_CV[pat[i]] |= mask;
				for(int i = 0; i < plen; ++i)					//	for 順方向検索
					m_skipTable[(uchar)m_wpat[i]] = plen - i - 1;	//	下位バイトのみ参照
			} else {
				RegType mask = 1;
				for(int i = 0; i < plen; ++i, mask<<=1) {
					m_CV[toupper(pat[i])] |= mask;
					m_CV[tolower(pat[i])] |= mask;
				}
				for(int i = 0; i < plen; ++i) {
					if( m_wpat[i] < 0x80 ) {
						m_skipTable[(uchar)toupper(m_wpat[i])] = plen - i - 1;	//	下位バイトのみ参照
						m_skipTable[(uchar)(m_wpat[i] = tolower(m_wpat[i]))] = plen - i - 1;	//	下位バイトのみ参照
					} else
						m_skipTable[(uchar)m_wpat[i]] = plen - i - 1;	//	下位バイトのみ参照
				}
			}
			m_strstrWFunc = &SSSearch::bitmap_strstr;
		} else {
			m_nreg = (plen + 31) / 32;		//	必要な32bitレジスタ数
			if( m_CV != 0 ) delete m_CV;
			m_CV = new RegType[m_nreg * WORD_SIZE]();		//	0 initialized
			int i = 0;
			int offset = 0;
			if( !m_ignoreCase ) {
				for(int r = 0; r < m_nreg; ++r) {
					RegType mask = 1;
					for(int k = 0; k < 32 && i < plen; ++k, mask<<=1)
						m_CV[pat[i++] + offset] |= mask;
					offset += WORD_SIZE;
				}
			} else {
				for(int r = 0; r < m_nreg; ++r) {
					RegType mask = 1;
					for(int k = 0; k < 32 && i < plen; ++k, mask<<=1, ++i) {
						m_CV[toupper(pat[i]) + offset] |= mask;
						m_CV[tolower(pat[i]) + offset] |= mask;
					}
					offset += WORD_SIZE;
				}
			}
			m_strstrWFunc = &SSSearch::bitmap_strstr_ex;
		}
	}
#endif
	return true;
}
bool SSSearch::setup(const char *pat, int plen, uint opt, uchar algorithm)
{
	//if( algorithm == SAKUSAKU ) algorithm = QUICK_SEARCH;		//	暫定処理
	delete m_CV;
	m_CV = 0;
	m_algorithm = algorithm;
	m_opt = opt;
	m_ignoreCase = (opt & IGNORE_CASE) != 0;
	m_wholeWordOnly = (opt & WHOLE_WORD_ONLY) != 0;
	m_plen = m_matchLength = plen;
	m_strstrAFunc = 0;
	m_strstrWFunc = 0;
	if( !plen ) return false;
	m_pat = std::string(pat, pat + plen);
	if( plen == 1 ) {
		m_strstrAFunc = &SSSearch::a_strchr;
		return true;
	}
	const int WORD_SIZE = 0x100;
	RegType mask;
	switch( algorithm ) {
	case QUICK_SEARCH:
		m_strstrAFunc = &SSSearch::a_quick_search;
		for(int i = 0; i < 0x100; ++i)					//	skip テーブル初期化
			m_skipTable[i] = m_rskipTable[i] = plen + 1;
		if( !m_ignoreCase ) {
			for(int i = 0; i < plen; ++i)					//	for 順方向検索
				m_skipTable[(uchar)pat[i]] = plen - i;
		} else {
			for(int i = 0; i < plen; ++i) {					//	for 順方向検索
				m_skipTable[(uchar)tolower(pat[i])] = plen - i;
				m_skipTable[(uchar)toupper(pat[i])] = plen - i;
				m_pat[i] = tolower(m_pat[i]);		//	小文字化しておく
			}
		}
		break;
	case SHIFT_AND:
	case SAKUSAKU:
		if( plen > 32 ) goto SS;
		for(int i = 0; i < 0x100; ++i)					//	skip テーブル初期化
			m_skipTable[i] = m_rskipTable[i] = plen;
		m_CV = new RegType[WORD_SIZE]();		//	() for 0 で初期化
		if( !m_ignoreCase ) {
			for(int i = 0, mask = 1; i < plen; ++i, mask<<=1)
				m_CV[pat[i]] |= mask;
			for(int i = 0; i < plen; ++i)					//	for 順方向検索
				m_skipTable[(uchar)pat[i]] = plen - i - 1;
		} else {
			for(int i = 0, mask = 1; i < plen; ++i, mask<<=1) {
				m_CV[tolower(pat[i])] |= mask;
				m_CV[toupper(pat[i])] |= mask;
			}
			for(int i = 0; i < plen; ++i) {					//	for 順方向検索
				m_skipTable[(uchar)tolower(pat[i])] = plen - i - 1;
				m_skipTable[(uchar)toupper(pat[i])] = plen - i - 1;
			}
		}
		if( algorithm == SHIFT_AND )
			m_strstrAFunc = &SSSearch::a_bitmap_strstr;
		else
			m_strstrAFunc = &SSSearch::a_sakusaku_strstr;
		break;
	case BNDM: 
		if( plen > 32 ) goto SS;
		m_strstrAFunc = &SSSearch::a_BNDM;
		m_CV = new RegType[WORD_SIZE]();		//	() for 0 で初期化
		if( !m_ignoreCase ) {
			for(int i = plen - 1, mask = 1; i >= 0; --i, mask<<=1)
				m_CV[(uchar)pat[i]] |= mask;
		} else {
			for(int i = plen - 1, mask = 1; i >= 0; --i, mask<<=1) {
				m_CV[(uchar)tolower(pat[i])] |= mask;
				m_CV[(uchar)toupper(pat[i])] |= mask;
			}
		}
		break;
	case FORWARD_SBNDM: 
		if( plen > 31 ) goto SS;
		m_strstrAFunc = &SSSearch::a_FORWARD_SBNDM;
		m_CV = new RegType[WORD_SIZE];
		for(int i = 0; i < WORD_SIZE; ++i)
			m_CV[i] = 1;
		if( !m_ignoreCase ) {
			for(int i = plen, mask = 2; --i >= 0; mask<<=1)
				m_CV[(uchar)pat[i]] |= mask;
		} else {
			for(int i = plen, mask = 2; --i >= 0; mask<<=1) {
				m_CV[(uchar)tolower(pat[i])] |= mask;
				m_CV[(uchar)toupper(pat[i])] |= mask;
			}
		}
		break;
	case STRSTR:
	default:
SS:
		m_strstrAFunc = &SSSearch::a_strstr;
		break;
	}
	return true;
}
//	1文字検索
int SSSearch::w_strchr(const Buffer &buffer, int pos, int last)
{
	wchar_t ch = m_wpat[0];
	if( !m_ignoreCase ) {
		for(int i = pos; i < last; ++i)
			if( buffer[i] == ch ) return i;
	} else {
		ch = tolower(ch);
		for(int i = pos; i < last; ++i)
			if( tolower(buffer[i]) == ch ) return i;
	}
	return -1;
}
int SSSearch::w_quick_search(const Buffer &buffer, int pos, int last)
{
	int plen = m_wpat.size();
	int pend = last - plen - 1;
	while( pos < pend ) {
		int j = 0;
		while( j < plen ) {
			if( buffer[pos+j] != m_wpat[j] ) break;
			++j;
		}
		if( j == plen )
			return pos;
		pos += m_skipTable[(uchar)buffer[pos + plen]];
	}
	return buffer.strstr(m_wpat.c_str(), m_wpat.size(), pos, last);
}
int SSSearch::w_quick_search_ic(const Buffer &buffer, int pos, int last)
{
	int plen = m_wpat.size();
	int pend = last - plen - 1;
	while( pos < pend ) {
		int j = 0;
		while( j < plen ) {
			if( tolower(buffer[pos + j]) != m_wpat[j] ) break;
			++j;
		}
		if( j == plen )
			return pos;
		//if( text + plen >= tend ) break;
		pos += m_skipTable[(uchar)buffer[pos + plen]];		//	skip テーブルには大小文字共に登録しているので、変換不要
	}
	return buffer.strstr(m_wpat.c_str(), m_wpat.size(), pos, last, /*ic=*/true);
}
//	順方向検索
int SSSearch::strstr(const Buffer &buffer, int pos, int last /*, bool matchNotBOL*/)
{
	if( m_strstrWFunc == 0 ) return -1;
	//m_matchNotBOL = matchNotBOL;
	if( last < 0 || last > buffer.size() )
		last = buffer.size();
	if( !m_wholeWordOnly )
		return (this->*m_strstrWFunc)(buffer, pos, last);
	for(;;) {
		pos = (this->*m_strstrWFunc)(buffer, pos, last);
		if( pos < 0 ) return -1;
		if( buffer.isWordBegin(pos) && buffer.isWordEnd(pos+m_matchLength) )
			return pos;
		pos += m_plen;
	}
}
int SSSearch::strstr(const std::vector<char> &buffer, int pos, int last)
{
	if( m_strstrAFunc == 0 ) return -1;
	if( last < 0 ) last = buffer.size();
	return (this->*m_strstrAFunc)(buffer, pos, last);
}
int SSSearch::w_bitmap_strstr(const Buffer &buffer, int pos, int last)
{
	RegType R = 0;
	const int plen = m_wpat.size();
	RegType Matched = 1 << (plen - 1);
	//const int pend = buffer.size();
	while( pos < last ) {
		wchar_t ch = buffer[pos++];
		R = ((R<<1) + 1) & m_CV[ch];
		if( (R & Matched) ) return pos - plen;
	}
	return -1;
}
int SSSearch::w_sakusaku_strstr(const Buffer &buffer, int pos, int last)
{
	RegType R = 0;
	const int plen = m_wpat.size();
	const int plenMinus1 = plen - 1;
	const int plenMinus2 = plen - 2;
	RegType Matched = 1 << (plen - 1);
	//const int pend = buffer.size();
	while( pos < last ) {
		wchar_t ch = buffer[pos++];
		R = ((R<<1) + 1) & m_CV[ch];
		if( !R ) {	//	スキップ出来る場合はスキップ
			int k = pos + plenMinus1;
			for(;;) {
				if( k >= last ) return -1;
				uint v = m_CV[ch = buffer[k]];		//	パターン最後に対応する文字
				if( !v ) {
					k += plen;
				} else {
					if( plen < 3 ) break;
					uint v2 = m_CV[buffer[k - 1]];		//	もうひとつ前
					if( !(v &= (v2 << 1)) ) {
						k += plenMinus1;
					} else {
#if 0	//	64文字の場合は効果が無かった 13/09/12
						if( plen < 6 ) break;
						uint v2 = m_CV[buffer[k - 2]];		//	もうひとつ前
						if( !(v &= (v2 << 2)) )
							k += plenMinus2;
						else
#endif
							break;
					}
				}
			}
			pos = k - plenMinus1 + m_skipTable[(uchar)ch];	//	スキップテーブルは下位バイトのみ参照
			//pos += m_skipTable[(uchar)ch];		//	下位バイトのみ参照
		} else
			if( (R & Matched) ) return pos - plen;
	}
	return -1;
}
int SSSearch::w_sakusaku_strstr_ex(const Buffer &buffer, int pos, int last)
{
	return w_sakusaku_strstr_r(buffer, pos, 0, last);
}
int SSSearch::w_sakusaku_strstr_r(const Buffer &buffer, int pos, int last, int reg)
{
	const int WORD_SIZE = 0x10000;
	const int offset = reg * WORD_SIZE;
	RegType R = 0;
	int len = reg < m_nreg - 1 ? 32 : m_plen % 32;
	RegType Matched = 1 << (len - 1);
	//int pend = buffer.size();
	while( pos < last ) {
		wchar_t ch = buffer[pos++];
		R = ((R<<1) + 1) & m_CV[ch + offset];
		if( !R ) {		//	スキップ出来る場合はスキップ
			uint v;
			for(;;) {
				if( pos + len - 1 >= last ) return -1;
				v = m_CV[buffer[pos + len - 1] + offset];
				if( v != 0 ) break;
				pos += len;
			}
			while( !(v & Matched) ) {
				v <<= 1;
				++pos;
			}
		} else if( (R & Matched) ) {
			int t;
			if( reg == m_nreg - 1 || (t = w_sakusaku_strstr_r(buffer, pos, last, reg + 1)) == pos )
				return pos - len;
			if( t < 0 ) return -1;
			pos = t - 32;
		}
	}
	return -1;
}
//	逆方向検索		pos から検索し、last まで検索
int SSSearch::strrstr(const Buffer &buffer, int pos, int last)
{
	if( m_strstrAFunc == 0 && m_strstrWFunc == 0 )
		return -1;
	if( !pos ) return -1;
	if( !m_wholeWordOnly )
		return strrstr_sub(buffer, pos, last);
	for(;;) {
		pos = strrstr_sub(buffer, pos, last);
		if( pos < 0 ) return -1;
		if( buffer.isWordBegin(pos) && buffer.isWordEnd(pos+m_matchLength) )
			return pos;
	}
}
int SSSearch::strrstr_sub(const Buffer &buffer, int pos, int last)
{
	const int plen = m_wpat.size();
	switch( m_algorithm ) {
		case STD_REGEX:
			return w_STD_REGEX_REV(buffer, pos, last);
			break;
		case QUICK_SEARCH:
			if( --pos > buffer.size() - plen )
				pos = buffer.size() - plen;
			if( !m_ignoreCase ) {
				while( pos > 0 ) {
					int j = 0;
					while( j < plen ) {
						if( buffer[pos+j] != m_wpat[j] ) break;
						++j;
					}
					if( j == plen )
						return pos;
					const int ix = (uchar)buffer[pos - 1];
					pos -= m_rskipTable[ix];
				}
				if( buffer.isMatched(m_wpat.c_str(), m_wpat.size(), 0) )
					return 0;
			} else {
				while( pos > 0 ) {
					int j = 0;
					while( j < plen ) {
						if( tolower(buffer[pos+j]) != m_wpat[j] ) break;
						++j;
					}
					if( j == plen )
						return pos;
					const int ix = (uchar)buffer[pos - 1];
					pos -= m_rskipTable[ix];
				}
				if( buffer.isMatchedIC(m_wpat.c_str(), m_wpat.size(), 0) )
					return 0;
			}
			break;
		case SHIFT_AND: {
			RegType R = 0;
			RegType Matched = 1 << (plen - 1);
			int pend = buffer.size();
			while( pos >= 0 ) {
				wchar_t ch = buffer[--pos];
				R = ((R>>1) | Matched) & m_CV[ch];
				if( (R & 1) ) return pos;
			}
			break;
		}
		default:
		case STRSTR:
			return buffer.strrstr(m_wpat.c_str(), m_wpat.size(), pos, last, m_ignoreCase);
	}
	return -1;
}
//	1文字検索
int SSSearch::a_strchr(const std::vector<char> &buffer, int pos, int last)
{
	char ch = m_wpat[0];
	if( !m_ignoreCase ) {
		for(int i = pos; i < last; ++i)
			if( buffer[i] == ch ) return i;
	} else {
		ch = tolower(ch);
		for(int i = pos; i < last; ++i)
			if( tolower(buffer[i]) == ch ) return i;
	}
	return -1;
}
int SSSearch::w_strstr(const Buffer &buffer, int pos, int last)
{
	return buffer.strstr(&m_wpat[0], m_wpat.size(), pos, last, m_ignoreCase);
}
int SSSearch::a_strstr(const std::vector<char> &buffer, int pos, int last)
{
	const char *ptr;
	if( !m_ignoreCase ) {
		ptr = std::search(&buffer[0] + pos, &buffer[0] + last,
							&m_pat[0], &m_pat[0] + m_pat.size());
	} else {
		ptr = std::search(&buffer[0] + pos, &buffer[0] + last,
							&m_pat[0], &m_pat[0] + m_pat.size(),
							[](char lhs, char rhs) { return tolower(lhs) == tolower(rhs); });
	}
	if( ptr < &buffer[0] + last )
		return ptr - &buffer[0];
	return -1;
}
int SSSearch::a_quick_search(const std::vector<char> &buffer, int pos, int last)
{
	int plen = m_pat.size();
	int pend = last - plen - 1;
	const char *ptr;
	if( !m_ignoreCase ) {
		while( pos < pend ) {
			int j = 0;
			while( j < plen ) {
				if( buffer[pos+j] != m_pat[j] ) break;
				++j;
			}
			if( j == plen )
				return pos;
			pos += m_skipTable[(uchar)buffer[pos + plen]];
		}
		ptr = std::search(&buffer[0] + pos, &buffer[0] + buffer.size(),
							&m_pat[0], &m_pat[0] + m_pat.size());
	} else {
		while( pos < pend ) {
			int j = 0;
			while( j < plen ) {
				if( tolower(buffer[pos+j]) != m_pat[j] ) break;
				++j;
			}
			if( j == plen )
				return pos;
			pos += m_skipTable[(uchar)buffer[pos + plen]];
		}
		ptr = std::search(&buffer[0] + pos, &buffer[0] + buffer.size(),
							&m_pat[0], &m_pat[0] + m_pat.size(),
							[](char lhs, char rhs) { return tolower(lhs) == tolower(rhs); });
	}
	if( ptr != &buffer[0] + buffer.size() )
		return ptr - &buffer[0];
	return -1;
}
int SSSearch::a_quick_search_ic(const std::vector<char> &buffer, int pos, int last)
{
	return -1;
}
int SSSearch::a_bitmap_strstr(const std::vector<char> &buffer, int pos, int last)
{
	RegType R = 0;
	const int plen = m_pat.size();
	RegType Matched = 1 << (plen - 1);
	//const int pend = buffer.size();
	while( pos < last ) {
		//uchar ch = buffer[pos++];
		R = ((R<<1) + 1) & m_CV[(uchar)buffer[pos++]];
		if( (R & Matched) ) return pos - plen;
	}
	return -1;
}
int SSSearch::a_bitmap_strstr_ex(const std::vector<char> &, int pos, int last)
{
	return -1;
}
int SSSearch::a_bitmap_strstr_r(const std::vector<char> &, int pos, int last, int reg)
{
	return -1;
}
int SSSearch::a_sakusaku_strstr(const std::vector<char> &buffer, int pos, int last)
{
	RegType R = 0;
	const int plen = m_pat.size();
	const int plenMinus1 = plen - 1;
	const int plenMinus2 = plen - 2;
	RegType Matched = 1 << (plenMinus1);
	//const int pend = buffer.size();
	while( pos < last ) {
		//uchar ch = buffer[pos++];
		R = ((R<<1) + 1) & m_CV[buffer[pos++]];
		if( !R ) {	//	スキップ出来る場合はスキップ
			int k = pos + plenMinus1;
			uchar ch;
			for(;;) {
				if( k >= last ) return -1;
				uint v = m_CV[ch = buffer[k]];		//	パターン最後に対応する文字
				if( !v ) {
					k += plen;
				} else {
					if( plen < 3 ) break;
					uint v2 = m_CV[buffer[k - 1]];		//	もうひとつ前
					if( !(v &= (v2 << 1)) ) {
						k += plenMinus1;
					} else {
#if 0	//	64文字の場合は効果が無かった 13/09/12
						if( plen < 6 ) break;
						uint v2 = m_CV[buffer[k - 2]];		//	もうひとつ前
						if( !(v &= (v2 << 2)) )
							k += plenMinus2;
						else
#endif
							break;
					}
				}
			}
			//pos = k - plenMinus1;
			pos = k - plenMinus1 + m_skipTable[ch];
		} else
			if( (R & Matched) ) return pos - plen;
	}
	return -1;
}
int SSSearch::w_STD_REGEX(	const Buffer &buffer, int pos, int last)
{
	if( last < 0 ) last = buffer.size();
	int gi = buffer.gapIndex();
	int ln = buffer.positionToLine(gi);
	int ls = buffer.lineStartPosition(ln);
	buffer.setGapIndex(ls);		//	ギャップを行先頭位置に設定
	//std::match_results<const wchar_t *> mr;
	ln = buffer.positionToLine(pos);
	int offset = pos - buffer.lineStartPosition(ln);
	int nxls = buffer.lineStartPosition(ln);
	auto flags = std::regex_constants::match_default;
	if( offset != 0 ) {
		flags |= std::regex_constants::match_prev_avail;		//	regex_search に渡すポインタは行頭ではないフラグ
		//flags |= std::regex_constants::match_not_bol;
		//if( !buffer.isWordBegin(pos) )
		//	flags |= std::regex_constants::match_not_bow;
	}
	for(; ln < buffer.lineCount(); ++ln) {
		if( (ls = nxls) >= last ) break;
		nxls = buffer.lineStartPosition(ln + 1);
		const wchar_t *ptr = buffer.raw_data(ls) + offset;
		const wchar_t *endptr = ptr + (nxls - (ls + offset));
		while( endptr > ptr && isNewLineChar(endptr[-1]) )
			--endptr;
		//std::regex_constants::match_flag_type flags = std::regex_constants::match_default;
		//if( offset ) flags |= std::regex_constants::match_not_bol; 
		if( ptr < endptr && std::regex_search(ptr, endptr, m_mresult, m_wregex, flags) ) {
			m_matchLength = m_mresult.length(0);
			if( !m_matchLength ) break;
			pos = ls + offset + m_mresult.position(0);
			if( pos >= last ) break;
			return pos;
		}
		offset = 0;
		flags = std::regex_constants::match_default;
	}
	return -1;
}
int SSSearch::w_STD_REGEX_REV(	const Buffer &buffer,
												int first,		//	検索開始位置
												int last)		//	検索終了位置
{
	if( last < 0 ) last = 0;	//buffer.size();
	int gi = buffer.gapIndex();
	int ln = buffer.positionToLine(gi);
	int ls = buffer.lineStartPosition(ln);
	buffer.setGapIndex(ls);		//	行の途中にバッファが来ないように、ギャップを行先頭位置に設定
	//std::match_results<const wchar_t *> mr;
	ln = buffer.positionToLine(first);
	//int offset = first - buffer.lineStartPosition(ln);
	//ls = buffer.lineStartPosition(ln + 1);		//	次行位置を入れておく
	for(; ln >= 0; --ln) {
		int nxls = buffer.lineStartPosition(ln+1);
		ls = buffer.lineStartPosition(ln);
		const wchar_t *ptr = buffer.raw_data(ls);
		const wchar_t *endptr = ptr + (nxls - ls);
		while( endptr > ptr && isNewLineChar(endptr[-1]) ) --endptr;
#if		1
		if( std::regex_search(ptr, endptr, m_mresult, m_wregex) ) {
			int length = m_mresult.length(0);
			int pos = ls + m_mresult.position(0);
			if( pos < first ) {
				auto flags = std::regex_constants::match_default | std::regex_constants::match_prev_avail;
				auto ptr2 = ptr + m_mresult.position(0);
				while( std::regex_search(ptr2 + 1, endptr, m_mresult, m_wregex, flags) ) {	//	発見されなくなるまで検索
					ptr2 += 1 + m_mresult.position(0);		//	マッチ位置
					if( ls + (ptr2 - ptr) >= first ) break;		//	マッチ位置が検索開始位置を超えた場合
					pos = ls + (ptr2 - ptr);
					length = m_mresult.length(0);
				}
				m_matchLength = length;
				return pos;
			}
		}
#else
		//	13/10/11	複数箇所にマッチしてくれない
		auto flags = std::regex_constants::match_default | std::regex_constants::match_any;
		if( std::regex_search(ptr, endptr, mr, m_wregex) ) {
			int sz = mr.size();
			for(int i = mr.size(); --i >= 0; ) {
				int pos = ls + mr.position(i);
				if( pos < first ) {
					m_matchLength = mr.length(i);
					return pos;
				}
			}
		}
#endif
	}
	return -1;
}
int SSSearch::w_BNDM(const Buffer &buffer, int pos, int last)
{
	int pend = last - m_plen;
	while( pos <= pend ) {
		int i = m_plen - 1;
		int last = m_plen;
		RegType d = ~0;
		while( i >= 0 && d != 0 ) {
			d &= m_CV[buffer[pos + i]];
			--i;
			if( d != 0 ) {
				if( i >= 0 )
					last = i + 1;
				else
					return pos;
			}
			d <<= 1;
		}
		pos += last;
	}
	return -1;
}
int SSSearch::w_FORWARD_SBNDM(const Buffer &buffer, int pos, int last)
{
	const int mMinus1 = m_plen - 1;
	if( buffer.isMatched(m_wpat.c_str(), m_wpat.size(), pos) ) return pos;
	//if( !memcmp(&buffer[pos], m_pat.c_str(), m_plen) ) return pos;
	pos += m_plen;
	while( pos < last - 1 ) {
		uint R = (m_CV[buffer[pos + 1]] << 1) & m_CV[buffer[pos]];
		if( R != 0 ) {
			int j = pos;
			while( (R = (R<<1) & m_CV[buffer[pos - 1]]) )
				--pos;
			pos += mMinus1;
			if( pos == j )
				return j - mMinus1;
		}
		pos += m_plen;
	}
	return -1;
}
int SSSearch::a_BNDM(const std::vector<char> &buffer, int pos, int last)
{
	int pend = last - m_plen;
	while( pos <= pend ) {
		int i = m_plen - 1;
		int last = m_plen;
		RegType d = ~0;
		while( i >= 0 && d != 0 ) {
			d &= m_CV[(uchar)buffer[pos + i]];
			--i;
			if( d != 0 ) {
				if( i >= 0 )
					last = i + 1;
				else
					return pos;
			}
			d <<= 1;
		}
		pos += last;
	}
	return -1;
}
int SSSearch::a_FORWARD_SBNDM(	const std::vector<char> &buffer, int pos, int last)
{
	const int mMinus1 = m_plen - 1;
	if( !memcmp(&buffer[pos], m_pat.c_str(), m_plen) ) return pos;
	pos += m_plen;
	while( pos < last - 1 ) {
		uint R = (m_CV[(uchar)buffer[pos + 1]] << 1) & m_CV[(uchar)buffer[pos]];
		if( R != 0 ) {
			int j = pos;
			while( (R = (R<<1) & m_CV[(uchar)buffer[pos - 1]]) )
				--pos;
			pos += mMinus1;
			if( pos == j )
				return j - mMinus1;
		}
		pos += m_plen;
	}
	return -1;
}
