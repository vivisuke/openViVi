//----------------------------------------------------------------------
//
//			File:			"LineMgr.h"
//			Created:		11-Jun-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_LINEMGR_H
#define		_HEADER_LINEMGR_H

#include "gap_buffer.h"
#include <QDebug>
#include "Buffer.h"

//#define		USE_STEP		1

typedef unsigned int uint;

struct LineInfo
{
public:
	LineInfo(int lineStart = 0, uint flags = 0)
		: m_lineStart(lineStart)
		, m_flags(flags)
		{}

public:
	int		operator+=(int delta) { return m_lineStart += delta; }
	void	setFlag(uint flag) { m_flags |= flag; }
	void	resetFlag(uint flag) { m_flags &= ~flag; }

public:
	int		m_lineStart;
	uint	m_flags;
};

template<bool useStep>
class LineMgrTmp //: public gap_buffer<int>
{
public:
	LineMgrTmp(Buffer *buffer)
		: m_buffer(buffer)
	{
		init();
	}

public:
	bool	isEmpty() const { return m_lv.isEmpty(); }
	int		lineCount() const { return m_lv.size(); }
	int		lineStartRaw(int ln) const { return m_lv[ln].m_lineStart; }
	uint	lineFlags(int ln) const { return ln < m_lv.size() ? m_lv[ln].m_flags : 0; }
	//	ポジション → （ポジションを含む行の）行インデックス変換
	int	positionToLine(int pos) const	//	0 org
	{
		if( lineCount() <= 1 ) return 0;
#if 1
		if( pos != 0 && pos >= m_buffer->size() ) {
			if( m_buffer->isEmpty() ) return 0;
			wchar_t ch = m_buffer->charAt(m_buffer->size() - 1);
			if( ch == '\r' || ch == '\n' )
				return lineCount() - 1;
			else
				return lineCount() - 2;
		}
#endif
		if( useStep ) {
			int start = lineStartPosition(m_stepIndex);
			if( pos >= start ) {
				if( m_stepIndex + 1 < lineCount() ) {
					int nxStart = lineStartPosition(m_stepIndex + 1);
					if( pos < nxStart )
						return m_stepIndex;
				}
				if( m_stepIndex + 2 < lineCount() ) {
					int nxStart = lineStartPosition(m_stepIndex + 2);
					if( pos < nxStart )
						return m_stepIndex + 1;
				}
			} else {
				if( m_stepIndex != 0 ) {
					int pvStart = lineStartPosition(m_stepIndex - 1);
					if( pos >= pvStart )
						return m_stepIndex - 1;
				}
			}
		}
		int lower = 0;
		int upper = lineCount() - 1;
		do {
			int middle = (upper + lower + 1) / 2; 	// Round high
			int posMiddle = lineStartRaw(middle);
			if( useStep ) {
				if( middle > m_stepIndex )
					posMiddle += m_stepSize;
			}
			if( pos < posMiddle ) {
				upper = middle - 1;
			} else {
				lower = middle;
			}
		} while( lower < upper );
		return lower;
	}
	//	行インデックス → 行頭ポジション変換
	int lineStartPosition(int line) const
	{
		if( line <= 0 ) return 0;	//lineStartRaw(0);
		if( line >= lineCount() ) line = lineCount() - 1;
		if( useStep )
			return lineStartRaw(line) + (line > m_stepIndex ? m_stepSize : 0);
		else
			return lineStartRaw(line);
	}
	int lastLineStartPosition() const
	{
		return lineStartPosition(lineCount() - 1);
	}
	int lineSize(int line) const
	{
		if( line + 1 >= lineCount() ) return 0;
		return lineStartPosition(line+1) - lineStartPosition(line);
	}
	void debugPrint() const
	{
		for(int i = 0; i < lineCount(); ++i) {
			qDebug() << i << lineStartPosition(i);
		}
		qDebug() << ".";
	}

public:
	void	init()
	{
		clear();
		push_back(0);
	}
	void clear()
	{
		m_lv.clear();
		//if( useStep )
			m_stepIndex = m_stepSize = 0;
	}
	void push_back(int v)
	{
		m_lv.push_back(v);
	}
	void setAt(int ln, int v) { m_lv.setAt(ln, v); }
	void addAt(int ln, int v) { m_lv.addAt(ln, v); }
	void addToDataFL(int first, int last, int delta) { m_lv.addToDataFL(first, last, delta); }
	void setLineFlag(int ln, uint flag) { m_lv.ref(ln).m_flags |= flag; }
	void resetLineFlag(int ln, uint flag) { m_lv.ref(ln).m_flags &= ~flag; }
	void clearLineFlag(int ln) { m_lv.ref(ln).m_flags = 0; }
	void clearLineFlags() { for (int i = 0; i != m_lv.size(); ++i) m_lv[i].m_flags = 0; }
	void setSavedFlag()
	{
		//for(auto itr = m_lv.begin(); itr != m_lv.end(); ++itr)
		//	(*itr).m_flags |= Buffer::LINEFLAG_SAVED;
		for(int ln = 0; ln < m_lv.size(); ++ln) {
			if( (m_lv.ref(ln).m_flags & Buffer::LINEFLAG_MODIFIED) != 0 )
				m_lv.ref(ln).m_flags |= Buffer::LINEFLAG_SAVED;
		}
	}
	void clearGlobalFlag()
	{
		for (int i = 0; i < m_lv.size(); ++i) {
			m_lv[i].m_flags &= ~Buffer::LINEFLAG_GLOBAL;
		}
	}
	void insert(int ln, int v)
	{
		m_lv.insert(ln, v);
	}
	void erase(int ln)
	{
		m_lv.erase(ln);
	}
	void erase(int ln, int n)
	{
		m_lv.erase(ln, n);
	}
	void setLineStartPosition(int line, int pos)
	{
		if( useStep ) {
			if( m_stepIndex < line )
				setStepIndex(line);
		}
		setAt(line, pos);
	}
	//	追加行の行長は0とみなす
	void insertLine(int line)
	{
		if( useStep ) {
			if( m_stepIndex < line )
				setStepIndex(line);
			insert(line, lineStartRaw(line));		//	行ベクターにline行を追加
			m_stepIndex++;
		} else
			insert(line, lineStartRaw(line));		//	行ベクターにline行を追加
	}
	void insertLine(int line, int pos)
	{
		if( useStep ) {
			if( m_stepIndex < line )
				setStepIndex(line);
			insert(line, pos);		//	行ベクターにline行を追加
			m_stepIndex++;
		} else
			insert(line, pos);		//	行ベクターにline行を追加
	}
	//	line の直後に 行頭位置 position の行を追加
	//	m_stepIndex <= line の場合は、m_stepIndex を新しく追加した行（line+1）に設定
	void addLine(int line, int position)
	{
		if( useStep ) {
			if( m_stepIndex < line )
				setStepIndex(line);
			insert(line+1, position);		//	行ベクターにline+1行を追加
			m_stepIndex++;
		} else
			insert(line+1, position);		//	行ベクターにline+1行を追加
	}
	void textInserted(int line, int delta)	//	line 行にdelta文字のテキストが挿入された時の処理
	{
		if( useStep ) {
			//if( line == size() - 1 ) {		//	EOF行に挿入された場合
			//	push_back(lineStartRaw(line));
			//}
			if( !m_stepSize ) {
				m_stepIndex = line;
				m_stepSize = delta;
			} else {
				if( line >= m_stepIndex ) {
					setStepIndex(line);
					m_stepSize += delta;
				} else if( line >= (m_stepIndex - lineCount() / 10) ) {
					setStepIndex(line);
					m_stepSize += delta;
				} else {
					setStepIndex(lineCount()-1);
					m_stepIndex = line;
					m_stepSize = delta;
				}
			}
		} else
			addToDataFL(line + 1, lineCount(), delta);
	}
	//	line 行を削除
	void eraseLine(int line)
	{
		if( useStep ) {
			if( m_stepIndex < line )
				setStepIndex(line);
			--m_stepIndex;		//	line 行は削除されるのでひとつ前に移動
		}
		erase(line);
	}
	void eraseLine(int line, int n)
	{
		if( !n ) return;
		if( useStep ) {
			if( m_stepIndex < line + n )
				setStepIndex(line + n);		//	make sure ステップインデックスを削除範囲以降
			m_stepIndex -= n;
		}
		erase(line, n);
	}
protected:
#if 0
	int bufferSize() const
	{
		if( isEmpty() ) return 0;
		int bs = lineStartRaw(lineCount() - 1);
		if( useStep ) {
			if( m_stepIndex < lineCount() - 1 )
				bs += m_stepSize;
		}
		return bs;
	}
#endif
	void setStepIndex(int line)
	{
		if( !useStep ) return;
		if( line == m_stepIndex ) return;
		if( !m_stepSize ) return;
		if( line > m_stepIndex ) {
			addToDataFL(m_stepIndex + 1, line + 1, m_stepSize);
			if( (m_stepIndex = line) >= lineCount() - 1 ) {
				m_stepIndex = lineCount() - 1;
				m_stepSize = 0;
			}
		} else {
			addToDataFL(line + 1, m_stepIndex + 1, -m_stepSize);
			m_stepIndex = line;
		}
	}
#if 0
	// Move step forward
	void ApplyStep(int partitionUpTo ) {
#if 0
		if( stepLength != 0 ) {
			body->RangeAddDelta(stepPartition+1, partitionUpTo + 1, stepLength);
		}
		stepPartition = partitionUpTo;
		if( stepPartition >= body->Length()-1 ) {
			stepPartition = body->Length()-1;
			stepLength = 0;
		}
#endif
	}

	// Move step backward
	void BackStep(int partitionDownTo ) {
#if 0
		if( stepLength != 0 ) {
			body->RangeAddDelta(partitionDownTo+1, stepPartition+1, -stepLength);
		}
		stepPartition = partitionDownTo;
#endif
	}
#endif


private:
	int		m_stepIndex;	//	ステップ位置、ここ以降は +m_stepSize されているとみなす
	int		m_stepSize;		//	ステップサイズ
	Buffer	*m_buffer;
	gap_buffer<LineInfo>	m_lv;

friend class TestClass;
};

//typedef LineMgrTmp<false> LineMgr;		//	ステップ未使用版

#endif		//_HEADER_LINEMGR_H
