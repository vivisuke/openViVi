#include "viewLineMgr.h"
#include "EditView.h"
#include "drawTokenizer.h"
#include "typeSettings.h"
#include "../Buffer/Buffer.h"
#include <QDebug>

inline bool isNewLineChar(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}

ViewLineMgr::ViewLineMgr(EditView *view /*, Buffer *buffer*/)
	: m_view(view)
	, m_buffer(view->buffer())
	, m_lineBreak(false)
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
bool ViewLineMgr::isLineBreakMode() const
{
	return m_lineBreak;
}
void ViewLineMgr::clear()
{
	m_lineBreak = false;
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
//	同じ論理行の最初の行の表示行番号を返す
int ViewLineMgr::logicalFirstLine(int vln) const
{
	if( m_lv.isEmpty() )
		return vln;
	while( m_lv.at(vln).m_offset > 0 )
		--vln;
	return vln;
}
//	折り返しの場合、最初の行か？
bool ViewLineMgr::isLogicalFirstLine(int vln) const
{
	if( m_lv.isEmpty() )
		return true;
	else
		return m_lv.at(vln).m_offset <= 0;
}
int ViewLineMgr::viewLineStartPosition(int vln) const
{
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	return m_buffer->lineStartPosition(dln) + offset;
}
int ViewLineMgr::viewLineSize(int vln) const
{
	if( m_lv.isEmpty() ) {
		return m_buffer->lineStartPosition(vln+1) - m_buffer->lineStartPosition(vln);
	}
	if( vln + 1 >= m_lv.size() ) return 0;		//	EOF 行
	const int dln = m_lv[vln].m_docLine;
	const int nxdln = m_lv[vln+1].m_docLine;
	if( nxdln == dln )
		return m_lv[vln+1].m_offset - m_lv[vln].m_offset;
	else {
		//	折畳まれている場合があるので、次の論理行の先頭位置から現行位置を引く
		const pos_t nxls = m_buffer->lineStartPosition(dln+1);
		const pos_t ls = viewLineStartPosition(vln);
		return nxls - ls;
	}
}
int ViewLineMgr::viewLineToDocLine(int vln) const
{
	int offset;
	return viewLineToDocLine(vln, offset);
}
int ViewLineMgr::viewLineToDocLine(int vln, int &offset) const
{
	offset = 0;
	if( m_lv.isEmpty() )
		return vln;
	if( vln >= m_lv.size() )
		return m_buffer->lineCount();
	if( vln < 0 ) vln = 0;
	offset = m_lv.at(vln).m_offset;
	if( offset < 0 ) offset = 0;
	return m_lv[vln].m_docLine;
	//return m_lv.at(vln).m_docLine;
}
int ViewLineMgr::positionToViewLine(pos_t pos) const
{
	if( m_lv.isEmpty() )
		return m_buffer->positionToLine(pos);
	if( pos >= m_buffer->size() )
		return EOFLine();
	int first = 0;
	int last = m_lv.size();
	while( first < last - 1 ) {
		const int ix = (first + last) / 2;
		const pos_t ls = viewLineStartPosition(ix);
		if( pos < ls ) {
			last = ix;
		} else if( pos > ls ) {
			first = ix;
		} else
			return ix;
	}
	return first;
}
int ViewLineMgr::docLineToViewLine(int ln) const
{
	if( m_lv.isEmpty() )
		return ln;
	//for (int  i = 0; i < m_lv.size(); ++i) {
	//	qDebug() << i << ": " << m_lv[i];
	//}
	//	undone: 行番号キャッシュ
	int first = 0;
	int last = m_lv.size();
	while( first < last - 1 ) {
		int ix = (first + last) / 2;
		if( ln < m_lv[ix].m_docLine ) {
			last = ix;
		} else if( ln > m_lv[ix].m_docLine ) {
			first = ix;
		} else {
			while( m_lv[ix].m_offset > 0 ) --ix;	//	論理先頭行に移動
			return ix;
		}
	}
	return first;
}
bool ViewLineMgr::isCollapsed(int vln) const
{
	if (m_lv.isEmpty()) {
		return false;
	} else {
		int dln = m_lv[vln].m_docLine;
		for(;;) {
			if( ++vln == m_lv.size() ) return false;
			if( m_lv[vln].m_docLine != dln )
				return m_lv[vln].m_docLine != dln + 1;
		}
	}
}
//	(vln1, vln2] 行を削除
//	ただし、vln1 からの折り返し行は非表示にしない
void ViewLineMgr::collapse(int vln1, int vln2)
{
	if( vln1 >= vln2 ) return;
	if( m_lv.isEmpty() ) {
		m_lv.resize(m_buffer->lineCount() - (vln2 - vln1) + 1);
		int  i = 0;
		for (; i <= vln1; ++i) {
			m_lv[i].m_docLine = i;
		}
		for (int  k = vln2 + 1; k <= m_buffer->lineCount(); ++k, ++i) {
			m_lv[i].m_docLine = k;
		}
	} else {
		const int dln = viewLineToDocLine(vln1);
		while( vln1 + 1 && m_lv[vln1+1].m_docLine == dln )
			++vln1;
		m_lv.eraseFL(vln1+1, vln2+1);
	}
}
//	vln の折り畳み解除
void ViewLineMgr::expand(int vln)
{
	if (m_lv.isEmpty()) return;
	int vln0 = vln;
	int dln = m_lv[vln].m_docLine;
	for(;;) {
		if( vln + 1 >= m_lv.size() ) return;
		if( m_lv[vln+1].m_docLine != dln )
			break;
		++vln;
	}
	int dln2 = m_lv[vln+1].m_docLine;		//	次の行のドキュメント行番号
	for (int  i = dln + 1; i < dln2; ++i) {
		m_lv.insert(++vln, ViewLine(++dln, -1));
	}
}

//	for Debug
void ViewLineMgr::print() const
{
#if	0
	for (int  i = 0; i < m_lv.size(); ++i) {
		qDebug() << i << ": " << m_lv[i].m_docLine
						<< " " << m_lv[i].m_offset;
	}
	qDebug() << " ";
#endif
}
void ViewLineMgr::check() const		//	for Debug
{
	int dln = 0;
	for (int  i = 0; i < m_lv.size(); ++i) {
		if( m_lv[dln].m_docLine < dln ) {
			qDebug() << "???";
		}
		dln = m_lv[dln].m_docLine;
	}
}

//	（ドキュメント）dln 行に d 行挿入された場合の処理
//	単に文字が挿入された場合でも d = 0 でコールされる
//
//		(1) dln 行の折り返し行を削除
//		(2) 挿入行分の表示行を挿入し
//		(3) 挿入範囲以降の論理行番号を += d
//		(4) 折り返し処理のためにオフセットを -1 に設定
void ViewLineMgr::inserted(int dln, int d)
{
	if( m_lv.isEmpty() ) return;
	print();
	int vln = docLineToViewLine(dln);
	int vln0 = vln;
	int dln0 = dln;
	//		(1) dln 行の折り返し行を削除
	while( vln + 1 < m_lv.size() && m_lv[vln+1].m_offset > 0 )
		m_lv.erase(vln+1);
	//		(2) 挿入行分の表示行を挿入し
	for(int i = 0; i < d; ++i) {
		m_lv.insert(vln++, ViewLine(dln++));
	}
	print();
	//		(3) 挿入範囲以降の論理行番号を += d
	if( d != 0 ) {
		while( vln < m_lv.size() )
			m_lv[vln++].m_docLine += d;
		print();
	}
	if( m_lineBreak ) {	//	折り返し状態
		for(int i = 0; i <= d; ++i, ++vln0) {
			if( vln0 + i >= m_lv.size() ) break;
			//doLineBreak(dln0+i, vln0);
			m_lv[vln0+i].m_offset = -1;		//	後でラインブレイク処理
		}
		print();
	}
#if	0
	if( m_lineBreak ) {	//	折り返し状態
		for(int i = 0; i < d; ++i) {
			m_lv.insert(vln+i, ViewLine(dln+i));
		}
		while( m_lv[vln].m_docLine == dln )
			m_lv.erase(vln);
		doLineBreak(dln, vln);
		for(int i = 0; i < d; ++i) {
			doLineBreak(dln, vln);
			++dln;
		}
		print();
	} else {
		for(int i = 0; i < d; ++i) {
			m_lv.insert(vln++, ViewLine(dln++));
		}
	}
#endif
	//qDebug() << " ";
}
//	d は削除された論理行数
//	単に文字が削除された場合でも d = 0 でコールされる
void ViewLineMgr::deleted(int dln, int d)
{
	if( m_lv.isEmpty() ) return;
	int vln0 = docLineToViewLine(dln);
	int vln = vln0;
	print();
	if( d != 0 ) {
		int v = docLineToViewLine(dln + d);
		while( v < m_lv.size() && m_lv[v].m_docLine < dln + d )
			++v;
		m_lv.erase(vln, v - vln);
		print();
		while( vln < m_lv.size() )
			m_lv[vln++].m_docLine -= d;
		print();
	}
	if( m_lineBreak ) {
		while( vln0 + 1 < m_lv.size() && m_lv[vln0+1].m_docLine == dln )
			m_lv.erase(vln0+1);
		//doLineBreak(dln, vln0);
		if( vln0 < m_lv.size() )
			m_lv[vln0].m_offset = -1;	//	後でラインブレイク処理
		print();
	}
}
void ViewLineMgr::setLineBreak(bool b)
{
	if( b == m_lineBreak ) return;
	m_lineBreak = b;
	if( m_lineBreak )
		doLineBreakAll();
	else
		clear();
}
void ViewLineMgr::doLineBreakAll()
{
	qDebug() << "*** doLineBreakAll() ***\n";
#if	1
	if( m_lv.isEmpty() ) {
		const int cnt = m_buffer->lineCount();
		m_lv.resize(cnt + 1);
		for(int dln = 0; dln <= cnt; ++dln) {
			m_lv[dln].m_docLine = dln;
			m_lv[dln].m_offset = -1;
		}
	} else {
		//	折り畳み状態を維持するため、折り畳み２行目以降をすべて削除、行頭オフセットは -1 に設定
		for(int i = m_lv.size(); --i >= 0; ) {
			if( m_lv[i].m_offset > 0 )
				m_lv.erase(i);
			else
				m_lv[i].m_offset = -1;
		}
	}
#else
#if	1
	//qDebug() << "m_buffer->lineCount() = " << m_buffer->lineCount();
	const int cnt = m_buffer->lineCount();
	m_lv.resize(cnt + 1);
	//qDebug() << "m_lv.size() = " << m_lv.size();
	for(int dln = 0; dln <= cnt; ++dln) {
		m_lv[dln].m_docLine = dln;
		m_lv[dln].m_offset = -1;
	}
	//qDebug() << "m_buffer->lineCount() = " << m_buffer->lineCount();
	//qDebug() << "m_lv.size() = " << m_lv.size();
	print();
#else
	clear();
	int vln = 0;
	for(int dln = 0; dln <= m_buffer->lineCount(); ++dln, ++vln) {
#if	1
		m_lv.insert(vln, ViewLine(dln, -1));
#else
		m_lv.insert(vln, ViewLine(dln, 0));
		doLineBreak(dln, vln);
#endif
	}
#endif
#endif
}
void ViewLineMgr::doLineBreak(int dln, int &vln)
{
#if	1		//##
	if( m_lv[vln].m_offset < 0 )
		m_lv[vln].m_offset = 0;
#if	1
	int width = qMax(64, m_view->width() - m_view->lineNumberAreaWidth() - 64);
	pos_t ls = m_buffer->lineStartPosition(dln);
	pos_t vls = ls;
	int offset;
	while( (offset = m_view->pxToOffset(vln, width)) != 0) {
		pos_t pos = vls + offset;
		//pos = qMin(pos, m_buffer->size());
		Q_ASSERT(pos >= 0 && pos <= m_buffer->size());
		if( pos == m_buffer->size() || isNewLineChar(m_buffer->charAt(pos)) )
			break;
		//if( m_lv.size() >= 20 ) {
		//	qDebug() << m_lv.size();
		//}
		m_lv.insert(++vln, ViewLine(dln, pos - ls));
		vls = pos;
	}
#else
#if	1
	int width = m_view->width() - 64;
	QFontMetrics fm = m_view->fontMetrics();
	QFontMetrics fmBold = m_view->fontBold();
	int nTab = m_view->typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int tabWidth = fm.width(QString(nTab, QChar(' ')));
	int sz = m_buffer->lineSize(dln);
	pos_t ls = m_buffer->lineStartPosition(dln);
	pos_t nxls = m_buffer->lineStartPosition(dln+1);
	DrawTokenizer dt(m_view->typeSettings(), m_buffer, ls, nxls - ls, nxls);
	QString token;
	int wd = 0;		//	現在行長
	int wd2;			//	トークン長を加えた行長
	for(;;) {
		token = dt.nextToken();
		if( token.isEmpty() ) break;
		if( token == "\t" ) {
			wd2 = (wd / tabWidth + 1) * tabWidth;
			//wd += tabWidth;
		} else if( token[0].unicode() < 0x20 ) {
			wd2 = wd + fm.width(QString(QChar('@' + token[0].unicode())));
		} else {
			QString token0 = token;
			//if( dt.ix() > first + sz ) {
			//	token = token.left(first + sz - dt.tokenix());
			//}
			//if( !dt.isInLineComment() && !dt.isInBlockComment()
			//		&& dt.tokenType() == DrawTokenizer::ALNUM
			//		&& (!bHTML || dt.isInHTMLTag())
			//		&& (pTypeSettings->boolValue(TypeSettings::KEYWORD1_BOLD)
			//				&& pTypeSettings->isKeyWord1(token0)
			//			|| pTypeSettings->boolValue(TypeSettings::KEYWORD2_BOLD)
			//				&& pTypeSettings->isKeyWord2(token0)) )
			//{
			//	wd2 += fmBold.width(token);
			//} else
				wd2 = wd + fm.width(token);
		}
		if( wd2 >= width ) {
			int offset = dt.ix() - ls;
			m_lv.insert(++vln, ViewLine(dln, offset));
		} else
			wd = wd2;
	}
#else
	const int len = 10;
	int sz = m_buffer->lineSize(dln);
	pos_t ls = m_buffer->lineStartPosition(dln);
	int offset = 0;
	while( (sz -= len) > 0 && !isNewLineChar(m_buffer->charAt(ls+offset+len)) ) {
		m_lv.insert(++vln, ViewLine(dln, offset += len));
	}
#endif
#endif
#endif
}
bool ViewLineMgr::ensureLineBreak(int vln)
{
	if( m_lv.empty() || m_lv[vln].m_offset >= 0 )		//	折り返し済み
		return false;
	//qDebug() << "vln = " << vln;
	//qDebug() << m_lv[vln].m_docLine << ", " << m_lv[vln].m_offset;
	doLineBreak(m_lv[vln].m_docLine, vln);
	//qDebug() << "vln = " << vln;
	//qDebug() << m_lv[vln].m_docLine << ", " << m_lv[vln].m_offset;
	return true;
}
