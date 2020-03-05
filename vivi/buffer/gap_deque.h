//----------------------------------------------------------------------
//
//			File:			"gap_deque.h"
//			Created:		16-Sep-2014
//			Author:			津田伸秀
//			Description:	末尾にもギャップを持つギャップバッファ
//
//----------------------------------------------------------------------

#pragma once

//#ifndef		_HEADER_GAP_DEQUE_H
//#define		_HEADER_GAP_DEQUE_H

//#include <QtCore>
#include <memory>
#include <vector>

//#ifndef		Q_OS_WIN32
//#define		Q_OS_WIN32		1
//#endif

#define		OS_WIN32		1

#ifdef OS_WIN32
#include <windows.h>
#endif

//typedef unsigned int uint;
//typedef size_t index_t;

//	インデックス、サイズの型は符号付き int 

/**

	途中だけでなく、末尾にもギャップを持つギャップバッファ、gap_deque = deque + gap_buffer

	基本的に gapSize と tailGapSize はバランスしているものとする
	（バッファが空になった場合、TailGap を後ろ半分に設定する）
	gap または tailGap が足りなくなった時、gapSize + tailGapSize >= dataSize であれば、
	メモリリアロケートを行わず、データの移動のみを行い、gapSize と tailGap をバランスさせる
	そうでなければ、dataSize の倍のサイズのメモリをアロケートし、データを先頭～中央に配置する
	（gapSize と tailGapSize はバランス）

*/

template<typename Type>
class gap_deque
{
	enum {
		DEFAULT_BUFFER_SIZE = 8,
		DEFAULT_TAILGAP_SIZE = 8,
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
	gap_deque()
		: m_data(0), m_size(0), m_gapIndex(0), m_gapSize(0), m_tailGapSize(0)
		{}
	gap_deque(ssize_t sz, value_type v = 0)
		: m_size(sz), m_gapIndex(0), m_gapSize(0), m_tailGapSize(0)
		{
#ifdef	OS_WIN32
			m_data = (pointer)VirtualAlloc(0, sz*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
			m_data = new value_type[sz];
#endif
			for (ssize_t i = 0; i < sz; ++i) {
				m_data[i] = v;
			}
		}
	gap_deque(const gap_deque<value_type> &x)
	{
		if( x.m_data == 0 ) {
			m_data = 0;
			m_size = 0;
			m_gapIndex = 0;
			m_gapSize = 0;
			m_tailGapSize = 0;
		} else {
			size_t tcp = x.totalCapacity();
#ifdef	OS_WIN32
			m_data = (pointer)VirtualAlloc(0, tcp*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
			m_data = new value_type[tcp];
#endif
			m_size = x.m_size;
			m_gapIndex = x.m_gapIndex;
			m_gapSize = x.m_gapSize;
			m_tailGapSize = x.m_tailGapSize;
			//	undone: gap が大きすぎる場合は非効率なので、ギャップ前後で処理を分けた方がよい
			memcpy((void *)m_data, (void *)x.m_data, (tcp - m_tailGapSize)*sizeof(value_type));
		}
	}
	~gap_deque() {
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
	size_type capacity() const { return m_size + min(m_gapSize, m_tailGapSize); }
	size_type totalCapacity() const { return m_size + m_gapSize + m_tailGapSize; }
	pos_t gapIndex() const { return m_gapIndex; }
	void setGapIndex(pos_t gapIndex) const { move_gap(gapIndex); }
	ssize_t gapSize() const { return m_gapSize; }
	ssize_t tailGapSize() const { return m_tailGapSize; }
	ssize_t totalGapSize() const { return m_gapSize + m_tailGapSize; }
	value_type operator[](pos_t ix) const { return at(ix); }
	value_type &operator[](pos_t ix) { return ref(ix); }
	value_type at(pos_t ix) const
	{
		if( ix < 0 || ix >= size() )		//	範囲外の場合
			return value_type();
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
	value_type front() const
	{
		return at(0);
	}
	value_type back() const
	{
		if( isEmpty() )
			return value_type();
		return at(m_size - 1);
	}
	const value_type *raw_data(pos_t ix) const
	{
		if( ix >= m_gapIndex )
			return m_data + ix + m_gapSize;
		else
			return m_data + ix;
	}
	size_type get_data(pos_t ix, pointer buf, int bufSize) const
	{
		//Q_ASSERT( bufSize > 0 );
		if( bufSize <= 0 ) return 0;
		if( isEmpty() ) return 0;
		if( bufSize > size() - ix ) bufSize = size() - ix;
		ssize_t copiedSize = 0;
		if( ix < m_gapIndex ) {
			if( ix + bufSize <= m_gapIndex ) {	//	データが前半部に全てある場合
				//	undone: gap_deque では起こりえないはず
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
	bool operator==(const gap_deque<value_type> &x) const
	{
		if( size() != x.size() ) return false;
		for(pos_t i = 0; i != size(); ++i) {
			if( at(i) != x.at(i) ) return false;
		}
		return true;
	}
	bool operator!=(const gap_deque<value_type> &x) const
	{
		return !operator==(x);
	}

public:
	void clear()
	{
		if( !size() ) return;
		//m_gapSize = size();
		const auto tg = totalGapSize();
		m_gapSize = tg / 2;
		m_tailGapSize = tg - m_gapSize;			//	TailGap は後ろ半分とする
		m_gapIndex = m_size = 0;
	}
	value_type &ref(pos_t ix)
	{
		//if( ix < 0 || ix >= size() )
		//	return ix = 0;
		if( ix >= m_gapIndex ) ix += m_gapSize;
		return m_data[ix];
	}
	void resize(size_type sz)		////undone
	{
		if (!reserve(sz))
			return;
		size_type s = size();
		if( sz <= s ) {
#if	1
			move_gap(0);		//	ギャップを先頭に移動
			m_size = sz;
			m_tailGapSize = sz + m_gapSize;
#else
			move_gap(size());		//	ギャップを末尾に移動
			m_gapIndex -= s - sz; 
			m_gapSize += s - sz;
			m_size = sz;
#endif
		} else {
			//	undone: 高速化
			while( ++s <= sz )
				push_back(Type());
		}
	}
	bool reserveTail(ssize_t sz)
	{
		if( m_size + m_tailGapSize >= sz ) return true;
		auto total = m_size + m_gapSize + m_tailGapSize;
#if	1
		if( total >= 1024 && m_gapSize >= (sz - m_size) * 2 - m_tailGapSize ) {
			//	最初のギャップを減らしてやりくりできる場合はやりくりする
			//
			//	□□■■■■■□■
			//		↓ reserveTail(4) の場合
			//	□□■■■□■■■
			auto first = m_gapIndex + m_gapSize;
			auto last = m_size + m_gapSize;
			auto t = m_gapSize + m_tailGapSize;
			m_gapSize = t / 2;
			m_tailGapSize = t - m_gapSize;
			auto dst = m_gapIndex + m_gapSize;
			while( first != last )
				m_data[dst++] = m_data[first++];
			return true;
		}
#endif
		//	m_size + m_tailGapSize >= sz が成立するまで、トータルバッファ・サイズを２倍する
		total *= 2;
		if( total == 0 )
			total = DEFAULT_TAILGAP_SIZE;
		while( total - m_gapSize < sz )
			total *= 2;
		//sz = max(sz, DEFAULT_TAILGAP_SIZE);
		//ssize_t cp = capacity();	//	tailGap を含まないサイズ
		//ssize_t total = cp + (m_tailGapSize = sz - m_size);
#ifdef	OS_WIN32
		pointer data = (pointer)VirtualAlloc(0, total*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
		pointer data = new value_type[total];
#endif
		if( !data )
			return false;
		if( !m_data ) {		//	バッファが空だった場合
			m_data = data;
			m_size = m_gapIndex = 0;
			//m_gapSize = total / 2;
			m_tailGapSize = total - m_size - (m_gapSize = 0);
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
		//m_gapSize += cp - cp0;
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
		m_tailGapSize = total - m_size - m_gapSize;
		return true;
	}
	bool reserveFirst(ssize_t sz)
	{
		if( m_size + m_gapSize >= sz ) return true;
		auto total = m_size + m_gapSize + m_tailGapSize;
#if	1
		if( total >= 1024 && m_tailGapSize >= (sz - m_size) * 2 - m_gapSize ) {
			//	末尾ギャップを減らしてやりくりできる場合はやりくりする
			//
			//	□□■□■■■■■
			//		↓ reserveFirst(4) の場合
			//	□□■■■□■■■
			auto first = m_gapIndex + m_gapSize;
			auto last = m_size + m_gapSize;
			auto t = m_gapSize + m_tailGapSize;
			m_gapSize = t / 2;
			m_tailGapSize = t - m_gapSize;
			auto dst = m_size + m_gapSize;
			while( first != last )
				m_data[--dst] = m_data[--last];
			return true;
		}
#endif
		//	m_size + m_gapSize >= sz が成立するまで、トータルバッファ・サイズを２倍する
		total *= 2;
		if( total == 0 )
			total = DEFAULT_TAILGAP_SIZE;
		while( total - m_tailGapSize < sz )
			total *= 2;
		//sz = max(sz, DEFAULT_TAILGAP_SIZE);
		//ssize_t cp = capacity();	//	tailGap を含まないサイズ
		//ssize_t total = cp + (m_tailGapSize = sz - m_size);
#ifdef	OS_WIN32
		pointer data = (pointer)VirtualAlloc(0, total*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
		pointer data = new value_type[total];
#endif
		if( !data )
			return false;
		if( !m_data ) {		//	バッファが空だった場合
			m_data = data;
			m_size = m_gapIndex = 0;
			m_gapSize = total / 2;
			m_tailGapSize = total - m_size - m_gapSize;
			return true;
		}
		//	ギャップ位置を広げつつデータコピー
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
		//m_gapSize += cp - cp0;
		m_gapSize = total - m_size - m_tailGapSize;
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
	//	reserve() 後に push_back() or それ以外が呼ばれるか分からないので、
	//	tailGap も sz - size() まで拡張する
	bool reserve(size_type sz)
	{
		ssize_t cp = capacity();
		if( sz <= cp ) return true;
		//	undone: gap + TailGap >= sz の場合は、tail
		const ssize_t cp0 = cp;		//	以前のキャパシティ
		if( !cp ) cp = DEFAULT_BUFFER_SIZE;
		while( sz > cp )
			cp += cp;
		//m_tailGapSize = cp - m_size;	//	tailGap も gapSize と同じだけ拡張
		ssize_t total = cp + cp - m_size;
		if( total <= totalCapacity() ) {
			//	undone: TailGap を小さくしてやりくり出来る場合
		}
#ifdef	OS_WIN32
		pointer data = (pointer)VirtualAlloc(0, total*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
		pointer data = new value_type[total];
#endif
		if( !data )
			return false;
		if( !m_data ) {		//	バッファが空だった場合
			m_data = data;
			m_size = m_gapIndex = 0;
			m_gapSize = total / 2;
			m_tailGapSize = total - m_gapSize;
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
		m_tailGapSize = total - (m_size + m_gapSize);
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
	bool push_back(value_type v)
	{
#if	1
		if( !reserveTail(size() + 1) ) return false;
#else
		if( !m_tailGapSize ) {
			if( m_gapSize <= 1 || m_gapSize < m_size ) {
				ssize_t total = m_size * 2;
#ifdef	OS_WIN32
				pointer data = (pointer)VirtualAlloc(0, total*sizeof(Type), MEM_COMMIT, PAGE_READWRITE);
#else
				pointer data = new value_type[total];
#endif
				if( !data )
					return false;
				auto gsz0 = m_gapSize;
				m_gapSize = total / 2;
				m_tailGapSize = total - m_gapSize;
				if( m_size != 0 ) {
					//	データコピー
				}
			} else {		//	totalGapSize >= m_size の場合
				make_gap_balanced();
			}
		}
#endif
		m_data[m_size+m_gapSize] = v;
		--m_tailGapSize;
		++m_size;
		return true;
	}
	bool push_front(value_type v)
	{
#if	0
		return insert(0, v);
#else
		if( !reserveFirst(size() + 1) ) return false;
		move_gap(0);
		m_data[/*m_gapIndex +*/ --m_gapSize] = v;
		++m_size;
		return true;
#endif
	}
	bool insert(pos_t ix, value_type v)
	{
		if( ix == m_size ) {
			push_back(v);
			return;
		}
		if( !reserveFirst(size() + 1) ) return false;
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
		if( ix >= m_size ) {			//	末尾挿入
			if( !reserveTail(size() + sz) ) return false;
			memcpy((void *)(m_data + m_size + m_gapSize), (void *)first, sz*sizeof(value_type));
			m_tailGapSize -= sz;
			m_size += sz;
		} else {
			ssize_t s = m_size - ix;	//	移動文字数
			if( m_gapIndex < ix && s <= 4096 && s <= m_size / 1024 ) {
				//	末尾付近に挿入する場合
				if( !reserveTail(size() + sz) ) return false;
				const value_type *last2 = m_data + m_size + m_gapSize;
				const value_type *first2 = last2 - s;
				value_type *dst = (value_type *)last2 + sz;
				while( first2 != last2 )
					*--dst = *--last2;
				memcpy((void *)first2, (void *)first, sz*sizeof(value_type));
				m_tailGapSize -= sz;
				m_size += sz;
			} else {
				if( !reserve(size() + sz) ) return false;
				move_gap(ix);
				memcpy((void *)(m_data + ix), (void *)first, sz*sizeof(value_type));
				m_gapIndex += sz;
				m_gapSize -= sz;
				m_size += sz;
			}
		}
		return true;
	}
	void pop_back()
	{
		if( isEmpty() ) return;
		++m_tailGapSize;
		if( --m_size == 0 ) {
			m_gapIndex = 0;
			auto total = m_gapSize + m_tailGapSize;
			m_gapSize = total / 2;			//	TailGap を後ろ半分に設定
			m_tailGapSize = total - m_gapSize;
		} else if( m_gapIndex == m_size ) {		//	ギャップが末尾にある場合
			m_tailGapSize += m_gapSize;
			m_gapIndex = 0;
			m_gapSize = 0;
		}
	}
	void pop_front()
	{
		if( isEmpty() ) return;
		erase(0);
	}
	void erase(pos_t ix)
	{
		if( ix < 0 || ix >= size() ) return;
		if( ix == size() - 1 ) {		//	末尾削除
			pop_back();
			return;
		}
		move_gap(ix);
		++m_gapSize;
		if( --m_size == 0 ) {
			m_gapIndex = 0;
			auto total = m_gapSize + m_tailGapSize;
			m_gapSize = total / 2;			//	TailGap を後ろ半分に設定
			m_tailGapSize = total - m_gapSize;
		} else if( m_gapIndex == m_size ) {		//	ギャップ以降にデータが無い場合
			m_tailGapSize += m_gapSize;
			m_gapIndex = 0;
			m_gapSize = 0;
		}
	}
	void erase(pos_t first, size_t sz)
	{
		eraseFL(first, first + sz);
	}
	void eraseFL(pos_t first, pos_t last)
	{
		if( first < 0 ) first = 0;
		if( last > size() ) last = size();
		if( first >= last ) return;
		const size_t sz = last - first;		//	削除文字数
		if( last == size() ) {		//	末尾まで削除の場合
			m_tailGapSize += sz;
			m_size -= sz;
			return;
		}
		ssize_t ms = m_size - last;	//	移動文字数
		if( m_gapIndex < first && ms <= 4096 && ms <= m_size / 1024 ) {
			//	末尾付近を削除する場合
			value_type *dst = m_data + first + m_gapSize;
			const value_type *src = dst + sz;
			const value_type *srcend = src + ms;
			while( src != srcend )
				*dst++ = *src++;
			m_tailGapSize += sz;
			m_size -= sz;
			return;
		}
		move_gap(first);
		m_gapSize += sz;
		m_size -= sz;
		if( m_size == 0 ||
			m_gapIndex == m_size )		//	ギャップ以降にデータが無い場合
		{
			m_tailGapSize += m_gapSize;
			m_gapIndex = 0;
			m_gapSize = 0;
		}
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
	void swap(gap_deque<value_type> &x)
	{
		pointer p = m_data;	m_data = x.m_data;			x.m_data = p;
		int s = m_gapIndex;	m_gapIndex = x.m_gapIndex;		x.m_gapIndex = s;
		s = m_gapSize;		m_gapSize = x.m_gapSize;	x.m_gapSize = s;
		s = m_size;			m_size = x.m_size;			x.m_size = s;
		std::swap(m_tailGapSize, x.m_tailGapSize);
		std::swap(m_tailGapCapacity, x.m_tailGapCapacity);
	}
	value_type *data()
	{
		move_gap(size());
		return m_data;
	}
private:
	void make_gap_balanced()
	{
		//const auto totalGapSize = m_gapSize + m_tailGapSize;
		const auto tg = totalGapSize();
		if( isEmpty() ) {
			m_gapSize = tg / 2;
			m_tailGapSize = tg - m_gapSize;
		} else {
			const auto gsz0 = m_gapSize;
			const auto tgsz0 = m_tailGapSize;
			m_gapSize = tg / 2;
			m_tailGapSize = tg - m_gapSize;
			if( gsz0 < tgsz0 ) {
				//	□□□□■■□□■■■■■■
				//				↓
				//	□□□□■■■■□□■■■■
				pos_t first = m_gapIndex + gsz0;
				pos_t last = m_size + gsz0;
				pos_t dst = m_size + m_gapSize;
				while( first != last )
					m_data[--dst] = m_data[--last];
			} else if( gsz0 > tgsz0 ) {
				//	□□□□■■■■■■□□■■
				//				↓
				//	□□□□■■■■□□■■■■
				pos_t first = m_gapIndex + gsz0;
				pos_t last = m_size + gsz0;
				pos_t dst = m_gapIndex + m_gapSize;
				while( first != last )
					m_data[dst++] = m_data[first++];
			}
		}
	}
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
		if( ix >= size() ) return;		//	末尾挿入・削除の場合、ギャップを移動しない
		if( ix < 0 ) ix = 0;
		//else if( ix > size() ) ix = size();
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
	size_type	m_tailGapSize;		//	末尾ギャップサイズ
	//size_type	m_tailGapCapacity;		//	末尾ギャップキャパシティ for 次にアロックする時のため
	size_type	m_size;		//	データトータルサイズ
    //  ↓data  ↓gapIndex
    //  ┌───┬────┬────┬────┐
    //  │      │gapSize │        │tailGap │
    //  └───┴────┴────┴────┘
    //   ←─── capacity ────→
	//		※ capacity はリアロケートが起こらないことを保証するサイズだと
	//			解釈できるので、tailGap の分は含まないものとする

	//		※ バッファが空でない限り、First ギャップ以降に必ずデータが存在するものとする
	//			データが無くなった場合は、ギャップを先頭に移動し、gapSize = 0 とする（データの移動は行われない）

	friend class TestClass;
};

//#endif		//_HEADER_GAP_DEQUE_H
