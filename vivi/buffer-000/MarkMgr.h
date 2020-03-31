//----------------------------------------------------------------------
//
//			File:			"MarkMgr.h"
//			Created:		21-8-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_MARKMGR_H
#define		_HEADER_MARKMGR_H

#include "gap_buffer.h"
#include <vector>
#include <QDebug>
#include "Buffer.h"
#include "markItem.h"

#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号
	
class MarkMgr
{
	enum {
		N_MARK = 26,
	};
public:
	MarkMgr();
	~MarkMgr() {};

public:
	pos_t		markPos(char ch) const;
	char	isMarked(pos_t pos) const;	//	マークされていない場合は 0 を返す
	void	getMarks(std::vector<MarkItem> &) const;
	//int		position() const { return m_pos; }

public:
	void	clear();
	bool	setMark(pos_t pos, char ch = '\0');	//	ch : ['a', 'z'], pos < 0 for clear Mark
												//	ch : '\0' の場合は空いているマークを割り振る、空きが無いとエラー
	void	clearMark(char ch);
	void	clearMark(pos_t pos);
	void	inserted(pos_t pos, ssize_t sz);
	void	deleted(pos_t pos, ssize_t sz);

protected:
	int		indexOf(char ch) const;
	int		indexOf(pos_t pos) const;

private:
	gap_buffer<MarkItem>	m_marks;
#if 0
	gap_buffer<int>		m_mkpos;	//	マーク位置
	int		m_mkix[N_MARK];			//	'a'～'z' mark index of mkpos, -1 for unmarked
#endif
};


#endif		//_HEADER_MARKMGR_H
