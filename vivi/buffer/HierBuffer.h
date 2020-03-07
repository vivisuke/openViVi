//----------------------------------------------------------------------
//
//			File:			"HierBuffer.h"
//			Created:		06-3ar-2020
//			Author:			津田伸秀
//			Description:	階層的データ構造のテキストバッファクラス
//
//----------------------------------------------------------------------

#pragma once

#include "gap_buffer.h"

template<typename Type>
class HierBuffer {
public:
	typedef Type value_type;
#ifdef		_WIN64
	typedef __int64 size_type;
	typedef __int64 ssize_t;
	typedef __int64 difference_type;	//	ギャップが無いとみなした場合のインデックス差
	typedef __int64 index_type;			//	バッファインデックス [0, size()]
	typedef __int64 pos_t;					//	バッファインデックス [0, size()]
#else
	typedef __int32 size_type;
	typedef __int32 ssize_t;
	typedef __int32 difference_type;	//	ギャップが無いとみなした場合のインデックス差
	typedef __int32 index_type;			//	バッファインデックス [0, size()]
	typedef __int32 pos_t;					//	バッファインデックス [0, size()]
#endif
	typedef Type& reference;
	typedef const Type& const_reference;
	typedef Type* pointer;
	typedef const Type* const_pointer;
public:
	HierBuffer()
		: m_size(0)
		, m_curPage(0)
	{
		m_buffer.push_back(gap_buffer<Type>());
	}
public:
	bool isEmpty() const { return !m_size; }
	bool empty() const { return !m_size; }
	size_type size() const { return m_size; }
public:
	void push_back(value_type v)
	{
		++m_size;
		m_buffer.back().push_back(v);
		//return true;
	}
private:
	gap_buffer<gap_buffer<Type>>	m_buffer;
	size_type	m_size;		//	データトータルサイズ
	int	m_curPage;		//	現ページ、ランダムアクセス時は現ページから優先して探索
	
};
