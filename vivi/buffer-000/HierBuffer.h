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
		PAGE_MAX_SZ = 1024,
		//PAGE_MAX_SZ = 1024*1024,
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
	size_type capacity() const
	{
		size_type c = 0;
		for (int i = 0; i != m_buffer.size(); ++i) {
			c += m_buffer[i]->capacity();
		}
		return c;
	}
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
	void reserve(size_type sz)
	{
		if( sz <= PAGE_MAX_SZ ) {
			m_buffer[m_curPage]->reserve(sz);
		} else {
			size_type d = sz - capacity();		//	増加サイズ
			if( d > 0 ) {
				if( m_buffer[m_curPage]->capacity() < PAGE_MAX_SZ ) {
					d -= PAGE_MAX_SZ - m_buffer[m_curPage]->capacity();
					m_buffer[m_curPage]->reserve(PAGE_MAX_SZ);
				}
				if( d > 0 ) {		//	まだ増加させる必要がある場合
					for (int i = 0; i != m_buffer.size(); ++i) {
						if( i != m_curPage && m_buffer[i]->capacity() < PAGE_MAX_SZ ) {
							d -= PAGE_MAX_SZ - m_buffer[i]->capacity();
							m_buffer[i]->reserve(PAGE_MAX_SZ);
						}
					}
				}
				while( d > 0 ) {		//	まだ増加させる必要がある場合
					m_buffer.push_back(new gap_buffer<Type>());
					m_buffer.back()->reserve(PAGE_MAX_SZ);
					d -= PAGE_MAX_SZ;
				}
			}
			//assert(0);
		}
	}
	void push_back(value_type v)
	{
		if( m_buffer.back()->size() == PAGE_MAX_SZ ) {
			m_buffer.push_back(new gap_buffer<Type>());
		}
		m_buffer.back()->push_back(v);
		++m_size;
		m_curPage = m_buffer.size() - 1;
		m_curFront = m_size - m_buffer.back()->size();
		//return true;
	}
	void pop_back()
	{
		if( !isEmpty() ) {
			int ix = m_buffer.size();
			while( m_buffer[--ix]->isEmpty() ) { }		//	最後の空でないページを探す
			m_buffer[ix]->pop_back();
			--m_size;
			m_curPage = m_buffer.size() - 1;
			m_curFront = m_size - m_buffer.back()->size();
		}
	}
	bool insert(pos_t ix, value_type v)
	{
		reserve(size() + 1);
		if( ix < m_curFront ) {
			//	挿入位置が現ページ以前の場合
			if( ix >= m_curFront - m_buffer[m_curPage-1]->size() ) {		//	前ページ内
#if	1
				m_curFront -= m_buffer[--m_curPage]->size();
#else
				if( m_buffer[m_curPage-1]->size() < PAGE_MAX_SZ ) {
					m_curFront -= m_buffer[--m_curPage]->size();
				} else {
					//	undone: ページ分割
					assert(0);
				}
#endif
			} else {
				//	undone: 先頭から挿入すべきページを探す
				assert(0);
			}
		} else if( ix >= m_curFront + m_buffer[m_curPage]->size() && m_buffer[m_curPage]->size() >= PAGE_MAX_SZ ) {
			//	挿入位置が現ページ以降の場合
			assert(0);
		}
		if( m_buffer[m_curPage]->size() >= PAGE_MAX_SZ ) {
			//	undone: ページ分割
			//assert(0);
			size_type mvsz = m_buffer[m_curPage]->size() - (ix - m_curFront);
			m_buffer.insert(m_curPage+1, new gap_buffer<Type>());
			m_buffer[m_curPage+1]->resize(mvsz);
			m_buffer[m_curPage]->get_data(ix - m_curFront, &(*m_buffer[m_curPage+1])[0], mvsz);
			m_buffer[m_curPage]->erase(ix - m_curFront, mvsz);
		}
		m_buffer[m_curPage]->insert(ix - m_curFront, v);
		++m_size;
		return true;
	}
	void erase(pos_t ix)
	{
		if( ix < 0 || ix >= size() ) return;
		if( ix < m_curFront ) {
			if( ix >= m_curFront - m_buffer[m_curPage-1]->size() ) {		//	前ページ内
				m_curFront -= m_buffer[--m_curPage]->size();
			} else {
				//	undone: 先頭から挿入すべきページを探す
				assert(0);
			}
		} else if( ix >= m_curFront + m_buffer[m_curPage]->size() ) {
			//	挿入位置が現ページ以降の場合
			assert(0);
		}
		m_buffer[m_curPage]->erase(ix - m_curFront);
		--m_size;
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
		if( ix < m_curFront ) {
			if( ix >= m_curFront - m_buffer[m_curPage-1]->size() ) {		//	前ページ内
				m_curFront -= m_buffer[--m_curPage]->size();
			} else {
				m_curPage = 0;
				m_curFront = 0;
				while( ix >= m_curFront + m_buffer[m_curPage]->size() ) {
					m_curFront += m_buffer[m_curPage++]->size();
				}
			}
		} else if( ix >= m_curFront + m_buffer[m_curPage]->size() ) {
			if( ix < m_curFront + m_buffer[m_curPage]->size() + m_buffer[m_curPage+1]->size() ) {
				m_curFront += m_buffer[m_curPage++]->size();
			} else {
				m_curPage = m_buffer.size() - 1;
				m_curFront = m_size - m_buffer[m_curPage]->size();;
				while( ix < m_curFront ) {
					m_curFront -= m_buffer[--m_curPage]->size();;
				}
			}
		}
		return m_buffer[m_curPage]->operator[](ix - m_curFront);
	}
private:
	gap_buffer<gap_buffer<Type>*>	m_buffer;
	size_type	m_size;				//	データトータルサイズ
	int			m_curPage;		//	現ページ、ランダムアクセス時は現ページから優先して探索
	ssize_t		m_curFront;		//	現ページ先頭インデックス
	
friend void test_HierBuffer();
};
