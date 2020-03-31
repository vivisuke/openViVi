#include "viewLineMgr.h"
#include "Buffer.h"
#include <QDebug>

ViewLineMgr::ViewLineMgr(Buffer *buffer)
	: m_buffer(buffer)
{
}
bool ViewLineMgr::isEmpty() const		//	テーブルが空か？
{
	return m_lv.isEmpty();
}
int ViewLineMgr::size() const				//	テーブルサイズ
{
	return m_lv.size();
}
void ViewLineMgr::clear()
{
	m_lv.clear();
}
int ViewLineMgr::EOFLine() const
{
	if (m_lv.isEmpty()) {
		return m_buffer->EOFLine();
	} else {
		int vln = m_lv.size() - 1;
		if( !m_buffer->isBlankEOFLine() ) --vln;
		return vln;
	}
}
int ViewLineMgr::viewLineToDocLine(int vln) const
{
	if( m_lv.isEmpty() )
		return vln;
	if( vln >= m_lv.size() )
		return m_buffer->lineCount();
	if( vln < 0 ) vln = 0;
	return m_lv.at(vln).m_docLine;
}
int ViewLineMgr::docLineToViewLine(int ln) const
{
	if( m_lv.isEmpty() )
		return ln;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	int first = 0;
	int last = m_lv.size();
	while( first < last - 1 ) {
		int ix = (first + last) / 2;
		if( ln < m_lv[ix].m_docLine ) {
			last = ix;
		} else if( ln > m_lv[ix].m_docLine ) {
			first = ix;
		} else
			return ix;
	}
	return first;
}
bool ViewLineMgr::isCollapsed(int vln) const
{
	if (m_lv.isEmpty()) {
		return false;
	} else {
		return vln <= m_lv.size() - 1
			&& m_lv[vln].m_docLine + 1 != m_lv[vln + 1].m_docLine;
	}
}
void ViewLineMgr::collapse(int ln1, int ln2)		//	(ln1, ln2] 行を削除
{
	if( ln1 >= ln2 ) return;
#if	1
	if( m_lv.isEmpty() ) {
		m_lv.resize(m_buffer->lineCount() - (ln2 - ln1) + 1);
		int  i = 0;
		for (; i <= ln1; ++i) {
			m_lv[i].m_docLine = i;
		}
		for (int  k = ln2 + 1; k <= m_buffer->lineCount(); ++k, ++i) {
			m_lv[i].m_docLine = k;
		}
	} else {
		m_lv.eraseFL(ln1+1, ln2+1);
	}
#endif
}
//	vln の折り畳み解除
void ViewLineMgr::expand(int vln)
{
	if (m_lv.isEmpty()) return;
	int ln = m_lv[vln++].m_docLine + 1;		//	次の行のドキュメント行番号
	while( vln < m_lv.size() && m_lv[vln].m_docLine != ln ) {
		m_lv.insert(vln++, ViewLine(ln++));
	}
}
//	（ドキュメント）dln 行に d 行挿入された場合の処理
void ViewLineMgr::inserted(int dln, int d)
{
	if( m_lv.isEmpty() ) return;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	int vln = docLineToViewLine(dln);
	for(int i = 0; i < d; ++i)
		m_lv.insert(vln++, ViewLine(dln++));
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	//undone: ステップベクター対応
	while( vln < m_lv.size() )
		m_lv[vln++].m_docLine += d;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	//qDebug() << "\n";
}
//	d should be > 0
void ViewLineMgr::deleted(int dln, int d)
{
	if( m_lv.isEmpty() ) return;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	int vln = docLineToViewLine(dln);
	m_lv.erase(vln, d);
	while( vln < m_lv.size() )
		m_lv[vln++].m_docLine -= d;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
}
