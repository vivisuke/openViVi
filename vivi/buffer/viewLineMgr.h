#include "gap_buffer.h"

class Buffer;

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

class ViewLineMgr
{
public:
	ViewLineMgr(Buffer *);

public:
	bool	isEmpty() const;		//	テーブルが空か？
	int	size() const;				//	テーブルサイズ
	int	EOFLine() const;
	int	viewLineToDocLine(int ln) const;
	int	docLineToViewLine(int ln) const;
	bool	isCollapsed(int vln) const;

public:
	void	clear();
	void	inserted(int dln, int d);
	void	deleted(int dln, int d);			//	d should be > 0
	void	collapse(int ln1, int ln2);		//	(ln1, ln2) 行を削除
	void	expand(int ln);
	
	void	doLayoutAll();

private:
	Buffer	*m_buffer;
	gap_buffer<ViewLine>	m_lv;		//	ビュー行→バッファ行変換テーブル
};
