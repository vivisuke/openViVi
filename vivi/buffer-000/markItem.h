//----------------------------------------------------------------------
//
//			File:			"markItem.h"
//			Created:		12-10-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_MARKITEM_H
#define		_HEADER_MARKITEM_H

#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif

class EditView;

struct MarkItem
{
	char	m_ch;
	pos_t		m_pos;
	EditView	*m_view;
public:
	MarkItem(char ch = 0, pos_t pos = 0, EditView *view = 0)
		: m_ch(ch)
		, m_pos(pos)
		, m_view(view)
		{}
};

#endif		//_HEADER_MARKITEM_H
