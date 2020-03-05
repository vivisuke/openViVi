//----------------------------------------------------------------------
//
//			File:			"Buffer.h"
//			Created:		09-Jun-2013
//			Author:			津田伸秀
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_BUFFER_H
#define		_HEADER_BUFFER_H

//#include <QObject>

typedef unsigned char byte;
typedef unsigned int uint;

#include <vector>
#include <deque>
#include <ctype.h>

#define		USE_GAP_DEQUE		0

#if	USE_GAP_DEQUE
#include "gap_deque.h"
#endif

#if 1
#ifndef	wchar_t
//typedef unsigned short wchar_t;
#endif
#endif
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号

template<typename T> class gap_buffer;
//template<typename T>
class UndoMgr;
template<bool> class LineMgrTmp;
#if 1
typedef LineMgrTmp<true> LineMgr;		//	ステップ使用版
#else
typedef LineMgrTmp<false> LineMgr;		//	ステップ未使用版
#endif
//class SearchEngine;
struct MarkItem;
class MarkMgr;
//class ViewLineMgr;

class UndoActionInsert;
class UndoActionDelete;
class SSSearch;

class Buffer //: public QObject
{
	//Q_OBJECT

public:
	typedef wchar_t char_t;
	typedef const wchar_t cchar_t;

	enum {
		CASE_SENSITIVE = 0,
		IGNORE_CASE,

		LINEFLAG_IN_BLOCK_COMMENT = 0x01,
		LINEFLAG_MODIFIED = 0x02,
		LINEFLAG_SAVED = 0x04,
		LINEFLAG_IN_SCRIPT = 0x08,		//	<script> </script> 間
		LINEFLAG_IN_PHP = 0x10,				//	<? ?> 間
		LINEFLAG_IN_TAG = 0x20,				//	< > 間
		LINEFLAG_CAN_COLLAPSE = 0x40,	//	折畳可能行
		LINEFLAG_GLOBAL = 0x80,			//	:g, :v で使用
		
		EDIT_POS_SIZE = 10,		//	編集箇所数最大値
	};

public:
	Buffer();
	Buffer(const Buffer &);
	~Buffer();

public:
	bool	isEmpty() const;	// { return m_buffer.empty(); }
	bool	isBlankEOFLine() const;
	bool	isModified() const { return m_modified; };
	int	seqNumber() const { return m_seqNumber; }
	//bool	isLastCharNewLine() const;
	ssize_t		size() const;	// { return m_buffer.size(); }
	line_t		lineCount() const;
	line_t		EOFLine() const;
	pos_t		lineStartPosition(line_t line) const;
	ssize_t		lineSize(line_t line) const;
	line_t		positionToLine(pos_t pos) const;
	char_t	charAt(pos_t) const;
	char_t	*getText(pos_t &) const;
	const char_t	*raw_data(pos_t pos) const;
	bool	getText(pos_t, char_t *buf, int length) const;
	bool	getText(pos_t pos, ssize_t sz, std::vector<char_t> &) const;
	bool	isEqual(pos_t pos, const char_t *ptr) const;
	bool	canUndo() const;
	bool	canRedo() const;
	//bool	startsWith(pos_t pos, cchar_t *pat, ssize_t sz) const;
	pos_t		strstr(cchar_t *pat, ssize_t sz, pos_t from = 0, pos_t last = -1, bool ic = false) const;
	pos_t		strrstr(cchar_t *pat, ssize_t sz, pos_t from = -1, pos_t last = 0, bool ic = false) const;
	//int		indexOf(char_t *pat, char_t *pend, int from = 0, bool ic = false) const;
	//int		indexOf(cchar_t *pat, int from = 0, bool ic = false) const;
	//	from:検索開始位置、last：検索終了位置
	pos_t		indexOf(SSSearch &, cchar_t *pat, ssize_t sz, pos_t from = 0, uint opt = 0, pos_t last = -1, byte = 0) const;
	//	from:検索開始位置、last：検索終了位置
	pos_t		rIndexOf(SSSearch &, cchar_t *pat, ssize_t sz, pos_t from = 0, uint opt = 0, pos_t last = -1, byte = 0) const;
	bool	isMatched(cchar_t *pat, ssize_t sz, pos_t pos) const;
	bool	isMatched(cchar_t *pat, pos_t pos) const;
	bool	isMatchedIC(cchar_t *pat, ssize_t sz, pos_t pos) const;
	bool	isMatchedIC(cchar_t *pat, pos_t pos) const;
	bool	operator==(const Buffer &) const;
	bool	operator!=(const Buffer &x) const { return !operator==(x); }
	char_t	operator[](pos_t pos) const { return charAt(pos); }
	uint	lineFlags(int ln) const;
	char	isMarked(pos_t pos) const;	//	マークされていない場合は 0 を返す
	int		markPos(char ch) const;
	void	getMarks(std::vector<MarkItem> &) const;
	bool	isWordBegin(pos_t) const;
	bool	isWordEnd(pos_t) const;
	int		lastModifiedPos() const { return m_lastModifiedPos; }
	void	print() const;
	const std::vector<pos_t> &emphasizedRanges() const { return m_emphasizedRanges; }
	int		gapIndex() const;
	void	setGapIndex(int) const;
	bool	isSpaces(pos_t, pos_t) const;
	int		undoIndex() const;
	bool	isUndoBlockOpened() const;
	bool	canCollapse(line_t ln) const;
	int	editedPosCount() const { return (int)m_editedPos.size(); }
	int	editedPos(pos_t ix) const { return m_editedPos[ix]; }

public:
	void	clear() { init(); }
	void	init();
	void	setModified(bool = true);
	void	clearLineFlags();
	void	prohibitMergeUndo();			//	挿入マージ禁止
	void	onSaved();			//	保存時にコールされる
	char_t	*data();
	bool	insertText(pos_t, cchar_t *, ssize_t, int = -1);		//	挿入、undo 対応
	bool	deleteText(pos_t, ssize_t, bool BS=false, int = -1);	//	削除、undo 対応
//	置換、undo 対応
	bool	replaceText(pos_t pos, ssize_t dsz, cchar_t *, int isz, int = -1,
									bool = false);		//	編集箇所更新
	int		replaceAll(cchar_t *before, ssize_t, cchar_t *after, ssize_t, uint opt = 0, byte = 0);
	int		replaceAll(cchar_t *before, ssize_t, cchar_t *after, ssize_t, uint opt, byte, pos_t, pos_t &, pos_t &, bool = true);
	int		undo();
	int		redo();
	void	openUndoBlock();
	void	closeUndoBlock();
	void	closeAllUndoBlock();
	void	clearUndoMgr();
	//	undo 処理を行わない編集メソッド
	bool	basicInsertText(pos_t, cchar_t);
	bool	basicInsertText(pos_t, cchar_t *, cchar_t *, line_t ln = -1);
	bool	basicInsertText(pos_t, cchar_t *, ssize_t, line_t ln = -1);
	void	basicDeleteText(pos_t);
	void	basicDeleteText(pos_t, ssize_t sz, line_t ln = -1);
	void	basicReplaceText(pos_t, ssize_t dsz, cchar_t *, ssize_t isz, line_t ln = -1);
	//	↑undo 処理を行わない編集メソッド
	void	rebuildLineMgr();
	void	setLineFlag(line_t, uint);
	void	resetLineFlag(line_t, uint);
	void	setMark(pos_t pos, char ch = '\0');		//	ch = ['a', 'z'], pos < 0 for clear Mark
	void	clearMark(char ch);
	void	clearMark(pos_t pos);
	void	setEmphasizedRanges(const std::vector<pos_t> &v) { m_emphasizedRanges = v; }
	void	setCanCollapse(line_t ln, bool b);
	//ViewLineMgr	*createViewLineMgr();
	void	incSeqNumber() { ++m_seqNumber; }
	int	nextEditedPos();
	int	prevEditedPos();
	bool	setGlobalFlag(const wchar_t *, ssize_t sz);		//	行の LINEFLAG_GLOBAL をセット/アンセット

protected:
	void	inserted(pos_t pos, ssize_t sz);		//	削除範囲修正
	void	deleted(pos_t pos, ssize_t sz);

#if	0
signals:
	void	onCleared();
	void	onInserted(int dln, int d);
	void	onDeleted(int dln, int d);			//	d should be > 0
#endif

private:

private:
	//class GapBuffer	*m_buffer;
#if	USE_GAP_DEQUE
	gap_deque<char_t>	*m_buffer;		//	テキスト生データ
#else
	gap_buffer<char_t>	*m_buffer;		//	テキスト生データ
#endif
	LineMgr		*m_lineMgr;				//	行情報管理クラス
	UndoMgr		*m_undoMgr;				//	Undo管理クラス
	MarkMgr		*m_markMgr;				//	マーク管理
	pos_t		m_lastModifiedPos;		//	最終編集位置
	int		m_seqNumber;			//	編集操作ごとに＋１される値
	bool		m_modified;
	std::vector<pos_t>	m_emphasizedRanges;		//	削除（開始、終了）位置
	int		m_epix;
	std::deque<pos_t>	m_editedPos;						//	最近の編集位置
	//std::vector<ViewLineMgr*>	m_viewLineMgrs;

friend class UndoMgr;
friend class TestClass;
};


#endif		//_HEADER_BUFFER_H
