//----------------------------------------------------------------------
//
//			File:			"undoMgr.cpp"
//			Created:		16-Jun-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#include "undoMgr.h"
#include "Buffer.h"
#include <QDebug>

#ifdef min
#undef	min
#endif
#if	0
namespace My {
	template<typename T>
	T min(T a, T b) { return (a>b ? a : b); }
};
#endif
inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}

//template<typename T>
UndoMgr::UndoMgr(Buffer *buffer)
	: m_buffer(buffer)
	, m_blockLevel(0)
#if !USE_BOOST_POOL
	, m_actInsPoolSize(0)
	, m_actDelPoolSize(0)
#endif
{
	init();
}
//template<typename T>
UndoMgr::~UndoMgr()
{
}
//template<typename T>
void UndoMgr::init()
{
	m_cur = 0;
	m_savePointCur = 0;
	m_lastDelTextSize = 0;
	m_lastInsTextSize = 0;
	//m_blockOpened = false;
	m_actionPushed = false;
	m_stack.clear();
	m_insText.clear();
	m_delText.clear();
}
void UndoMgr::openBlock()
{
	qDebug() << "openUndoBlock()";
#if	1
	//if( !m_blockLevel ) return;
	if( ++m_blockLevel != 1 ) return;
#else
	if( m_blockOpened ) return;
	m_blockOpened = true;
#endif
	m_actionPushed = false;
}
void UndoMgr::closeAllBlock()
{
	qDebug() << "closeAllUndoBlock()";
	if( !m_blockLevel ) return;
	m_blockLevel = 0;
	if( !m_actionPushed || m_stack.empty() )
		return;
	int last = m_stack.size() - 1;
	m_stack[last]->m_flags ^= UndoAction::FLAG_BLOCK;
}
void UndoMgr::closeBlock()
{
	qDebug() << "closeUndoBlock()";
#if	1
	if( !m_blockLevel || --m_blockLevel != 0 )
		return;
#else
	if( !m_blockOpened ) return;
	m_blockOpened = false;
#endif
	if( !m_actionPushed || m_stack.empty() )
		return;
	int last = m_stack.size() - 1;
	m_stack[last]->m_flags ^= UndoAction::FLAG_BLOCK;
}
//	現状態を非モディファイ状態とする（保存時にコールされる）
void UndoMgr::onSaved()
{
	m_savePointCur = m_cur;
	for(int i = m_cur; --i >= 0; ) {
		switch( m_stack[i]->m_type ) {
		case UndoAction::ACT_INSERT: {
			auto p = (UndoActionInsert *)m_stack[i];
			p->m_saved = false;
			break;
		}
		case UndoAction::ACT_DELETE: {
			auto p = (UndoActionDelete *)m_stack[i];
			const int bits = sizeof(wchar_t) * 8 / 2;
			int msz = (p->m_lc + bits - 1) / bits;
			int ix = p->m_ix + p->m_size;
			for(int i = 0; i < msz; ++i) {
				m_delText[ix] &= 0x5555;	//	reset Saved フラグ
			}
			break;
		}
#if 0
		case UndoAction::ACT_REPLACE_ALL: {
			auto p = (UndoActionReplaceAll *)m_stack[i];
			break;
		}
#endif
		}
	}
}
bool UndoMgr::push_back(UndoAction *ptr)
{
	try {
		if( m_stack.size() > (size_t)m_cur ) {
			int ix = m_insText.size();
			for(int i = m_stack.size(); --i >= m_cur; ) {
				switch( m_stack[i]->m_type ) {
				case UndoAction::ACT_INSERT: {
					auto p = (UndoActionInsert *)m_stack[i];
					ix = std::min(ix, p->m_ix);
#if USE_BOOST_POOL
					m_actInsPool.free(p);
#else
#endif
					break;
				}
				case UndoAction::ACT_DELETE: {
					auto p = (UndoActionDelete *)m_stack[i];
#if USE_BOOST_POOL
					m_actDelPool.free(p);
#else
#endif
					break;
				}
#if 0
				case UndoAction::ACT_REPLACE: {
					auto p = (UndoActionReplace *)m_stack[i];
					m_actRepPool.free(p);
					break;
				}
#endif
#if 0
				case UndoAction::ACT_REPLACE_ALL: {
					auto p = (UndoActionReplaceAll *)m_stack[i];
					delete p;
					break;
				}
#endif
				}
			}
			m_insText.resize(ix);
			m_stack.resize(m_cur);
			//	undone 不要な ins, del バッファを削除
		}
		m_stack.push_back(ptr);
	} catch(...) {
		return false;
	}
	if( isBlockOpened() && !m_actionPushed ) {
		int last = m_stack.size() - 1;
		m_stack[last]->m_flags |= UndoAction::FLAG_BLOCK;
		m_actionPushed = true;
	}
	if( m_savePointCur > m_cur )
		m_savePointCur = -1;
	++m_cur;
	return true;
}
void UndoMgr::prohibitMergeUndo()			//	挿入マージ禁止
{
	if( m_cur != 0 ) {
		UndoActionInsert *ptr = (UndoActionInsert *)m_stack[m_cur - 1];
		if( ptr->m_type == UndoAction::ACT_INSERT )
			ptr->m_prohibitMerge = true;
	}
}
bool UndoMgr::push_back_insText(int pos, int sz, int ln)
{
#if	1
	//	undone: 改行以外の文字を連続した場所に挿入した場合は、UndoActionInsert を
	if( m_cur != 0 && !isNewLine(m_buffer->charAt(pos)) ) {
		UndoActionInsert *ptr = (UndoActionInsert *)m_stack[m_cur - 1];
		if( ptr->m_type == UndoAction::ACT_INSERT && !ptr->m_prohibitMerge
			&& ptr->m_pos + ptr->m_size == pos )
		{
			ptr->m_size += sz;
			m_stack.resize(m_cur);
			return true;
		}
	}
#endif
#if USE_BOOST_POOL
	UndoActionInsert *act = m_actInsPool.construct();
#else
	UndoActionInsert *act = newActInsert();
#endif
	if( act == 0 )
		return false;	//	メモリ不足
	act->m_pos = pos;
	act->m_size = sz;
	act->m_ln = ln;
	//int ln = act->m_ln = m_buffer->positionToLine(pos);
	int ln2 = m_buffer->positionToLine(pos + sz);
	act->m_modified = (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0;
	act->m_saved = (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0;
	act->m_lc = ln2 - ln + 1;
	for(int i = 0; i < act->m_lc; ++i, ++ln) {
		m_buffer->setLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
		m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_SAVED);
	}
	push_back(act);
	return true;
}
bool UndoMgr::push_back_delText(int pos, int sz, bool BS, int ln)
{
	const int ix = m_delText.size();
	int ln2 = m_buffer->positionToLine(pos + sz);
	const int bits = sizeof(wchar_t) * 8 / 2;
	int msz = (ln2 - ln + 1 + bits - 1) / bits;
	m_delText.resize(ix + sz + msz);		//	削除文字用バッファ容量確保
	m_buffer->getText(pos, &m_delText[ix], sz);
#if USE_BOOST_POOL
	UndoActionDelete *act = m_actDelPool.construct();
#else
	UndoActionDelete *act = newActDelete();
#endif
	if( act == 0 )
		return false;
	if( BS )
		act->m_flags |= UndoAction::FLAG_BS;
	act->m_pos = pos;
	act->m_size = sz;
	act->m_ix = ix;
	act->m_ln = ln;
	act->m_lc = ln2 - ln + 1;		//	削除行情報行数
	int mix = ix + sz;
	wchar_t mask = 1;
	for(int i = 0; ln <= ln2; ++i, ++ln) {
		if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0 )
			m_delText[mix] |= mask;
		else
			m_delText[mix] &= ~mask;
		mask <<= 1;
		if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0 )
			m_delText[mix] |= mask;
		else
			m_delText[mix] &= ~mask;
		if( (mask <<= 1) == 0 ) {
			mask = 1;
			++mix;
		}
	}
	push_back(act);
	return true;
}
UndoActionReplace *UndoMgr::push_back_repText(int pos, int dsz, int isz, int ln)
{
	const int ix = m_delText.size();
	int ln2 = m_buffer->positionToLine(pos + dsz);
	const int bits = sizeof(wchar_t) * 8 / 2;
	int msz = (ln2 - ln + 1 + bits - 1) / bits;
	m_delText.resize(ix + dsz + msz);		//	削除文字用バッファ容量確保
	m_buffer->getText(pos, &m_delText[ix], dsz);
#if USE_BOOST_POOL
	UndoActionReplace *act = m_actRepPool.construct();
#else
	UndoActionReplace *act = newActReplace();
#endif
	if( act == 0 )
		return false;
	act->m_pos = pos;
	act->m_ln = ln;
	act->m_sizeDel = dsz;
	act->m_ixDel = ix;
	act->m_lcDel = ln2 - ln + 1;		//	削除行情報行数
	act->m_sizeIns = isz;
	int mix = ix + dsz;
	wchar_t mask = 1;
	for(int i = 0; ln <= ln2; ++i, ++ln) {
		if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0 )
			m_delText[mix] |= mask;
		else
			m_delText[mix] &= ~mask;
		mask <<= 1;
		if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0 )
			m_delText[mix] |= mask;
		else
			m_delText[mix] &= ~mask;
		if( (mask <<= 1) == 0 ) {
			mask = 1;
			++mix;
		}
	}
	push_back(act);
	return act;
}
#if	0
void UndoMgr::setLcIns(int)
{
	if( !m_cur ) return;
	UndoAction *ptr = m_stack[m_cur - 1];
	if( 
}
#endif
int UndoMgr::undo()
{
	int pos = 0;
	ushort flag = 0;
	do {
		if( !m_cur ) return false;
		UndoAction *ptr = m_stack[--m_cur];
		flag ^= (ptr->m_flags & UndoAction::FLAG_BLOCK);
		switch( ptr->m_type ) {
		case UndoAction::ACT_INSERT: {
			auto p = (UndoActionInsert *)ptr;
			pos = p->m_pos;
			const int ix = p->m_ix = m_insText.size();
			const int bits = sizeof(wchar_t) * 8 / 2;
			int msz = (p->m_lc + bits - 1) / bits;
			m_insText.resize(ix + p->m_size + msz);
			m_buffer->getText(p->m_pos, &m_insText[ix], p->m_size);
			int mix = ix + p->m_size;
			wchar_t mask = 1;
			int ln = p->m_ln;
			for(int i = 0; i < p->m_lc; ++i, ++ln) {
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0 )
					m_insText[mix] |= mask;
				else
					m_insText[mix] &= ~mask;
				mask <<= 1;
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0 )
					m_insText[mix] |= mask;
				else
					m_insText[mix] &= ~mask;
				if( (mask <<= 1) == 0 ) {
					mask = 1;
					++mix;
				}
			}
			m_buffer->basicDeleteText(p->m_pos, p->m_size);
			if( !p->m_modified )
				m_buffer->resetLineFlag(p->m_ln, Buffer::LINEFLAG_MODIFIED);
			if( p->m_saved )
				m_buffer->setLineFlag(p->m_ln, Buffer::LINEFLAG_SAVED);
			else
				m_buffer->resetLineFlag(p->m_ln, Buffer::LINEFLAG_SAVED);
			break;
		}
		case UndoAction::ACT_DELETE: {
			auto p = (UndoActionDelete *)ptr;
			pos = (p->m_flags & UndoAction::FLAG_BS) ? p->m_pos + p->m_size : p->m_pos;
			m_buffer->basicInsertText(p->m_pos, &m_delText[p->m_ix], p->m_size);
			int ln = p->m_ln;
			wchar_t mask = 1;
			int ix = p->m_ix + p->m_size;
			for(int i = 0; i < p->m_lc; ++i, ++ln) {
				if( (m_delText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				mask <<= 1;
				if( (m_delText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_SAVED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_SAVED);
				if( !(mask <<= 1) ) {
					mask = 1;
					++ix;
				}
			}
			m_delText.resize(p->m_ix);
			break;
		}
		case UndoAction::ACT_REPLACE: {
			auto p = (UndoActionReplace *)ptr;
			pos = p->m_pos;
			int ix = p->m_ixIns = m_insText.size();
			const int bits = sizeof(wchar_t) * 8 / 2;
			int msz = (p->m_lcIns + bits - 1) / bits;
			m_insText.resize(ix + p->m_sizeIns + msz);
			m_buffer->getText(p->m_pos, &m_insText[ix], p->m_sizeIns);
			int mix = ix + p->m_sizeIns;
			wchar_t mask = 1;
			int ln = p->m_ln;
			for(int i = 0; i < p->m_lcIns; ++i, ++ln) {
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0 )
					m_insText[mix] |= mask;
				else
					m_insText[mix] &= ~mask;
				mask <<= 1;
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0 )
					m_insText[mix] |= mask;
				else
					m_insText[mix] &= ~mask;
				if( (mask <<= 1) == 0 ) {
					mask = 1;
					++mix;
				}
			}
			m_buffer->basicDeleteText(p->m_pos, p->m_sizeIns);
			//???
			//if( !p->m_modified )
			//	m_buffer->resetLineFlag(p->m_ln, Buffer::LINEFLAG_MODIFIED);
			//if( p->m_saved )
			//	m_buffer->setLineFlag(p->m_ln, Buffer::LINEFLAG_SAVED);
			//else
			//	m_buffer->resetLineFlag(p->m_ln, Buffer::LINEFLAG_SAVED);

			m_buffer->basicInsertText(p->m_pos, &m_delText[p->m_ixDel], p->m_sizeDel);
			ln = p->m_ln;
			mask = 1;
			ix = p->m_ixDel + p->m_sizeDel;
			for(int i = 0; i < p->m_lcDel; ++i, ++ln) {
				if( (m_delText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				mask <<= 1;
				if( (m_delText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_SAVED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_SAVED);
				if( !(mask <<= 1) ) {
					mask = 1;
					++ix;
				}
			}
			m_delText.resize(p->m_ixDel);
			break;
		}
#if 0
		case UndoAction::ACT_REPLACE_ALL: {
			auto p = (UndoActionReplaceAll *)ptr;
			const int *posList0 = p->m_posList;
			const int *posList = posList0 + p->m_nReplace;
			while( --posList >= posList0 ) {
				m_buffer->basicDeleteText(*posList, p->m_afterSz);
				m_buffer->basicInsertText(*posList, p->m_before, p->m_beforeSz);
				pos = *posList;
			}
			break;
		}
#endif
		}
	} while( flag );
	//m_buffer->setModified(modified);
	m_buffer->setModified(m_cur != m_savePointCur);
	return pos;
}
int UndoMgr::redo()
{
	int pos = 0;
	ushort flag = 0;
	//bool modified = false;
	do {
		if( (size_t)m_cur >= m_stack.size() ) return false;
		UndoAction *ptr = m_stack[m_cur++];
#if 0
		modified = (ptr->m_flags & UndoAction::FLAG_MODIFIED) != 0;
		if( modified && !m_buffer->isModified()
			|| !modified && m_buffer->isModified() )
		{
			ptr->m_flags ^= UndoAction::FLAG_MODIFIED;
		}
#endif
		flag ^= (ptr->m_flags & UndoAction::FLAG_BLOCK);
		switch( ptr->m_type ) {
		case UndoAction::ACT_INSERT: {
			auto p = (UndoActionInsert *)ptr;
			m_buffer->basicInsertText(p->m_pos, &m_insText[p->m_ix], p->m_size);
			int ln = p->m_ln;
			wchar_t mask = 1;
			int ix = p->m_ix + p->m_size;
			for(int i = 0; i < p->m_lc; ++i, ++ln) {
				if( (m_insText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				mask <<= 1;
				if( (m_insText[ix] & mask) != 0 )
					m_buffer->setLineFlag(ln, Buffer::LINEFLAG_SAVED);
				else
					m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_SAVED);
				if( !(mask <<= 1) ) {
					mask = 1;
					++ix;
				}
			}
			m_insText.resize(p->m_ix);
			pos = p->m_pos + p->m_size;
#if 0
			int ln = p->m_ln;
			for(int i = 0; i < p->m_lc; ++i, ++ln) {
				m_buffer->setLineFlag(ln, Buffer::LINEFLAG_MODIFIED);
				m_buffer->resetLineFlag(ln, Buffer::LINEFLAG_SAVED);
			}
#endif
			break;
		}
		case UndoAction::ACT_DELETE: {
			auto p = (UndoActionDelete *)ptr;
			const int ix = m_delText.size();
			const int bits = sizeof(wchar_t) * 8 / 2;
			int msz = (p->m_lc + bits - 1) / bits;
			m_delText.resize(ix + p->m_size + msz);
			m_buffer->getText(p->m_pos, &m_delText[ix], p->m_size);
			m_buffer->basicDeleteText(p->m_pos, p->m_size);
			int mix = ix + p->m_size;
			wchar_t mask = 1;
			int ln = p->m_ln;
			for(int i = 0; i < p->m_lc; ++i, ++ln) {
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_MODIFIED) != 0 )
					m_delText[mix] |= mask;
				else
					m_delText[mix] &= ~mask;
				mask <<= 1;
				if( (m_buffer->lineFlags(ln) & Buffer::LINEFLAG_SAVED) != 0 )
					m_delText[mix] |= mask;
				else
					m_delText[mix] &= ~mask;
				if( (mask <<= 1) == 0 ) {
					mask = 1;
					++mix;
				}
			}
			m_buffer->setLineFlag(p->m_ln, Buffer::LINEFLAG_MODIFIED);
			pos = p->m_pos;
			break;
		}
#if 0
		case UndoAction::ACT_REPLACE_ALL: {
			//std::vector<wchar_t> v;
			auto p = (UndoActionReplaceAll *)ptr;
			const int *posList = p->m_posList;
			const int *plend = posList + p->m_nReplace;
			while( posList < plend ) {
				m_buffer->basicDeleteText(*posList, p->m_beforeSz);
				m_buffer->basicInsertText(*posList, p->m_after, p->m_afterSz);
				pos = *posList;
				++posList;
				//m_buffer->getText(0, m_buffer->size(), v);
				//wchar_t *v0 = &v[0];
			}
			break;
		}
#endif
		}
	} while( flag );
	//m_buffer->setModified(modified);
	m_buffer->setModified(m_cur != m_savePointCur);
	return pos;
}
#if !USE_BOOST_POOL
UndoActionInsert *UndoMgr::newActInsert()
{
	int pix = m_actInsPoolSize / POOL_SIZE;
	int mod = m_actInsPoolSize % POOL_SIZE;
	while( pix >= m_actInsPool.size() )
		m_actInsPool.push_back((UndoActionInsert *)(new char[sizeof(UndoActionInsert)*POOL_SIZE]()));
	UndoActionInsert *ptr = m_actInsPool[pix] + mod;
	++m_actInsPoolSize;
	ptr->m_type = UndoAction::ACT_INSERT;
	ptr->m_flags = 0;
	return ptr;
}
UndoActionDelete *UndoMgr::newActDelete()
{
	int pix = m_actDelPoolSize / POOL_SIZE;
	int mod = m_actDelPoolSize % POOL_SIZE;
	while( pix >= m_actDelPool.size() )
		m_actDelPool.push_back((UndoActionDelete *)(new char[sizeof(UndoActionDelete)*POOL_SIZE]()));
	UndoActionDelete *ptr = m_actDelPool[pix] + mod;
	++m_actDelPoolSize;
	ptr->m_type = UndoAction::ACT_DELETE;
	ptr->m_flags = 0;
	return ptr;
}
UndoActionReplace *UndoMgr::newActReplace()
{
	int pix = m_actRepPoolSize / POOL_SIZE;
	int mod = m_actRepPoolSize % POOL_SIZE;
	while( pix >= m_actRepPool.size() )
		m_actRepPool.push_back((UndoActionReplace *)(new char[sizeof(UndoActionReplace)*POOL_SIZE]()));
	UndoActionReplace *ptr = m_actRepPool[pix] + mod;
	++m_actRepPoolSize;
	ptr->m_type = UndoAction::ACT_REPLACE;
	ptr->m_flags = 0;
	return ptr;
}
#endif
