#pragma once

#include <QString>

typedef unsigned char byte;
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号

class EditView;

class TextCursor {
public:
	enum {
		LEFT = 0,
		RIGHT,
		UP,
		DOWN,
		PREV_WORD,		//	前の単語先頭へ
		BEG_WORD,		//	単語先頭
		END_WORD,		//	単語末尾
		NEXT_WORD,		//	次の単語先頭へ
		NEXT_WORD_NOSKIPSPC,		//	次の単語先頭へ（継続空白類をスキップしない）
		NEXT_CAP_WORD,		//	次の単語先頭（キャピタルワード、アンダーバー区切り）へ
		PREV_SS_WORD,		//	前の空白区切り単語先頭へ
		END_SS_WORD,			//	空白区切り単語末尾
		NEXT_SS_WORD,		//	次の空白区切り単語先頭へ
		NEXT_SS_WORD_NOSKIPSPC,		//	次の空白区切り単語先頭へ（継続空白類をスキップしない）
		HOME_LINE,		//	最初の非空白文字位置 or 行先頭
		BEG_LINE,
		FIRST_NOSPACE,		//	最初の非空白文字
		LAST_CHAR_LINE,		//	行の最後の文字
		END_LINE,
		PREV_HEADING_LINE,
		NEXT_HEADING_LINE,
		PREV_BLANK_LINE,
		NEXT_BLANK_LINE,
		BEG_OF_CUR_SECTION,		//	[[
		BEG_OF_NEXT_SECTION,	//	]]
		BEG_DOC,
		END_DOC,
		ASSOC_PAREN,	//	対応する括弧へ移動

		MOVE_ANCHOR = 0,
		KEEP_ANCHOR,
		
		NOMAL_MODE = 0,			//	通常選択モード
		VI_CHAR_SEL_MODE,		//	vi の v モード
		VI_LINE_SEL_MODE,		//	vi の v モード
		BOX_MODE,					//	矩形選択モード
	};

public:
	TextCursor(EditView *view, pos_t pos = 0, int anchor= 0);
	TextCursor(const TextCursor &x);
public:
	byte		mode() const { return m_mode; }
	int		position() const { return m_pos; }
	int		positionInLine() const;	//	行内オフセットを返す
	int		anchor() const { return m_anchor; }
	int		viewLine() const { return m_viewLine; }
	void	movePosition(int op, int mode = MOVE_ANCHOR, int n = 1, bool vi = false);
	wchar_t	charAt() const;
	wchar_t	charAt(int pos) const;
	int		selectionSize() const;
	int		selectionFirst() const;
	int		selectionLast() const;
	int		selectionFirstLine() const;
	int		selectionLastLine() const;
	bool	hasSelection() const { return m_mode != NOMAL_MODE || m_anchor != m_pos; }
	QString	selectedText() const;
	bool	getSelectedLineRange(int &dln1, int &dln2) const;
public:
	void	setMode(byte mode);
	void	clearSelection();
	void	setPosition(pos_t pos, int mode = MOVE_ANCHOR);
	void	setLineAndPosition(int vln, pos_t pos, int mode = MOVE_ANCHOR);
	void	setWordBegPos() { m_wordBegPos = m_pos; }
	void	setWordEndPos() { m_wordEndPos = m_pos; }
	void	setAnchorWordBeg() { m_anchor = m_wordBegPos; }
	void	setAnchorWordEnd() { m_anchor = m_wordEndPos; }
	void	setPX(int px) { m_px = px; }
	void	deleteChar(bool BS = false, bool vi = false);
	void	deletePrevChar(bool vi = false);
	void	insertText(const QString &);
protected:
	uchar	getCharType(wchar_t &);
	int		nextWord(int n, bool cw = false);
	int		nextCapWord(int n);
	int		prevWord(int n);
	int		begWord();
	int		endWord(int n);
	int		nextSSWord(int n, bool cw = false);
	int		prevSSWord(int n);
	int		endSSWord(int n);
	int		viewLineStartPosition(int vln) const;

private:
	EditView	*m_view;
	byte		m_mode;
	int		m_pos;
	int		m_anchor;
	int		m_wordBegPos;			//	単語先頭位置 for マウスダブルクリック単語単位選択
	int		m_wordEndPos;			//	単語末尾位置 for マウスダブルクリック単語単位選択
	int		m_viewLine;			//	表示行番号 [0, viewLineCount())
	int		m_anchorViewLine;			//	表示行番号 [0, viewLineCount())
	int		m_px;			//	保存位置
	int		m_boxCurPx1;
	int		m_boxCurPx2;
	int		m_boxCurLine;				//	表示行 [0, viewLineCount())
	int		m_boxAnchorPx1;
	int		m_boxAnchorPx2;
	int		m_boxAnchorLine;		//	表示行 [0, viewLineCount())
};
