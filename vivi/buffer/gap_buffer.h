//----------------------------------------------------------------------
//
//			File:			"gap_buffer.h"
//			Created:		10-Jun-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_GAP_BUFFER_H
#define		_HEADER_GAP_BUFFER_H

//#include <QtCore>
#include <memory>
#include <vector>
#include <iostream>
#include <exception>
#include <new>

//#ifndef		Q_OS_WIN32
//#define		Q_OS_WIN32		1
//#endif

#define		OS_WIN32		1

#ifdef OS_WIN32
#include <windows.h>
#endif

using namespace std;

typedef const char cchar;
//typedef unsigned int uint;
//typedef size_t index_t;

//	インデックス、サイズの型は符号付き int 

template<typename Type>
class gap_buffer
{
	enum {
		DEFAULT_BUFFER_SIZE = 8,
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
	//typedef _A Allocator;
public:
	gap_buffer() : m_data(0), m_gapIndex(0), m_gapSize(0), m_size(0) {}
	gap_buffer(const gap_buffer<value_type> &x)
	{
		if( x.m_data == 0 ) {
			m_data = 0;
			m_gapIndex = m_gapSize = m_size = 0;
		} else {
			size_t cp = x.capacity();
#ifdef	OS_WIN32
			m_data = (pointer)VirtualAlloc(0, cp*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
			m_data = new value_type[cp];
#endif
			memcpy((void *)m_data, (void *)x.m_data, cp*sizeof(value_type));
			m_gapIndex = x.m_gapIndex;
			m_gapSize = x.m_gapSize;
			m_size = x.m_size;
		}
	}
	~gap_buffer() {
#ifdef OS_WIN32
		VirtualFree((void *)m_data, 0, MEM_RELEASE);
#else
		delete m_data;
#endif
	}

public:
	bool isEmpty() const { return !m_size; }
	bool empty() const { return !m_size; }
	size_type size() const { return m_size; }
	size_type capacity() const { return m_size + m_gapSize; }
	pos_t gapIndex() const { return m_gapIndex; }
	void setGapIndex(pos_t gapIndex) const { move_gap(gapIndex); }
	ssize_t gapSize() const { return m_gapSize; }
	value_type operator[](pos_t ix) const {	//	operator[] は範囲チェックを行わない
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
		//return at(ix);
	}
	value_type &operator[](pos_t ix) {	//	operator[] は範囲チェックを行わない
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
		//return ref(ix);
	}
	value_type at(pos_t ix) const
	{
		if( ix < 0 || ix >= size() ) {
			//	done: out_of_range 例外スロー
			throw out_of_range("gap_buffer: out of range");
			//throw (cchar*)"gap_buffer: out of range";
			//return value_type();
		}
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
	}
	value_type& at(pos_t ix)
	{
		if( ix < 0 || ix >= size() ) {
			//	done: out_of_range 例外スロー
			throw out_of_range("gap_buffer: out of range");
			//throw (cchar*)"gap_buffer: out of range";
			//static value_type t;
			//return t;
		}
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
	}
	value_type &ref(pos_t ix) const
	{
		if( ix < 0 || ix >= size() )
			ix = 0;
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
	}
	const value_type *raw_data(pos_t ix) const
	{
		if( ix >= m_gapIndex )
			return m_data + ix + m_gapSize;
		else
			return m_data + ix;
	}
	size_type get_data(pos_t ix, pointer buf, int bufSize) const		//	[ix, ix+bufSize) を buf にコピー
	{
		//Q_ASSERT( bufSize > 0 );
		if( bufSize <= 0 ) return 0;
		if( isEmpty() ) return 0;
		if( bufSize > size() - ix ) bufSize = size() - ix;
		ssize_t copiedSize = 0;
		if( ix < m_gapIndex ) {
			if( ix + bufSize <= m_gapIndex ) {	//	データが前半部に全てある場合
				memcpy((void *)buf, (void *)(m_data + ix), bufSize*sizeof(value_type));
				return bufSize;
			}
			copiedSize = m_gapIndex - ix;
			memcpy((void *)buf, (void *)(m_data + ix), copiedSize*sizeof(value_type));
			buf += copiedSize;
			bufSize -= copiedSize;
			ix = m_gapIndex + m_gapSize;
		} else
			ix += m_gapSize;
		memcpy((void *)buf, (void *)(m_data + ix), bufSize*sizeof(value_type));
		return bufSize + copiedSize;
	}
	bool isMatch(pos_t ix, const value_type *pat, ssize_t sz) const
	{
		while( ix < size() ) {
			if( *pat != at(ix) ) break;
			if( --sz <= 0 ) return true;
			++pat;
			++ix;
		}
		return false;
	}
	bool isMatchLC(pos_t ix, const value_type *pat, size_t sz) const
	{
		while( ix < size() ) {
			if( *pat != tolower(at(ix)) ) break;
			if( --sz <= 0 ) return true;
			++pat;
			++ix;
		}
		return false;
	}
	pos_t indexOf(const value_type *pat, size_t sz, pos_t ix = 0, bool ic = false) const
	{
		if( !ic ) {
			while( ix + sz < size() ) {
				if( isMatch(ix, pat, sz) ) return ix;
				++ix;
			}
		} else {
			std::vector<value_type> lcpat(sz, 0);
			for(pos_t i = 0; i < sz; ++i) lcpat[i] = tolower(pat[i]);
			while( ix + sz < size() ) {
				if( isMatchLC(ix, &lcpat[0], sz) ) return ix;
				++ix;
			}
		}
		return -1;
	}
	bool operator==(const gap_buffer<value_type> &x) const
	{
		if( size() != x.size() ) return false;
		for(pos_t i = 0; i != size(); ++i) {
			if( at(i) != x.at(i) ) return false;
		}
		return true;
	}
	bool operator!=(const gap_buffer<value_type> &x) const
	{
		return !operator==(x);
	}

public:
	void clear()
	{
		if( !size() ) return;
		m_gapSize = size();
		m_gapIndex = m_size = 0;
	}
	value_type& front()
	{
		//	undone: データが無い場合対応
		return m_data[0];
	}
	value_type& back()
	{
		//	undone: データが無い場合対応
		return m_data[m_size - 1];
	}
	value_type &ref(pos_t ix)
	{
		//if( ix < 0 || ix >= size() )
		//	return ix = 0;
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
	}
	void resize(size_type sz)
	{
		if (!reserve(sz))
			return;
		size_type s = size();
		if( sz <= s ) {
			move_gap(size());		//	ギャップを末尾に移動
			m_gapIndex -= s - sz; 
			m_gapSize += s - sz;
			m_size = sz;
		} else {
			while( ++s <= sz )
				push_back(Type());
		}
	}
	bool reserve(size_type sz)
	{
		size_t cp = capacity();
		if( sz <= cp ) return true;
		const ssize_t cp0 = cp;		//	以前のキャパシティ
		if( !cp ) cp = DEFAULT_BUFFER_SIZE;
		while( sz > cp )
			cp += cp;
		//cout << "cp = " << cp << "\n";
#ifdef	OS_WIN32
		pointer data = (pointer)VirtualAlloc(0, cp*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
		pointer data = new value_type[cp];
#endif
		if( !data ) {
			throw bad_alloc();
			//throw (cchar*)"gap_buffer: length error";
			//return false;
		}
		if( !m_data ) {		//	バッファが空だった場合
			m_data = data;
			m_size = m_gapIndex = 0;
			m_gapSize = cp;
			return true;
		}
		//	ギャップ位置を維持してデータコピー
		//
		//       gap   gap+gapSize   size+gapSize
		//       ↓    ↓            ↓
		//	□□□■■■□□□□□□□
		//	
		if( m_gapIndex != 0 )
			memcpy((void *)data, (void *)m_data, m_gapIndex*sizeof(value_type));
		ssize_t n = m_size - m_gapIndex;	//	データ後半部分サイズ
		//Q_ASSERT( n >= 0 );
		const ssize_t oldGS = m_gapSize;
		m_gapSize += cp - cp0;
		if( n != 0 )
			memcpy((void *)(data + m_gapIndex + m_gapSize),
					(void *)(m_data + m_gapIndex + oldGS),
					n*sizeof(value_type));
#ifdef OS_WIN32
		VirtualFree((LPVOID)m_data, 0, MEM_RELEASE);
#else
		delete m_data;
#endif
		m_data = data;
		return true;
	}
	void setAt(pos_t ix, value_type v)
	{
		if( ix < 0 || ix >= size() ) return;
		if( ix >= m_gapIndex ) ix += m_gapSize;
		m_data[ix] = v;
	}
	void addAt(pos_t ix, int v)
	{
		if( ix < 0 || ix >= size() ) return;
		if( ix >= m_gapIndex ) ix += m_gapSize;
		m_data[ix] += v;
	}
	void push_back(value_type v)
	{
		if (!reserve(size() + 1)) {
			//	undone: 例外をスロー？
			return;
		}
		move_gap(size());
		m_data[m_gapIndex++] = v;
		--m_gapSize;
		++m_size;
		//return true;
	}
	bool insert(pos_t ix, value_type v)
	{
		if( !reserve(size() + 1) ) return false;
		move_gap(ix);
		m_data[m_gapIndex++] = v;
		--m_gapSize;
		++m_size;
		return true;
	}
	bool insert(pos_t ix, const_pointer first, const_pointer last)
	{
		return insert(ix, first, last - first);
	}
	bool insert(pos_t ix, const_pointer first, ssize_t sz)
	{
		if( !reserve(size() + sz) ) return false;
		move_gap(ix);
		memcpy((void *)(m_data + ix), (void *)first, sz*sizeof(value_type));
		m_gapIndex += sz;
		m_gapSize -= sz;
		m_size += sz;
		return true;
	}
	void pop_back()
	{
		if( !isEmpty() ) {
			move_gap(m_size);
			++m_gapSize;
			--m_size;
		}
	}
	void erase(pos_t ix)
	{
		if( ix < 0 || ix >= size() ) return;
		move_gap(ix);
		++m_gapSize;
		--m_size;
	}
	void erase(size_t first, size_t sz)
	{
		eraseFL(first, first + sz);
	}
	void eraseFL(size_t first, size_t last)
	{
		if( first < 0 ) first = 0;
		if( last > size() ) last = size();
		if( first >= last ) return;
		move_gap(first);
		const size_t sz = last - first;
		m_gapSize += sz;
		m_size -= sz;
	}
	template<typename T>
	void addToData(size_t first, size_t sz, T delta)
	{
		addToDataFL<T>(first, first + sz, delta);
	}
	template<typename T>
	void addToDataFL(size_t first, size_t last, T delta)
	{
		if( first < 0 ) first = 0;
		if( last > size() ) last = size();
		if( first >= last ) return;
		if( last > m_gapIndex ) {
			if( first < m_gapIndex ) {
				while( first != m_gapIndex )
					m_data[first++] += delta;
			}
			first += m_gapSize;
			last += m_gapSize;
		}
		while( first != last )
			m_data[first++] += delta;
	}
	void swap(gap_buffer<value_type> &x)
	{
		pointer p = m_data;	m_data = x.m_data;			x.m_data = p;
		int s = m_gapIndex;	m_gapIndex = x.m_gapIndex;		x.m_gapIndex = s;
		s = m_gapSize;		m_gapSize = x.m_gapSize;	x.m_gapSize = s;
		s = m_size;			m_size = x.m_size;			x.m_size = s;
	}
	value_type *data()
	{
		move_gap(size());
		return m_data;
	}
private:
	void move_data_backward(index_type dst, index_type src, ssize_t sz) const
	{
		pointer dp = m_data + dst + sz;
		const_pointer sp = m_data + src + sz;
		while( --sz >= 0 )
			*--dp = *--sp;
	}
	void move_data_forward(index_type dst, index_type src, ssize_t sz) const
	{
		pointer dp = m_data + dst;
		const_pointer sp = m_data + src;
		while( --sz >= 0 )
			*dp++ = *sp++;
	}
	void	move_gap(index_type ix) const
	{
		if( !m_data || ix == m_gapIndex ) return;
		if( ix < 0 ) ix = 0;
		else if( ix > size() ) ix = size();
		if( ix < m_gapIndex ) {
			//  ix < ギャップ位置 の場合:
			//
			//	□□□□□□■■■■■□□□
			//        ↑│           │
			//        ix└────┐ │
			//        ↓          ↓ │
			//	□□□■■■■■□□□□□□
			move_data_backward(	ix + m_gapSize,		//	dst
								ix,					//	src
								m_gapIndex - ix);
		} else {
			//       gap   ix  gap + gap
			//       ↓    ↓  ↓
			//	□□□■■■■■□□□□□□
			//       │           │ │
			//       │ ┌────┘ │
			//       │ ↓           │
			//	□□□□□□■■■■■□□□
			//             ↑
			//             ix
			move_data_forward(	m_gapIndex,				//	dst
								m_gapIndex + m_gapSize,	//	src
								ix - m_gapIndex);
		}
		m_gapIndex = ix;
	}

private:
	pointer		m_data;		//	バッファ先頭アドレス
	mutable size_type	m_gapIndex;	//	ギャップ位置
	mutable size_type	m_gapSize;	//	ギャップサイズ
	size_type	m_size;		//	データトータルサイズ
	//  ↓data  ↓gapIndex
	//  ┌───┬────┬────┐
	//	│      │gapSize │        │
	//  └───┴────┴────┘
	//   ←─── capacity ────→

	//pointer	m_first;		//   ↓first   ↓gapBegi n   ↓gapEnd        ↓m_last
	//pointer	m_gapBegin;		//┌────┬──────┬───────┐  
	//pointer	m_gapEnd;		//│  data  │    gap     │     data     │
	//pointer	m_last;			//└────┴──────┴───────┘

	//std::vector<Type>	m_findString;		//	検索文字列
	//int		m_skipTable[0x100];				//	検索時スキップテーブル for 下位バイト

	friend class TestClass;
};

#endif		//_HEADER_GAP_BUFFER_H
