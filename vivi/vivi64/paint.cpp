#include <assert.h>
#include <QtGui>
#include <QTransform>
#include <QMessageBox>
//#include <QPainter>
#include <QDebug>
#include "MainWindow.h"
#include "Document.h"
#include "EditView.h"
#include "tokenizer.h"
#include "ViewTokenizer.h"
#include "typeSettings.h"
#include "globalSettings.h"
#include "TextCursor.h"
#include "viewLineMgr.h"
#include "charEncoding.h"
#include "ViEngine.h"
#include "AutoCompletionDlg.h"
#include "assocParen.h"
#include "../buffer/Buffer.h"
#include "../buffer/bufferUtl.h"
#include "../buffer/UTF16.h"
#include "../buffer/sssearch.h"

void EditView::paintEvent(QPaintEvent *event)
{
	QFontMetrics fm(m_font);
	if( m_fontHeight != QFontInfo(m_font).pixelSize() )
		updateFont();
#if	0	//def	_DEBUG
	qDebug() << "m_lineHeight = " << m_lineHeight;
	qDebug() << "m_fontHeight = " << m_fontHeight;
	QFontMetrics fm(m_font);
	const auto descent = fm.descent();
	qDebug() << "descent = " << descent;
#endif
	m_preeditWidth = fm.width(m_preeditString);
	//qDebug() << "lineCount = " << buffer()->lineCount();
	QPainter pt(this);
	//QPainter pt2(&m_textAreaPixmap);
	//QPainter pt2(&m_textAreaWidget);
	auto rct = rect();
	auto wd = rct.width();
	//qDebug() << "rect = " << rct;
	//	全体背景描画
	pt.setPen(Qt::transparent);
	//pt2.setPen(Qt::transparent);
	QColor bg = typeSettings()->color(TypeSettings::BACK_GROUND);
	//pt2.setBrush(bg);
	//pt2.drawRect(m_textAreaPixmap.rect());
#if	1
	//QColor col = typeSettings()->color(TypeSettings::BACK_GROUND);
	pt.setBrush(bg);
	pt.drawRect(rct);
#endif
	//
	paintMatchedBG(pt);
	paintSelection(pt);
	paintAssocParenBG(pt);
	paintCursor(pt);				//	テキストカーソル表示
	paintPreeditString(pt);
	paintTextArea(pt);			//	テキストエリア描画
	paintMinMap(pt);				//	ミニマップ描画
	paintLineCursor(pt);			//	行カーソル表示
	//	行番号部分背景描画
	pt.setOpacity(1.0);
	QColor col = typeSettings()->color(TypeSettings::LINENUM_BG);
	pt.setPen(Qt::transparent);
	pt.setBrush(col);
	rct.setWidth(m_lineNumAreaWidth);
	pt.drawRect(rct);
	paintLineNumberArea(pt);		//	行番号エリア描画
	//
#if 0
	pt.setOpacity(1.0);
	rct.setX(m_lineNumAreaWidth);
	rct.setWidth(wd - m_lineNumAreaWidth - MINMAP_WIDTH);
	pt.drawPixmap(rct, m_textAreaPixmap, m_textAreaPixmap.rect());
#endif
	//	削除された落下文字描画
	if( !m_fallingChars.empty() ) {
		QPoint hp(m_scrollX0*m_fontWidth, 0);
		pt.setPen(typeSettings()->color(TypeSettings::DEL_TEXT));
		QTransform t;
		for(int i = 0; i < m_fallingChars.size(); ++i) {
			//##t.translate(-(m_fallingChars[i].m_pnt.x() - hp.x() + m_fontWidth/2), -(m_fallingChars[i].m_pnt.y() - m_fontHeight/2));
			//##t.rotateRadians(3.1415926535/6);
			//##pt.setTransform(t);
			pt.drawText(m_fallingChars[i].m_pnt - hp, m_fallingChars[i].m_text);
			//qDebug() << m_fallingChars[i].m_pnt - hp;
		}
		pt.setTransform(QTransform());	//	回転リセット
	}
}
void EditView::paintLineNumberArea(QPainter& pt)
{
	auto rct = rect();
	pt.setPen(typeSettings()->color(TypeSettings::LINENUM));
    int mg = m_fontWidth*2;		//.width("88");
    int mg4 = mg / 4;
	int py = 0 /*DRAW_Y_OFFSET*/;
	if( !m_viewLineMgr->isLineBreakMode() ) {		//	非折返しモード
		int limit = buffer()->lineCount() + (buffer()->isBlankEOFLine() ? 1 : 0);
		for (int ln = 1 + m_scrollY0; ln <= limit && py < rct.height(); ++ln, py+=m_lineHeight) {
			//	行番号、行編集・保存済みフラグ表示
			uint flags = buffer()->lineFlags(ln-1);
			if( (flags & Buffer::LINEFLAG_MODIFIED) != 0 ) {
				if( (flags & Buffer::LINEFLAG_SAVED) != 0 ) {
					pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
										typeSettings()->color(TypeSettings::LINENUM_SAVED));
				} else {
					pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
										typeSettings()->color(TypeSettings::LINENUM_MODIFIED));
				}
			}
			//qDebug() << "line flags = " << flags;
			//
			QString number = QString::number(ln);
			int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
			pt.drawText(px, py+m_baseLineDY, number);
		}
	} else {	//	折返しモード
		int limit = m_viewLineMgr->size() + (buffer()->isBlankEOFLine() ? 1 : 0);
		for (int vln = 1 + m_scrollY0; vln <= limit && py < rct.height(); ++vln, py+=m_lineHeight) {
			//	論理行番号、行編集・保存済みフラグ表示
			int offset;
			int ln = m_viewLineMgr->viewLineToDocLine(vln, offset);
			uint flags = ln != 0 ? buffer()->lineFlags(ln-1) : 0;
			if( (flags & Buffer::LINEFLAG_MODIFIED) != 0 ) {
				if( (flags & Buffer::LINEFLAG_SAVED) != 0 ) {
					pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
										typeSettings()->color(TypeSettings::LINENUM_SAVED));
				} else {
					pt.fillRect(QRect(m_lineNumAreaWidth - mg + mg4, py, mg4, lineHeight()),
										typeSettings()->color(TypeSettings::LINENUM_MODIFIED));
				}
			}
			//qDebug() << "line flags = " << flags;
			//
			if( !offset ) {
				QString number = QString::number(ln);
				int px = m_lineNumAreaWidth - m_fontWidth*(3 + (int)log10(ln));
				pt.drawText(px, py+m_baseLineDY, number);
			}
		}
	}
}
void EditView::paintMatchedBG(QPainter&pt)
{
	auto pat = mainWindow()->matchedString();
	if (pat.isEmpty()) return;
	if( mainWindow()->mode() == MODE_VI ) {
		//if( !mainWindow()->willShowMatchedMG() )
		//	return;
	} else if( mainWindow()->mode() == MODE_EX ) {
	} else {
		//if( !mainWindow()->hasSearchBoxFocus() )
		//	return;
	}
	//	とりあえず毎回 SSSearch を初期化、もしパフォーマンス的に問題があれば将来的に改修
	SSSearch &sssrc = mainWindow()->sssrc();
	uint opt = mainWindow()->getSearchOpt();
	sssrc.setup((wchar_t *)pat.data(), pat.size(), opt);
	const auto rct = rect();
	int px, py = 0;
	for (int ln = m_scrollY0; ln < buffer()->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		px = m_lineNumAreaWidth;
		auto startIX = buffer()->lineStartPosition(ln);
		paintMatchedBG(pt, ln, py);
	}
}
void EditView::adjustPx1Px2(const int& lineStart, const int& last, const pos_t& pos, int& px1, int& px2, const int& matchLength)
{
	if (!m_preeditString.isEmpty() &&		//	IME 変換中
		m_textCursor->position() >= lineStart && m_textCursor->position() < last)	//	現行にカーソルがある場合
	{
		if (m_textCursor->position() <= pos) {
			px1 += m_preeditWidth;
			px2 += m_preeditWidth;
		}
		else if (m_textCursor->position() < pos + matchLength) {
			px2 += m_preeditWidth;
		}
	}
}
void EditView::paintMatchedBG(QPainter &pt, int vln, int py)
{
	SSSearch &sssrc = mainWindow()->sssrc();
	const int hv = m_scrollX0 * m_fontWidth;
	const int lineStart = viewLineStartPosition(vln);
	pos_t pos = lineStart;
	//const int last = lineStart + m_viewLineMgr->viewLineSize(vln);
	const int last = viewLineStartPosition(vln + 1);
	while( (pos = sssrc.strstr(*buffer(), pos, last)) >= 0 ) {
		//const int plen = sssrc.patSize();
		const int matchLength = sssrc.matchLength();
		int px1 = textWidth(lineStart, pos - lineStart /*, last*/) + m_lineNumAreaWidth;
		int px2 = textWidth(lineStart, pos + matchLength - lineStart /*, last*/) + m_lineNumAreaWidth;
		adjustPx1Px2(lineStart, last, pos, px1, px2, matchLength);
		pt.fillRect(QRect(px1 - hv, py, px2 - px1, lineHeight()),
							typeSettings()->color(TypeSettings::MATCHED_BG));
		++pos;
	}
}
void EditView::paintAssocParenBG(QPainter &pt)
{
	const auto rct = rect();
	int py = 0;
	for (int ln = m_scrollY0; ln < buffer()->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		paintAssocParenBG(pt, ln, py);
	}
}
//	m_openParenPos, m_closeParenPos が行vlnにある場合は背景を強調
//		m_unbalancedAssocParen は checkAssocParen() で設定される
void EditView::paintAssocParenBG(QPainter &painter, int vln, int py)
{
	if( m_openParenPos < 0 ) return;
	const int hv = m_scrollX0;
	pos_t ls = viewLineStartPosition(vln);
	pos_t nxls = viewLineStartPosition(vln + 1);
	QColor col = m_unbalancedAssocParen ? QColor("red") : typeSettings()->color(TypeSettings::MATCHE_PAREN_BG);
	if( m_openParenPos >= ls && m_openParenPos < nxls ) {
		int px1 = textWidth(ls, m_openParenPos - ls /*, nxls*/);
		int px2 = textWidth(ls, m_openParenPos - ls + 1 /*, nxls*/);
		painter.fillRect(QRect(px1 - hv + m_lineNumAreaWidth, py, px2 - px1, lineHeight()), col);
	}
	if( m_closeParenPos >= ls && m_closeParenPos < nxls ) {
		int px1 = textWidth(ls, m_closeParenPos - ls /*, nxls*/);
		int px2 = textWidth(ls, m_closeParenPos - ls + 1 /*, nxls*/);
		painter.fillRect(QRect(px1 - hv + m_lineNumAreaWidth, py, px2 - px1, lineHeight()), col);
	}
}
void EditView::paintSelection(QPainter& pt)
{
	if( !m_textCursor->hasSelection() ) return;
	auto rct = rect();
	const auto nLines = rct.height() / m_lineHeight;
	auto first = m_textCursor->selectionFirst();
	auto last = m_textCursor->selectionLast();
	int firstLn = viewLineMgr()->positionToViewLine(first);
	int lastLn = viewLineMgr()->positionToViewLine(last);
	if( firstLn > m_scrollY0 + nLines || lastLn < m_scrollY0 ) return;		//	画面外の場合
	auto firstLnPos = viewLineMgr()->viewLineStartPosition(firstLn);
	auto lastLnPos = viewLineMgr()->viewLineStartPosition(lastLn);
	int px1 = viewLineOffsetToPx(firstLn, first - firstLnPos) + m_lineNumAreaWidth - m_scrollX0*m_fontWidth;
	int px9 = viewLineOffsetToPx(lastLn, last - lastLnPos) + m_lineNumAreaWidth - m_scrollX0*m_fontWidth;
	int py1 = (firstLn - m_scrollY0) * m_lineHeight;		//	行上部座標
	pt.setBrush(typeSettings()->color(TypeSettings::SEL_BG));
	pt.setPen(Qt::transparent);
	if( firstLn == lastLn ) {	//	選択が１行内の場合
		QRect r(px1, py1, px9 - px1, m_lineHeight);
		pt.drawRect(r);
		return;
	}
	//	undone: クリッピング処理
	//	選択開始行
	int sz = viewLineMgr()->viewLineSize(firstLn);
	int px2 = viewLineOffsetToPx(firstLn, sz) + m_lineNumAreaWidth - m_scrollX0*m_fontWidth;
	QRect r(px1, py1, px2 - px1 + m_fontWidth*2, m_lineHeight);
	pt.drawRect(r);
	//	途中行
	py1 += m_lineHeight;
	for (int vln = firstLn + 1; vln < lastLn; ++vln, py1+=m_lineHeight) {
		int pos0 = viewLineMgr()->viewLineStartPosition(vln);
		int sz = viewLineMgr()->viewLineSize(vln);
		int px2 = viewLineOffsetToPx(vln, sz) + m_lineNumAreaWidth - m_scrollX0*m_fontWidth;
		QRect r(m_lineNumAreaWidth, py1, px2 - m_lineNumAreaWidth /*- m_scrollX0*m_fontWidth*/ + m_fontWidth*2, m_lineHeight);
		pt.drawRect(r);
	}
	//	選択修了行
	QRect r2(m_lineNumAreaWidth, py1, px9 - m_lineNumAreaWidth /*- m_scrollX0*m_fontWidth*/, m_lineHeight);
	pt.drawRect(r2);
}
void EditView::paintTextArea(QPainter& pt)
{
	if( m_preeditString.isEmpty() ) m_preeditWidth = 0;
	auto rct = rect();
	pt.setPen(Qt::black);
	pt.setOpacity(1.0);
	//pt.drawText(100, 100, "Hello");
	int px, py = 0 /*DRAW_Y_OFFSET*2*/;
	//bool inBlockComment = false;
	bool inBlockComment = (document()->lineFlags(m_scrollY0) & Buffer::LINEFLAG_IN_BLOCK_COMMENT) != 0;
	//bool inLineComment = false;
	QString quotedText;
	for (int ln = m_scrollY0; ln < buffer()->lineCount() && py < rct.height(); ++ln, py+=m_lineHeight) {
		bool inLineComment = false;		//	undone: 折返し行対応
		px = m_lineNumAreaWidth;
		auto startIX = buffer()->lineStartPosition(ln);
		auto lnsz = buffer()->lineSize(ln);
		paintLineText(pt, px, py+m_baseLineDY, ln, startIX, lnsz, startIX+lnsz, inBlockComment, inLineComment, quotedText);
		if( inBlockComment )
			document()->setLineFlag(ln+1, Buffer::LINEFLAG_IN_BLOCK_COMMENT);
		else
			document()->resetLineFlag(ln+1, Buffer::LINEFLAG_IN_BLOCK_COMMENT);
		if( !buffer()->isBlankEOFLine() && ln == buffer()->lineCount() - 1 ) {
			if( !m_preeditString.isEmpty() && ln == m_textCursor->viewLine() )
				px += m_preeditWidth;
			pt.setPen(typeSettings()->color(TypeSettings::EOF_MARK));
			pt.drawText(px, py+m_baseLineDY, "[EOF]");
		}
	}
	if( buffer()->isBlankEOFLine() ) {
		pt.setPen(typeSettings()->color(TypeSettings::EOF_MARK));
		auto px = m_lineNumAreaWidth;
		if( m_textCursor->viewLine() == EOFLine() && !m_preeditString.isEmpty() )
			px += m_preeditWidth;
		pt.drawText(px, py+m_baseLineDY, "[EOF]");
	}
}
//	１行表示
//
//			
void EditView::paintLineText(QPainter &pt,
							int &px,
							int py,			//	ベースライン位置（行Top + m_fontHeight）
							int ln,			//	論理行番号, 0 org
							pos_t ls,			//	表示行先頭位置
							int vlnsz,		//	表示行サイズ
							pos_t nxdls,		//	次の論理行先頭位置
							bool &inBlockComment,
							bool &inLineComment,
							QString &quotedText)
{
	pt.setFont(m_font);
	int pxLimit = rect().width() - MINMAP_WIDTH + m_scrollX0*m_fontWidth;
	QFontMetrics fm(m_font);
	//QFontMetrics fmBold(m_fontBold);
	QFontMetrics fmMB(m_fontMB);
	const auto descent = fm.descent();
	const auto chWidth = m_fontWidth;		//fm.width(QString("8"));
	int nTab = typeSettings()->intValue(TypeSettings::TAB_WIDTH);
	int ix = 0;
	int clmn = 0;
	const int last = ls + vlnsz;
	const QString lineComment = typeSettings()->textValue(TypeSettings::LINE_COMMENT);
	//const QString lineComment = "//";		//	暫定コード
	int curpos = 0;
	if( !m_preeditString.isEmpty() && ln == m_textCursor->viewLine() )
		curpos = m_textCursor->position();
	ViewTokenizer tkn(typeSettings(), buffer(), ls, vlnsz, nxdls /*, curpos*/);
	if( ln == m_textCursor->viewLine() )	//	undone: 折返しモード対応
		tkn.setCursorLine();
	tkn.setInLineComment(inLineComment);
	tkn.setInBlockComment(inBlockComment);
	QString token = tkn.nextToken();
	int peDX = 0;
	QString nextToken;
	while( !token.isEmpty() ) {
		if( px >= pxLimit ) {		//	画面右に出た場合
			while( !token.isEmpty() ) {
				if( inBlockComment ) {
					if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_END) )
						inBlockComment = false;
				} else {
					if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_BEG) )
						inBlockComment = true;
				}
				token = tkn.nextToken();
			}
			break;
		}
		if( !m_preeditString.isEmpty() && ln == m_textCursor->viewLine() ) {
			if( tkn.tokenix() < m_textCursor->position() && tkn.ix() > m_textCursor->position() ) {
				nextToken = token.right(tkn.ix() - m_textCursor->position());
				token = token.left(m_textCursor->position() - tkn.tokenix());
			} else if( tkn.tokenix() >= m_textCursor->position() )
				peDX = m_preeditWidth;
		}
		if( tkn.tokenix() + token.size() > last )
			token = token.left(last - tkn.tokenix());
		//qDebug() << "type = " << tkn.tokenType() << ", token = " << token;
		bool bold = false;
		//pt.setFont(m_font);
		QColor col = typeSettings()->color(inBlockComment || inLineComment ? TypeSettings::COMMENT : TypeSettings::TEXT);
		auto tabwd = 0;	//fm.width(token);
		if( token == "\t" ) {
			token = ">";
			col = typeSettings()->color(TypeSettings::TAB);
			int clmn = (px - m_lineNumAreaWidth) / chWidth;
			tabwd = (nTab - (clmn % nTab)) * chWidth;
		} else {
			if( inLineComment ) {
				col = typeSettings()->color(TypeSettings::COMMENT);
			} else if( inBlockComment ) {
				col = typeSettings()->color(TypeSettings::COMMENT);
				if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_END) ) {
					inBlockComment = false;
				}
			} else {
				switch( tkn.tokenType() ) {
				case ViewTokenizer::ALNUM:
					if( typeSettings()->isKeyWord1(token) ) {
						col = typeSettings()->color(TypeSettings::KEYWORD1);
						bold = typeSettings()->boolValue(TypeSettings::KEYWORD1_BOLD);
					} else if( typeSettings()->isKeyWord2(token) ) {
						col = typeSettings()->color(TypeSettings::KEYWORD2);
						bold = typeSettings()->boolValue(TypeSettings::KEYWORD2_BOLD);
					}
					break;
				case ViewTokenizer::DIGITS:
					if( !inLineComment && !inBlockComment )
						col = typeSettings()->color(TypeSettings::DIGITS);
					break;
				case ViewTokenizer::QUOTED:
					if( !inLineComment && !inBlockComment )
						col = typeSettings()->color(TypeSettings::STRING);
					break;
				case ViewTokenizer::ZEN_SPACE:
					token = QChar(L'□');
					col = typeSettings()->color(TypeSettings::ZEN_SPACE);
					break;
				case ViewTokenizer::CTRL:
#if	0
					if( token == "\t" ) {
						token = ">";
						col = typeSettings()->color(TypeSettings::TAB);
						int clmn = (px - m_lineNumAreaWidth) / chWidth;
						tabwd = (nTab - (clmn % nTab)) * chWidth;
					}
#endif
					break;
				case ViewTokenizer::NEWLINE:
					col = typeSettings()->color(TypeSettings::NEWLINE);
					break;
				case ViewTokenizer::HTML_SPECIAL_CHARS:
					col = typeSettings()->color(TypeSettings::EOF_MARK);		//	暫定コード
					break;
				case ViewTokenizer::COMMENT:
					//if( !inBlockComment ) {
						if( token == typeSettings()->textValue(TypeSettings::BLOCK_COMMENT_BEG) ) {
							col = typeSettings()->color(TypeSettings::COMMENT);
							inBlockComment = true;
						} else if( token == typeSettings()->textValue(TypeSettings::LINE_COMMENT) ) {
							inLineComment = true;
							col = typeSettings()->color(TypeSettings::COMMENT);
						}
					//}
					break;
				}
			}
		}
		if( !token.isEmpty() ) {
			auto tlast = tkn.tokenix() + token.size();
			if( !m_textCursor->hasSelection() ||		//	非選択状態
				tkn.tokenix() >= m_textCursor->selectionLast() ||		//	選択範囲より後ろ
				tlast <= m_textCursor->selectionFirst() )			//	選択範囲より前
			{
				pt.setPen(col);
				px += paintTokenText(pt, token, clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
			} else {	//	選択状態にかかっている場合
				//if( tkn.tokenix() >= m_textCursor->selectionFirst() && tkn.ix() < m_textCursor->selectionLast() ) {
				//	//	token 全体が選択状態の場合
				//	pt.setPen(typeSettings()->color(TypeSettings::SEL_TEXT));
				//	px += paintTokenText(pt, token, clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
				//} else
				//{
					if( tkn.tokenix() < m_textCursor->selectionFirst() ) {	//	途中から選択状態の場合
						int len = m_textCursor->selectionFirst() - tkn.tokenix();
						if( m_textCursor->selectionLast() < tlast ) {		//	途中まで選択状態の場合
							pt.setPen(col);
							px += paintTokenText(pt, token.left(len), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
							int len2 = m_textCursor->selectionLast() - m_textCursor->selectionFirst();
							pt.setPen(typeSettings()->color(TypeSettings::SEL_TEXT));
							//auto t1 = token.mid(len, len2);
							px += paintTokenText(pt, token.mid(len, len2), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
							pt.setPen(col);
							//auto t2 = token.mid(len+len2);
							px += paintTokenText(pt, token.mid(len+len2), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
						} else {
							pt.setPen(col);
							px += paintTokenText(pt, token.left(len), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
							pt.setPen(typeSettings()->color(TypeSettings::SEL_TEXT));
							px += paintTokenText(pt, token.mid(len), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
						}
					} else {		//	最初から選択状態の場合
						if( m_textCursor->selectionLast() < tlast ) {		//	途中まで選択状態の場合
							pt.setPen(typeSettings()->color(TypeSettings::SEL_TEXT));
							int len = m_textCursor->selectionLast() - tkn.tokenix();
							px += paintTokenText(pt, token.left(len), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
							pt.setPen(col);
							px += paintTokenText(pt, token.mid(len), clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
						} else {
							//	token 全体が選択状態の場合
							pt.setPen(typeSettings()->color(TypeSettings::SEL_TEXT));
							px += paintTokenText(pt, token, clmn, px, py, peDX, tabwd, chWidth, descent, col, bold);
						}
					}
				//}
			}
			if (tkn.tokenType() == ViewTokenizer::HTML_SPECIAL_CHARS) {
				px += (tkn.m_orgText.size() - 1) * chWidth;
			}
		}
		//
		if( !nextToken.isEmpty() ) {
			token = nextToken;
			nextToken.clear();
			tkn.m_tokenix = m_textCursor->position();
		} else
			token = tkn.nextToken();
	}
}
int EditView::paintTokenText(QPainter& pt,
								QString& token,
								int& clmn,
								int& px,
								int py,			//	ベースライン位置
								int peDX,		//	IME変換候補表示のためのX座標差分
								int tabwd,
								const int chWidth,
								const int descent,		//	フォントのベースライン下高さ
								QColor& col,
								bool bold)
{
	int wd = 0;	 //tabwd;
	int sx = m_scrollX0 * m_fontWidth;
	//pt.setPen(col);
#if	1
	auto x = px + peDX;
	for (int i = 0; i != token.size();) {
		if (token[i] < 0x100) {
			QString txt = token[i++];
			while( i != token.size() && token[i] < 0x100 ) txt += token[i++];
			pt.drawText(x - sx, py, txt);
			if( bold )
				pt.drawText(x - sx + 1, py, txt);
			x += chWidth * txt.size();
			wd += chWidth * txt.size();
		} else {
			QString txt = token[i];
			int w = 2;
			if( isSrgtPirFirstChar(token[i]) && isSrgtPirSecondChar(token[i+1]) ) {		//	サロゲートペア
				txt += token[++i];
				w = 4;
			}
			++i;
			if( txt[0] == L'　' ) {
				txt[0] = L'□';
				pt.setPen(typeSettings()->color(TypeSettings::ZEN_SPACE));
			}
			pt.drawText(x - sx, py + descent - m_lineHeight, chWidth * w, m_lineHeight, Qt::AlignHCenter | Qt::AlignBottom, txt);
			if( bold )
				pt.drawText(x - sx + 1, py + descent - m_lineHeight, chWidth * w, m_lineHeight, Qt::AlignHCenter | Qt::AlignBottom, txt);
			x += chWidth * w;
			wd += chWidth * w;
			pt.setPen(col);
		}
	}
#else
	if (token[0] < 0x100) {
		//pt.setFont(m_font);
		pt.drawText(px + peDX - sx, py, token);
		if( bold )
			pt.drawText(px + peDX - sx + 1, py, token);
		wd = chWidth * token.size();
	} else {
		auto x = px + peDX;
		wd = 0;
		for (int i = 0; i != token.size(); ++i) {
			QString txt = token[i];
			int w = 2;
			if( isSrgtPirFirstChar(token[i]) && isSrgtPirSecondChar(token[i+1]) ) {
				txt += token[++i];
				w = 4;
			}
			if (txt == " ") {
				x += chWidth;
				wd += chWidth;
			}
			else {
				pt.drawText(x - sx, py + descent - m_lineHeight, chWidth * w, m_lineHeight, Qt::AlignHCenter | Qt::AlignBottom, txt);
				if( bold )
					pt.drawText(x - sx + 1, py + descent - m_lineHeight, chWidth * w, m_lineHeight, Qt::AlignHCenter | Qt::AlignBottom, txt);
				x += chWidth * w;
				wd += chWidth * w;
			}
		}
	}
#endif
	return !tabwd ? wd : tabwd;
}
void EditView::paintMinMap(QPainter& pt)
{
	//	全体マップ QPixmap 作成後に編集されている場合
	if( buffer()->seqNumber() > document()->mmSeqNumber() )
		document()->buildMinMap();
	//
	QPixmap& minMap = document()->minMap();
	auto rct = rect();
	int nLines = rct.height() / m_lineHeight;
	int px = rct.width() - minMap.width();
	int py = 0;
	//
	rct.setX(rct.width() - MINMAP_WIDTH);
	rct.setWidth(MINMAP_WIDTH);
	//pt.setBrush(QColor("lightgray"));
	pt.setBrush(typeSettings()->color(TypeSettings::BACK_GROUND));
	pt.setPen(Qt::transparent);
	pt.drawRect(rct);		//	背景（テキスト背景と同一）描画
	//
	pt.setOpacity(0.5);
	//	undone: minMap 高さがビュー高さより高い場合は、縮小表示
	double scale = 1.0;
	if( minMap.height() <= rct.height() ) {	//	ミニマップが全部表示できる場合
		pt.drawPixmap(px, py, minMap);		//	テキストPixmap描画
	} else {
		scale = (double)rct.height() / minMap.height();
		pt.drawPixmap(rct, minMap, minMap.rect());
	}
	//
	pt.setOpacity(0.1);
	pt.setBrush(Qt::black);
	if( m_scrollY0 != 0 ) {
		rct.setHeight(m_scrollY0*scale);
		pt.drawRect(rct);			//	現エリアより上部（前）背景描画
	}
	if( minMap.height() - (m_scrollY0+nLines) > 0 ) {
		rct.setY((m_scrollY0+nLines)*scale);
		rct.setHeight((minMap.height() - (m_scrollY0+nLines))*scale);
		pt.drawRect(rct);			//	現エリアより下部（後）背景描画
	}
	pt.setOpacity(1.0);
	//	検索マッチ部分強調
	if( !mainWindow()->findString().isEmpty() ) {
		pt.setPen(typeSettings()->color(TypeSettings::MATCHED_BG));
		SSSearch &sssrc = mainWindow()->sssrc();
		pos_t pos = 0;
		const int last = document()->size();
		while( (pos = sssrc.strstr(*buffer(), pos, last)) >= 0 ) {
			const int matchLength = sssrc.matchLength();
			int ln = document()->positionToLine(pos);
			auto lineStartPos = document()->lineStartPosition(ln);
			auto px1 = px + textWidth(lineStartPos, pos - lineStartPos) / m_fontWidth;
			auto px2 = px + textWidth(lineStartPos, pos - lineStartPos + matchLength) / m_fontWidth;
			int py = ln * scale;
			pt.drawLine(px1, py, px2, py);
			pos += matchLength;
		}
	}
	//
	rct.setY(m_scrollY0*scale);
	rct.setHeight(nLines*scale);
	pt.setBrush(Qt::transparent);
	pt.setPen(Qt::red);
	pt.drawRect(rct);				//	現エリアに赤枠描画
}
#if	0
void EditView::drawPreeditBG(QPainter&)
{
}
#endif
void EditView::paintPreeditString(QPainter&pt)
{
	if( m_preeditString.isEmpty() ) {
		m_preeditWidth = 0;
		return;
	}
	QFontMetrics fm(m_font);
	pt.setOpacity(1.0);
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	int py = (vln - m_scrollY0) * lineHeight() /*+ DRAW_Y_OFFSET*/;		//	行上部位置
	QRect rct = rect();
	if( py < 0 || py >= rct.height() )
		return;		//	画面外の場合
	int hv = 0;		//horizontalScrollBar()->value();
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) + m_lineNumAreaWidth;
	py -= m_scrollX0 * m_fontWidth;
	int ht = fm.ascent();
	//const auto descent = fm.descent();
	m_preeditWidth = fm.width(m_preeditString);
	//	背景描画
	QRect r(px, py, m_preeditWidth, m_lineHeight);
	pt.setPen(Qt::transparent);
	pt.setBrush(typeSettings()->color(TypeSettings::PREEDIT_BG));
	pt.drawRect(r);
	//	変換中テキスト描画
	pt.setPen(typeSettings()->color(TypeSettings::TEXT));
	pt.drawText(px, py+m_baseLineDY /*-m_descent*/, m_preeditString);
}
//	行カーソル表示
void EditView::paintLineCursor(QPainter &pt)
{
	if( !typeSettings()->boolValue(TypeSettings::LINE_CURSOR) ) return;
	int vln = m_textCursor->viewLine();
	int py = (vln - m_scrollY0) * lineHeight() + m_baseLineDY + m_fontDescent /*+ DRAW_Y_OFFSET*2*/;
	//py += fontHeight();
	QRect rct = rect();
	if( py >= 0 && py < rct.height() ) {
		//QPixmap wholeMap = document()->wholeMap();
		pt.setPen(typeSettings()->color(TypeSettings::LINE_CURSOR));
		int wd = rct.width();
		//if( globSettings()->boolValue(GlobalSettings::WHOLE_MAP) && py <= wholeMap.height() )
			wd -= MINMAP_WIDTH;
		pt.drawLine(0, py, wd, py);
	}
}
void EditView::paintCursor(QPainter& pt)
{
	if( !m_dispCursor ) return;
	pt.setOpacity(1.0);
	pos_t pos = m_textCursor->position();
	int vln = m_textCursor->viewLine();
	int offset;
	int dln = viewLineToDocLine(vln, offset);
	int py = (vln - m_scrollY0) * lineHeight() + m_baseLineDY + m_fontDescent - m_fontHeight;
	QRect rct = rect();
	if( py < 0 || py >= rct.height() )
		return;		//	画面外の場合
	int hv = m_scrollX0 * m_fontWidth;
	int px = viewLineOffsetToPx(vln, pos - viewLineStartPosition(vln)) + m_lineNumAreaWidth;
	if( !m_preeditString.isEmpty() ) px += m_preeditWidth;
	//int ht = QFontMetrics(m_font).ascent();
	int wd = m_fontWidth;
	if( pos < document()->size() && charAt(pos) >= 0x100 ) {	//	手抜き判定
		wd *= 2;
	}
	const auto mode = mainWindow()->mode();
	switch( mode ) {
	case MODE_INS:
		pt.fillRect(QRect(px - hv, py, CURSOR_WD, m_fontHeight),
							typeSettings()->color(TypeSettings::CURSOR));
		break;
	case MODE_REP:
		pt.fillRect(QRect(px - hv, py, wd, m_fontHeight),
							typeSettings()->color(TypeSettings::CURSOR));
		break;
	case MODE_VI:
		pt.fillRect(QRect(px - hv, py+m_fontHeight/2, wd, m_fontHeight/2),
							typeSettings()->color(TypeSettings::CURSOR));
		break;
	}
}
