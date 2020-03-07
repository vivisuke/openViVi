//----------------------------------------------------------------------
//
//			File:			"HierBuffer.h"
//			Created:		06-3ar-2020
//			Author:			津田伸秀
//			Description:	階層的データ構造のテキストバッファクラス
//
//----------------------------------------------------------------------

#pragma once

#include <assert.h>
#include "gap_buffer.h"

template<typename Type>
class HierBuffer {
public:
	enum {
		PAGE_MAX_SZ = 1024*1024,
	};
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
		, m_curFront(0)
	{
		m_buffer.push_back(new gap_buffer<Type>());
	}
	~HierBuffer()
	{
		for (int i = 0; i != m_buffer.size(); ++i) {
			delete m_buffer[i];
		}
	}
public:
	bool isEmpty() const { return !m_size; }
	bool empty() const { return !m_size; }
	size_type size() const { return m_size; }
	size_type pageSize() const { return m_buffer.size(); }
public:
	value_type& front()
	{
		//	undone: データが無い場合対応
		return m_buffer.front()->front();
	}
	value_type& back()
	{
		//	undone: データが無い場合対応
		return m_buffer.back()->back();
	}
	void clear()
	{
		m_size = 0;
		m_curPage = 0;
		m_curFront = 0;
		//for(auto& page: m_buffer) page.clear();
		for (int i = 0; i != m_buffer.size(); ++i)
			m_buffer[i]->clear();
	}
	void push_back(value_type v)
	{
		++m_size;
		if( m_buffer.back()->size() == PAGE_MAX_SZ ) {
			m_buffer.push_back(new gap_buffer<Type>());
		}
		m_buffer.back()->push_back(v);
		//return true;
	}
	void pop_back()
	{
		if( !isEmpty() ) {
			int ix = m_buffer.size();
			while( m_buffer[--ix]->isEmpty() ) { }		//	最後の空でないページを探す
			m_buffer[ix]->pop_back();
			--m_size;
		}
	}
	bool insert(pos_t ix, value_type v)
	{
		return true;
	}
	value_type& at(pos_t ix)
	{
		if( ix >= m_curFront && ix < m_curFront + m_buffer[m_curPage]->size() )
			return m_buffer[m_curPage]->at(ix - m_curFront);
		assert(0);
		return back();	//	暫定コード
	}
	value_type& operator[](pos_t ix)
	{
		if( ix >= m_curFront && ix < m_curFront + m_buffer[m_curPage]->size() )
			return m_buffer[m_curPage]->at(ix - m_curFront);
		assert(0);
		return back();	//	暫定コード
	}
private:
	gap_buffer<gap_buffer<Type>*>	m_buffer;
	size_type	m_size;				//	データトータルサイズ
	int			m_curPage;		//	現ページ、ランダムアクセス時は現ページから優先して探索
	ssize_t		m_curFront;		//	現ページ先頭インデックス
	
};
