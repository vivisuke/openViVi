#ifndef VIENGINE_H
#define VIENGINE_H

#include <QObject>
#include <vector>

class EditView;

typedef unsigned char uchar;
typedef unsigned char byte;
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号

struct LineNumbers
{
public:
	LineNumbers()
		: m_first(0)
		, m_second(0)
		, m_nLines(0)
		, m_curLine(0)
		, m_lineCount(0)
		, m_dest(0)
	{}
	
public:
	int	m_first;
	int	m_second;
	int	m_nLines;
	int	m_curLine;
	int	m_lineCount;
	int	m_dest;
};

//	モード
namespace Mode {
	enum {
		INSERT = 0,		//	挿入モード
		REPLACE,			//	上書きモード
		COMMAND,		//	vi コマンドモード
		CMDLINE,			//	: / コマンド入力モード
	};
};
//	確定したコマンド
namespace ViCmd {
	enum {
		UNDETERMINED = 0,		//	コマンド未確定状態
		INSERT,							//	文字挿入
		TO_COMMAND_MODE,
		TO_INSERT_MODE,
		OPEN_NEXT_LINE,
		OPEN_PREV_LINE,
		CUR_LEFT,
		CUR_RIGHT,
		CUR_UP,
		CUR_DOWN,
		CUR_CAP_WORD,		//	cdy u
		CUR_NEXT_WORD,		//	w
		CUR_PREV_WORD,		//	b
		CUR_END_WORD,		//	e
		CUR_NEXT_SS_WORD,		//	W
		CUR_PREV_SS_WORD,		//	B
		CUR_END_SS_WORD,		//	E
		CUR_BEG_LINE,			//	0 行先頭
		CUR_NTH_COLUMN,			//	指定カラム位置へ移動
		CUR_FIRST_NOSPACE,		//	^ 行の最初の非空白文字
		CUR_LAST_CHAR_LINE,		//	行の最後の文字へ
		CUR_END_LINE,
		CUR_NEXT_FNS_LINE,		//	次行先頭非空白文字
		CUR_PREV_FNS_LINE,		//	前行先頭非空白文字
		CUR_TOP_OF_SCREEN,
		CUR_MIDDLE_OF_SCREEN,
		CUR_BOTTOM_OF_SCREEN,
		CUR_ASSOC_PAREN,
		CUR_PREV_BLANK_LINE,		//	{
		CUR_NEXT_BLANK_LINE,	//	}
		BEG_OF_CUR_SECTION,		//	[[
		BEG_OF_NEXT_SECTION,	//	]]
		CUR_BEG_DOC,
		CUR_END_DOC,
		CUR_JUMP_LINE,
		SEARCH_PAT,				//	/? 検索、パターンは m_searchPat, 方向は m_searchFwd に格納
		SEARCH_CUR_WORD,		//	*	カーソル位置単語検索
		SEARCH_NEXT,				//	n
		SEARCH_PREV,				//	N
		SEARCH_CHAR_f,
		SEARCH_CHAR_F,
		SEARCH_CHAR_t,
		SEARCH_CHAR_T,
		SCROLL_CUR_AT_TOP,			//	z Enter
		SCROLL_CUR_IN_MIDDLE,		//	z.
		SCROLL_CUR_AT_BOTTOM,		//	z-
		SELECT_CHAR_MODE,		//	v mode
		SELECT_LINE,				//	V
		TEXT_OBJECT_I,
		TEXT_OBJECT_A,
		DELETE_CHAR,				//	文字削除
		DELETE_PREV_CHAR,		//	前文字削除
		DELETE_TO_EOL,			//	行待つまで削除
		DELETE_LINE,				//	行削除
		REPLACE_CHAR,				//	1文字置換
		YANK_TEXT,				//	選択範囲をヤンク
		YANK_LINE,				//	行ヤンク
		PASTE_PREV,				//	P コマンド
		PASTE_NEXT,				//	p コマンド
		EXEC_YANK_TEXT,		//	@@
		SHIFT_LEFT,
		SHIFT_RIGHT,
		TAB_SHIFT,			//	選択状態でTAB
		JOIN_LINES,			//	J
		TOGGLE_UPPER_LOWER,
		TOGGLE_TRUE_FALSE,
		INCREMENT,			//	+= 1
		DECREMENT,			//	-= 1
		UNDO,
		REDO,
		OPEN_UNDO_BLOCK,
		CLOSE_UNDO_BLOCK,
		CLOSE_ALL_UNDO_BLOCK,
		MARK,						//	マーク、マーク文字は lastCmdChar で指定
		JUMP_MARK_POS,		//	`
		JUMP_MARK_LINE,		//	'
		SAVE_ALL_EXIT,		//	ZZ
		
		EX_DELETE_LINE,		//	:d[elete]
		EX_MOVE_LINE,			//	:mo[ve]
		EX_COPY_LINE,			//	:co[py]
		EX_SHIFT_LEFT,			//	:<
		EX_SHIFT_RIGHT,		//	:>
		
	};
};

struct YankBufferItem
{
public:
	YankBufferItem(bool bLine = false, QString text = QString())
		: m_bLine(bLine)
		, m_text(text)
	{
	}
	YankBufferItem(const YankBufferItem &x)
		: m_bLine(x.m_bLine)
		, m_text(x.m_text)
	{
	}

public:
	YankBufferItem &operator=(const YankBufferItem &x)
	{
		m_bLine = x.m_bLine;
		m_text = x.m_text;
	}
	
public:
	bool		m_bLine;		//	行単位
	QString	m_text;
};

class ViEngine : public QObject
{
	Q_OBJECT
	
public:
	enum {
		N_CMD_HIST = 100,
		N_YANK_BUFFER = 10 + 26,		//	0-9, a-z	（大文字小文字同一視）
	};

public:
	ViEngine(QObject *parent = 0);
	~ViEngine();

public:
	byte	mode() const { return m_mode; }
	byte	prevMode() const { return m_prevMode; }
	char	vMode() const { return m_vMode; }
	int	cmd() const { return m_cmd; }
	int	subMode() const { return m_subMode; }
	int	repeatCount() const { return (!m_repeatCount ? 1 : m_repeatCount) * m_cdyRepCount; }
	//int	repeatCount() const { return (!m_repeatCount ? 1 : m_repeatCount) * m_redoCount; }
	bool	isRedoRecording() const { return m_redoRecording; }
	bool	isPathNamePrinted() const { return m_pathNamePrinted; }
	bool	isUndoBlockOpened() const { return m_undoBlockOpened; }
	bool	redoing() const { return m_redoing; }
	const QString cmdText() const { return m_cmdText; }
	QChar lastCmdChar() const { return m_cmdText.isEmpty() ? QChar() : m_cmdText[m_cmdText.size() - 1]; }
	const QString yankText(bool &) const;
	wchar_t	lastFindChar() const { return m_lastFindChar; }
	wchar_t	cdy() const { return m_cdy; }
	wchar_t	textObjectChar() const { return m_textObjectChar; }
	QChar		cmdLineChar() const { return m_cmdLineChar; }
	int			incSearchPos() const { return m_incSearchPos; }
	EditView		*incSearchView() const { return m_incSearchView; }
	const YankBufferItem *yankBufferItem() const;
	const QString &searchPat(bool &fwd) { fwd = m_searchFwd; return m_searchPat; }
	QString insertedText() const { return m_insertedText; }
	bool	globDoing() const { return m_globalDoing; }
	
public:
	void	setMode(byte mode);
	void	setVMode(char ch) { m_vMode = ch; }
	void	clearVMode() { m_vMode = 0; }
	void	resetStatus(bool = true);
	void	setToInsertMode(bool b) { m_toInsertMode = b; }
	void	setCmd(wchar_t cmd) { m_cmd = cmd; }
	void	setCmdText(const QString &cmd) { m_cmdText = cmd; }
	void	setCmdLineChar(QChar ch) { m_cmdLineChar = ch; }
	void	setPrevMode(byte);
	void	popMode();			//	モードを元に戻す
	void	doViCommand(const QString &cmd) { processCommandText(cmd); }
	void	doViCommand(wchar_t ch) { processCommand(ch); }
	void	processCommandText(const QString &, bool hasSelection = false);
	void	processCommand(wchar_t, bool hasSelection = false);
	void	imeOpenStatusChanged();
	void	setYankText(const QString &text, bool bLine);
	void	resetRedoRecording() { m_redoRecording = false; }
	void	appendInsertedText(const QString &);
	void	removeFromInsertedText(const QString &);
	void	onBackSpace();		//	挿入文字列アップデート
	void	setIncSearchViewPos(EditView *view, pos_t pos);
	void	clearLineNumbers();
	LineNumbers &lns() { return m_lns; }
	YankBufferItem *yankBufferItem();
	void	doCmd();
	void	setPathNamePrinted(bool b) { m_pathNamePrinted = b; }
	void setUndoBlockOpened(bool b) { m_undoBlockOpened = b; }
	void	doFind(const QString &pat, bool forward);
	void	setLastSubstCmd(const QString &t) { m_lastSubstCmd = t; }
	void	setGlobDoing(bool b) { m_globalDoing = b; }
	void	setView(EditView *view) { m_view = view; }
	EditView *view() { return m_view; }

protected:
	void	viCommand(wchar_t, bool hasSelection);
	void 	processSubMode(wchar_t ch);
	void	openUndoBlock();

signals:
	void	modeChanged();			//	モード変化
	void	insertText(QString);		//	文字列挿入
	void	replaceText(QString);	//	文字列上書き
	void	clearSelection();			//	選択解除
	void	cmdFixed();				//	コマンドが確定
	void	doExCommand(QString);

private:
	bool		m_noInsModeAtImeOpenStatus;
	byte		m_mode;
	byte		m_prevMode;			//	直前のモード
	char		m_vMode;				//	{ v | V }
	wchar_t	m_cdy;				//	{c|d|y}
	wchar_t	m_subMode;		//	{c|d|g|y|z|>|<|#|[|]}
	wchar_t	m_lastFindChar;	//	fFtT 検索文字
	wchar_t	m_textObjectChar;		//	" ' ( { [ ...
	int		m_lastFindCmd;		//	f | F | t | T
	int		m_cmd;					//	確定したコマンド
	int		m_repeatCount;		//	繰り返し回数
	int		m_cdyRepCount;		//	cdy 指定以前に指定されて繰り返し回数
	int		m_lastRepeatCount;	//	実行済みコマンド指定時回数
	//int		m_redoCount;		//	. 実行時に指定された回数
	QString	m_cmdText;			//	コマンドテキスト
	QChar	m_cmdLineChar;		//	 ':' | '/' | '?'
	//QString	m_lastCmdText;		//	最後に実行したコマンド for .
	bool		m_moved;
	bool		m_editCmd;
	bool		m_toInsertMode;
	bool		m_redoRecording;			//  .(dot) のために挿入文字列記録中
	bool		m_redoing;						//  .(dot) 実行中フラグ
	bool		m_pathNamePrinted;
	bool		m_undoBlockOpened;		//	
	bool		m_globalDoing;				//	:g, :v コマンド実行中
	int		m_redoRepeatCount;
	QString	m_redoCmd;              //  .(dot) で再実行するコマンド
	QString	m_lastSubstCmd;		//	最後に実行された subst コマンド
	QString	m_insertedText;         //  挿入されたテキスト
	char		m_yankBufferChar;		//	'0'-'9' 'a'-'z'
	YankBufferItem	m_yankBuffer;		//	無名ヤンクバッファ
	YankBufferItem	m_namedYB[N_YANK_BUFFER];		//	名前付きヤンクバッファ
	//std::vector<QString>	m_exCmdHist;		//	ex コマンド履歴
	int		m_incSearchPos;			//	インクリメンタルサーチ前カーソル位置
	EditView	*m_view;						//	vi コマンド対象ビュー
	EditView	*m_incSearchView;			//	インクリメンタルサーチ中ビュー
	QString	m_searchPat;						//	検索文字列
	bool		m_searchFwd;			//	検索方向
	
#if	1
	LineNumbers	m_lns;		//	ex コマンド行番号情報
#else
	int		m_curLine;			//	現在行、1 org
	int		m_nLines;
	int		m_first;
	int		m_second;
#endif
	
};

#endif // VIENGINE_H
