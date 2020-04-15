#pragma once

#include <QObject>
#include "../Buffer/gap_buffer.h"

#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号
	
class Buffer;
class EditView;

struct ViewLine
{
public:
	enum {
		HEADING = 0x0001,
	};
public:
	ViewLine(int docLine = 0, int offset = 0 /*, int flags = 0*/)
		: m_docLine(docLine)
		, m_offset(offset)
		//, m_flags(flags)
		{}
	ViewLine(const ViewLine &x)
		: m_docLine(x.m_docLine)
		, m_offset(x.m_offset)
		//, m_flags(x.m_flags)
		{}
public:
	int	m_docLine;
	int	m_offset;				//	表示開始オフセット
	//int	m_flags;
};

class ViewLineMgr : public QObject
{
	Q_OBJECT
	
public:
	ViewLineMgr(EditView*);

public:
	bool	isEmpty() const;		//	テーブルが空か？
	int	size() const;				//	テーブルサイズ
	int	EOFLine() const;
	int	viewLineToDocLineRaw(int vln) const { return m_lv[vln].m_docLine; }
	int	viewLineToDocLine(int vln) const;
	int	viewLineToDocLine(int vln, int &offset) const;
	int	docLineToViewLine(int ln) const;
	int	positionToViewLine(pos_t pos) const;
	int	viewLineStartPosition(int vln) const;
	int	viewLineSize(int vln) const;
	int	logicalFirstLine(int vln) const;			//	同じ論理行の最初の行の表示行番号を返す
	bool	isLogicalFirstLine(int vln) const;
	bool	isCollapsed(int vln) const;
	bool isLineBreakMode() const;
	
	void	print() const;		//	for Debug
	void	check() const;		//	for Debug

public:
	//void	collapseAll();		//	折り畳み可能行をすべて折り畳む
	void	collapse(int ln1, int ln2);		//	(ln1, ln2) 行を削除
	void	expand(int ln);
	void	setLineBreak(bool);
	void	doLineBreakAll();
	void	doLineBreak(int dln, int &vln);
	bool	ensureLineBreak(int vln);		//	折り返し処理を行ったら true を返す
	
public slots:
	void	clear();
	void	inserted(int dln, int d);
	void	deleted(int dln, int d);			//	d should be > 0

protected:
	
private:
	bool		m_lineBreak;		//	折り返しモード
	Buffer	*m_buffer;
	EditView	*m_view;
	gap_buffer<ViewLine>	m_lv;		//	ビュー行→バッファ行変換テーブル
};
