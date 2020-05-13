//----------------------------------------------------------------------
//
//			File:			"sssearch.h"
//			Created:		06-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_SSSEARCH_H
#define		_HEADER_SSSEARCH_H

#include <vector>
#include <string>
#include <regex>


typedef unsigned char byte;
typedef unsigned __int8 byte_t;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef uint RegType;

class Buffer;

class SSSearch
{
public:
	enum {
		//	検索オプション
		IGNORE_CASE = 0x01,
		WHOLE_WORD_ONLY = 0x02,					//	単語単位
		REGEXP = 0x04,
		//LOOP_SEARCH = 0x8000,

		STRSTR = 0,
		QUICK_SEARCH,
		SHIFT_AND,
		SAKUSAKU,
		BNDM,
		FORWARD_SBNDM,
		STD_REGEX,
	};
public:
	SSSearch();
	~SSSearch();

public:
	int		patSize() const { return m_plen; }
	int		matchLength() const { return m_matchLength; }
	std::wstring	pattern() const { return m_wpat; }
	int		mrSize() const { return m_mresult.size(); }
	//const wchar_t *subMatched(int i) const { return m_mresult[i].str; }
	const std::match_results<const wchar_t *> &mresult() const { return m_mresult; }
	const RegType *CV() const { return m_CV; }
	const ushort *skipTable() const { return m_skipTable; }

public:
	void	clear();
	//bool	setup(const wchar_t *pat, int sz, bool ic)
	//{ return setup(pat, sz, ic ? IGNORE_CASE : 0); }
	bool	setup(const wchar_t *pat, int sz, uint opt = 0, uchar algorithm = SAKUSAKU);
	bool	setup(const char *pat, int sz, uint opt = 0, uchar algorithm = SAKUSAKU);

public:
	int		strstr(const std::vector<char> &, int pos = 0, int last = -1);
	int		strstr(const Buffer &, int pos = 0, int last = -1 /*, bool = false*/);
	int		strrstr(const Buffer &, int pos, int last = -1);			//	pos より前で位置する場所を探す

protected:
	int		w_strstr(const Buffer &, int pos, int last);			//	1文字検索
	int		w_strchr(const Buffer &, int pos, int last);			//	1文字検索
	int		w_quick_search(		const Buffer &, int pos, int last);
	int		w_quick_search_ic(	const Buffer &, int pos, int last);
	int		w_bitmap_strstr(	const Buffer &, int pos, int last);
	int		w_bitmap_strstr_ex(	const Buffer &, int pos, int last);
	int		w_bitmap_strstr_r(	const Buffer &, int pos, int last, int reg);
	int		w_sakusaku_strstr(		const Buffer &, int pos, int last);
	int		w_sakusaku_strstr_ex(	const Buffer &, int pos, int last);
	int		w_sakusaku_strstr_r(	const Buffer &, int pos, int last, int reg);
	int		w_BNDM(	const Buffer &, int pos, int last);
	int		w_FORWARD_SBNDM(	const Buffer &, int pos, int last);
	int		w_STD_REGEX(	const Buffer &, int pos, int last);
	int		w_STD_REGEX_REV(	const Buffer &, int pos, int last);
	int		a_strchr(const std::vector<char> &, int pos, int last);			//	1文字検索
	int		a_strstr(const std::vector<char> &, int pos, int last);			//	ブルートフォース
	int		a_quick_search(		const std::vector<char> &, int pos, int last);
	int		a_quick_search_ic(	const std::vector<char> &, int pos, int last);
	int		a_bitmap_strstr(	const std::vector<char> &, int pos, int last);
	int		a_bitmap_strstr_ex(	const std::vector<char> &, int pos, int last);
	int		a_bitmap_strstr_r(	const std::vector<char> &, int pos, int last, int reg);
	int		a_sakusaku_strstr(		const std::vector<char> &, int pos, int last);
	int		a_sakusaku_strstr_ex(	const std::vector<char> &, int pos, int last);
	int		a_sakusaku_strstr_r(	const std::vector<char> &, int pos, int last, int reg);
	int		a_BNDM(	const std::vector<char> &, int pos, int last);
	int		a_FORWARD_SBNDM(	const std::vector<char> &, int pos, int last);
	int		strrstr_sub(const Buffer &, int pos, int last = -1);

private:
	uint	m_opt;
	bool	m_ignoreCase;
	bool	m_wholeWordOnly;		//	単語単位
	//bool	m_matchNotBOL;			//	^exp とマッチしない
	byte_t	m_algorithm;
	int		m_plen;
	int		m_matchLength;		//	マッチしたテキスト文字数
	int		m_nreg;					//	使用レジスタ数 for SHIFT_AND
	RegType	*m_CV;
	std::string		m_pat;
	std::wstring	m_wpat;
	int		(SSSearch::*m_strstrWFunc)(const Buffer &, int pos, int last);
	int		(SSSearch::*m_strstrAFunc)(const std::vector<char> &, int pos, int last);
	//int		m_match_length;		//	マッチしたテキスト長
	std::wregex	m_wregex;
	std::match_results<const wchar_t *> m_mresult;
	ushort 	m_skipTable[0x100];
	ushort 	m_rskipTable[0x100];
	//std::vector<wchar_t>	m_wpat;
};

#endif		//_HEADER_SSSEARCH_H
