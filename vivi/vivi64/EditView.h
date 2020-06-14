#pragma once

#include <QScrollArea>
#include <QFont>
#include <QTimer>
#include "TextCursor.h"
#include "../buffer/Buffer.h"
//#include "typeSettings.h"
//#include "globalSettings.h"
class Buffer;
class ViewLineMgr;
class Document;
class TextCursor;
class TypeSettings;
struct MarkItem;
class SSSearch;
class MainWindow;
class AutoCompletionDlg;
class GlobalSettings;
class ViEngine;

struct FallingChar		//	落下中文字
{
public:
	FallingChar(QString txt, QPointF pnt, QPointF v)
		: m_text(txt)
		//, m_ch(ch)
		, m_pnt(pnt)
		, m_v(v)
		{}
#if	0
	FallingChar(QChar ch, QPointF pnt, QPointF v)
		: m_text(ch)
		, m_ch(ch)
		, m_pnt(pnt)
		, m_v(v)
		{}
#endif

public:
	//QChar	m_ch;
	QString	m_text;
	QPointF	m_pnt;		//	位置
	QPointF	m_v;		//	速度ベクター
};

class EditView : public QWidget		//QScrollArea
{
	Q_OBJECT
public:
	EditView(MainWindow*, Document *doc /*, TypeSettings* = nullptr*/);
	~EditView();
public:
	const MainWindow	*mainWindow() const { return m_mainWindow; }
	bool	isBoxSelectMode() const { return false; }
	bool	isKeisenMode() const;
	wchar_t	charAt(pos_t pos) const;
	size_t	bufferSize() const;
	int		lineCount() const;
	int		viewLineCount() const;
	pos_t	cursorPosition() const;
	int		cursorLine() const;		//	[0, EOFLine]
	int		EOFLine() const;
	QString	fullPathName() const;
	QString	title() const;
	QString	typeName() const;
	QByteArray	codecName() const;
	TypeSettings	*typeSettings();	// { return m_typeSettings; }
	const TypeSettings	*typeSettings() const;	// { return m_typeSettings; }
	//const TypeSettings* cTypeSettings() { return (const TypeSettings*)m_typeSettings; }
	bool	isModified() const;		// { return m_modified; }
	bool	hasSelection() const;
	bool	hasSelectionInALine() const;
	QString	text(pos_t pos, ssize_t sz) const;
	QString	selectedText() const;
	QString	newLineText() const;
	int	viewLineOffsetToPx(int vln, int offset) const;
	int		offsetToPx(int ln, int offset) const;
	const Document*	document() const { return m_document; }
	const Buffer	*buffer() const { return m_buffer; }
	int		positionToLine(pos_t pos) const;
	int		docLineToViewLine(int dln) const;
	int		viewLineToDocLine(int vln) const;
	int		viewLineToDocLine(int vln, int &offset) const;
	int		viewLineStartPosition(int vln) const;
	const ViewLineMgr *viewLineMgr() const { return m_viewLineMgr; }
	int		lineStartPosition(int ln) const;
	int		fontHeight() const { return m_fontHeight; }
	int		lineHeight() const { return m_lineHeight; }
	int		textWidth(const QString &) const;
	int		textWidth(pos_t, ssize_t, /*pos_t,*/ const Buffer* = 0) const;
	int		pxToOffset(int vln, int px) const;
	int		columnToPos(int vln, int clmn) const;
	int		posEOS(int dln) const;		//	dln 行の改行位置を返す
	void	pointToLineOffset(const QPoint &, int &, int &) const;
	bool 	getSelectedLineRange(int &dln1, int &dln2) const;
	const TextCursor*	textCursor() const;
	int	endOfLinePosition(int dln) const;		//	行の改行位置を返す

public:
	MainWindow	*mainWindow() { return m_mainWindow; }
	ViewLineMgr *viewLineMgr() { return m_viewLineMgr; }
	TextCursor*	textCursor() { return m_textCursor; }
	void	clearLineFlags();
	void	setModified(bool = true);
	void	setLineNumberVisible(bool);
	void	setPlainText(const QString&);
	Document*	document() { return m_document; }
	Buffer	*buffer() { return m_buffer; }
	bool	makeCursorInView(bool bQuarter = false);
	void	doViCommand();		//	viEngine が保有する vi コマンドを実行
	void	doFindText(const QString &);		//	/ ? 検索処理
	void	doFindText(const QString &, bool fwd);		//	/ ? 検索処理
	void	curMove(int mv, int n = 1, bool vi = false);
	void	jumpToLine(int ln, bool vi = false);		//	ln [0, EOFLine) ドキュメント行番号
	void	jumpAssociatedParen();
	void	setMark(pos_t pos, char ch);
	void	markSetUnset();
	void	clearMark(pos_t pos);
	bool	nextMark(bool = false);		//	次のマーク位置に移動（非ループ）
	bool	prevMark(bool = false);		//	前のマーク位置に移動（非ループ）
	bool	jumpMarkPos(char ch);
	void	scrollDown();
	void	scrollUp();
	void	scrollDownPage(bool = false);
	void	scrollUpPage(bool= false);
	void	scrollDownHalfPage();
	void	scrollUpHalfPage();
	void	exposeBottomOfScreen();
	void	exposeTopOfScreen();
	void	scrollCurTopOfScreen();
	void	scrollCurQuarterOfScreen();
	void	scrollCurCenterOfScreen();
	void	scrollCurBottomOfScreen();
	void	curTopCenterBottomOfScreen();
	void	curTopOfScreen(bool vi = false, int cnt = 1);
	void	curMiddleOfScreen(bool vi = false);
	void	curBottomOfScreen(bool vi = false, int cnt = 1);
	void	setCursorPosition(pos_t pos, int mode = TextCursor::MOVE_ANCHOR);
	void	nextTag();
	void	prevTag();
	void	nextHeadingLine();
	void	prevHeadingLine();
	void	nextBlankLine();
	void	prevBlankLine();
	void	begOfCurSection();		//	[[
	void	begOfNextSction();		//	]]
	void	jumpToLastModifiedPos();
	void	deleteText(pos_t pos, ssize_t sz = 1, bool BS = false);
	void	onCursorPosChanged();
	void	doInsertText(const QString &, bool, bool, bool);
	void	insertText(const QString &);
	void	insertText(const QString &, const QString &);		//	選択領域の前後に文字挿入
	void	insertTextRaw(pos_t pos, const QString &);
	void	replaceText(const QString &);
	QString	indentText(int ln);
	QString	autoIndentText(/*bool,*/ bool nxline = true);
	void	paste(const QString &);
	void	boxPaste(const QString &);
	void	joinLines(int = 0, bool vi = false);
	void	incDec(bool bInc, int n = 1);		//	インクリメント・デクリメント
	void	encomment();
	void	decomment();
	void	blockComment();
	void	openPrevLine();
	void	openNextLine();
	void	toggleUpperLowerCase();
	void	toggleTrueFalse();
	void	openUndoBlock();
	void	closeUndoBlock();
	void	closeAllUndoBlock();
	void	clearUndoMgr();
	void	indent(int, int, bool vi = false);
	void	revIndent(int, int, bool vi = false);
	void	selectAll();
	void	updateFont();
	void	setFullPathName(const QString &);
	bool	saveFile() const;
	bool	searchCurWord(QString &, bool vi = false);
	bool	findForward(const QString &, uint opt = 0, bool loop = false, bool next = true, bool vi = false);
	bool	findBackward(const QString &, uint opt = 0, bool loop = false, bool vi = false);
#if	1
	void	findNext(const QString &, bool vi = false);
	void	findPrev(const QString &, bool vi = false);
#else
	void	findNext(const QString&, bool word, bool vi = false);
	void	findPrev(const QString&, bool word, bool vi = false);
#endif
	void	emitModifiedChanged() { emit modifiedChanged(); }
	QString	getLineText(int dln) const;
	uint	lineFlags(int dln) const;
	void	substitute(int dln1, int dln2, const QString &pat, const QString &rep, const QString &opt);
	void	zenCoding();
	bool	zenCodingFromFile(const QString &, const QString &, const QString &, const pos_t);
	bool	zenCodingFromFile(const QString &, const QString &, const pos_t);
	void	completion();
	void	kwCompletion();
	void	clearOpenCloseParenPos();
	void	checkAssocParen();
	void	checkAssocParen(int vln, pos_t pos);
	void	checkAssocSharpTag();
	bool	isCppType() const;
	bool	isSpaceText(pos_t first, pos_t last);
	void	insertSharp();			//	# が押された場合の処理
	void	insertOpenCurl(QString &);		//	{ が押された場合の処理
	void	insertCloseCurl(QString &);		//	} が押された場合の処理
	void	insertCaseSpace(QString &);		//	"case" の直後に 半角空白 が押された場合の処理
	void	copyImplCode();
	QString getImplText(pos_t &pos);
	void	tagJump();
	void	tagsJump();
	void	moveLineCmtToPrev();	//	現在行に行コメントがあれば、全行に移動
	//void	toggleTrueFalse();
	void	sharpIfCommentOut();
	void	sharpIfElseCommentOut();
	void	appendCut();
	int		appendCopy();

protected:
	void	viFindCharForward(wchar_t qch, bool bPrev, int mvmd, int repCnt);
	void	viFindCharBackward(wchar_t qch, bool bPrev, int mvmd, int repCnt);
	void	viYankText();
	void	viYankLine();
	void	viPastePrev();
	void	viPasteNext();
	bool	focusNextPrevChild(bool next);
	void	resetCursorBlinking();
	void	drawLineNumbers();
	void	drawLineNumberArea(QPainter&);
	void	drawTextArea(QPainter&);
	void	drawLineText(QPainter&, int&, int, int, pos_t, int, pos_t, bool&, bool&, QString&);
	int		drawTokenText(QPainter& pt, QString& token, int& clmn, int& px, int py, int peDX, int wd,
							const int chWidth, const int descent /*, QColor& col*/ , bool bold);
	void	drawPreeditString(QPainter&);
	//void	drawPreeditBG(QPainter&);
	void	drawCursor(QPainter&);
	void	drawLineCursor(QPainter&);
	void	drawMatchedBG(QPainter&);
	void	drawMatchedBG(QPainter &, int ln, int py);
	void	drawCurWordBG(QPainter &, int ln, int py);
	void	drawEmphasizedBG(QPainter &, int ln, int py, int &eix);
	void	drawAssocParenBG(QPainter &);
	void	drawAssocParenBG(QPainter &, int vln, int py);
	void	drawSelection(QPainter&);
	void	drawMinMap(QPainter&);
	void	updateLineNumberInfo();
	void	onEscape(bool, bool, bool);
	void	onResized();
	void	insertTextSub(QString, bool ctrl, bool shift, bool alt);
	void	onBackSpace(bool ctrl, bool shift, bool alt);
	void	onDelete(bool ctrl, bool shift, bool alt);
	void	setupFallingChars();
	void	setupFallingCharsBoxSelected();
	int		setupCompletionCandidates(QStringList &, const QString &, pos_t);
	bool setupKeywordsCandidates(QStringList &, const QString &);
	void	setupKeywordsCandidates(QStringList &, const QString &, const QList<QString> &);
	void	setupCompletionCandidates(const QString &fileName, SSSearch *srch, QStringList &, const QString &);
	bool	setupCompletionCandidatesFollowingSpaces(QStringList &);
	bool	checkFileNameCompletion(QStringList &, QString &, QString &, pos_t &);	//	#include 補完
	bool	checkSrcFileNameCompletion(QStringList &, QString &, QString &, pos_t &);	//	src=" 補完
	bool	checkEndTagCompletion(QStringList &, pos_t &);	//	</endtag 補完
	bool	checkEndBraceCompletion(int);		//	} 補完
	bool	fileNameCompletion(QStringList &, QString &, pos_t &);
	bool	clibralyCompletion(QStringList &, const QString &, pos_t &);
	void	completion(bool);
	bool	isSpaces(pos_t first, pos_t last) const;
	void	setupCloseTags(QStringList &);
	void	showAutoCompletionDlg(const QStringList &, QString = QString(), bool = false, bool = true);
	void closeAutoCompletionDlg();
	bool	isAfter(pos_t &, const QString &, pos_t pos = -1);				//	単語 \s+ 直後か？
	bool	isAfterFor(pos_t &);				//	for \s+ 直後か？
	bool	isAfterInclude();		//	#include \s* 直後か？
	bool	isAfterIncludeDQ(pos_t &);		//	#include \s* "... 直後か？
	void setupHeaders(QStringList &lst, pos_t pos2, const QString &);
	bool setupWord(QStringList &, QString &, pos_t &);
	bool editForVar(const QString &);
	void	zenCodingCPP(const QString &, const QString &, int);
	void	zenCodingPython(const QString &, const QString &, int);
	bool zenCodingFor(const QString &, const QString &, int, QChar);
	void	sharpIfCommentOut(bool bElse);
	ViEngine	*viEngine();
	
protected:
	void	paintEvent(QPaintEvent *);
	void	mousePressEvent(QMouseEvent *);
	void	mouseMoveEvent(QMouseEvent *);
	void	mouseReleaseEvent(QMouseEvent *);
	void	mouseDoubleClickEvent(QMouseEvent *);
	void	wheelEvent(QWheelEvent * event);
	void	keyPressEvent(QKeyEvent *);
    QVariant	inputMethodQuery ( Qt::InputMethodQuery query ) const;
	void	inputMethodEvent(QInputMethodEvent * event);
	void	resizeEvent(QResizeEvent *event);
	//bool	eventFilter(QObject *obj, QEvent *event);
	void	focusInEvent ( QFocusEvent * event );
	void	focusOutEvent ( QFocusEvent * event );
public slots:
	void	undo();
	void	redo();
	void	cut(bool append = false);
	int		copy(bool bCut = false, bool append = false);
	void	paste();
	void	imeOpenStatusChanged();

private slots:
	void autoCmplKeyPressed(QString);
	void autoCmplBackSpace();
	void autoCmplDelete(bool, bool);
	void autoCmplLeft(bool, bool);
	void autoCmplRight(bool, bool);
	void autoCmplZenCoding();
	void autoCmplPasted();
	void autoCmplDecided(QString, bool);
	void autoCmplRejected();
	void	cmplTextChanged(const QString &);
	void	onTimer();

signals:
	void	modifiedChanged() const;
	void	cursorPosChanged(EditView*, int, int);
	void	doOutput(const QString &) const;
	void	escPressed();
	void	showMessage(const QString &, int timeout = 0) const;
	void	reloadRequest(EditView *);
	void	textSearched(const QString&, bool word = false);
	void	openFile(const QString &);
	void	gameFinished(bool, QString);
	void	textInserted(const QString &);		//	挿入後の位置
	void	textBackSpaced();
	void	textCopied(const QString &);
	void	boxSelectModeChanged(bool);
	void	vertScrolled(int);		//	垂直スクロール
	void	horzScrolled(int);		//	垂直スクロール
	void	textObject(wchar_t, bool bInner);
	void	findTagsPat(const QString &pat, const QString &symbol);
	void	tagJump(const QString &, int);
	void	tagsJump(const QString &, const QString &, const QString &);
	void	outputViewClosed();
    void	updateUndoRedoEnabled();
	
private:
	bool	m_lineNumberVisible;
	bool	m_mouseDragging;			//	マウスボタン押下状態
	bool	m_mouseLineDragging;		//	行選択中
	bool	m_mouseDblClkDragging;		//	単語選択中
	bool	m_minMapDragging;			//	ミニマップ窓ドラッグ移動中
	bool	m_dispCursor;
	bool	m_noDeleteAnimation;		//	一時的に削除アニメーション禁止
	bool	m_noFallingChars;			//	一時的に削除文字落下禁止
	int		m_scrollX0;			//	水平方向スクロール（0 org カラム位置）
	int		m_scrollY0;
	int		m_fontWidth;
	int		m_fontHeight;
	int		m_fontDescent;		//	ベースライン下ピクセル数
	int		m_baseLineDY;		//	行上部からベースラインまでのピクセル数
	int		m_lineHeight;		//	行高（in Pixel）
	int		m_lineNumDigits;	//	log EOF行番号
	int		m_lineNumWidth;		//	行番号表示幅
	int		m_lineNumAreaWidth;
	int		m_nViewLine;
	int		m_curLineNum;				//	0 org
	int		m_curLineNumAnchor;		//	0 org
	int		m_tmCounter;
	int		m_preeditWidth;			//	変換中文字列表示幅
	int		m_preeditPos;			//	変換位置
	std::vector<int>	m_delForVarPos;			//	for 変数削除位置
	MainWindow	*m_mainWindow;
	//int		m_preeditLine;			//	変換行（論理行番号）
	QString	m_selectedString;		//	変換開始時に選択されていた文字列
	QString	m_preeditString;		//	変換中文字列
	QTimer	m_timer;
	//TypeSettings	*m_typeSettings;		//	タイプ設定へのウィークポインタ
	//##TypeSettings	*m_jsTypeSettings;		//	JavaScriptタイプ設定へのウィークポインタ
	//##TypeSettings	*m_phpTypeSettings;		//	PHPタイプ設定へのウィークポインタ
	//	undone: マルチカーソル対応
	TextCursor	*m_textCursor;
	//TextCursor	*m_svTextCursor;
	//
	ViewLineMgr	*m_viewLineMgr;
	Document	*m_document;
	Buffer		*m_buffer;				//	ポイントのみで、非所有
	QFont		m_font;
	QFont		m_fontBold;
	QFont		m_fontMB;				//	マルチバイト用フォント
	QWidget		m_lineNumAreaWidget;	//	行番号表示用ウィジェット？
	//QWidget		m_textAreaWidget;		//	テキスト表示用ウィジェット
	//QPixmap		m_textAreaPixmap;		//	テキスト表示用 Pixmap
	//	ミニマップ関連、undone:そのうち Document に移動
	QWidget		m_minMapWidget;
	//double		m_mmScale;				//	1.0 未満であれば縮小されている
	//QPixmap		m_minMap;
	//
	int		m_openParenPos;				//	対応する括弧強調
	int		m_openParenViewLine;
	int		m_closeParenPos;
	int		m_closeParenViewLine;
	bool	m_unbalancedAssocParen;		//	対応する括弧が無い or インデントが同じではない
	int		m_aiCurPos;						//	オートインデント直後のカーソル位置
	QString		m_aiSpaces;					//	オートインデント空白類
	std::vector<FallingChar>	m_fallingChars;	//	落下文字たち
	//
	bool	m_autoCmplAtBegWord;			//	単語先頭で自動補完
	int		m_autoCmplPos;			//	補完開始位置（# で開始した場合は # 位置）
	QString	m_autoCmplFilter;		//	動的補完フィルター文字列
	QString	m_autoCmplTyped;		//	動的補完でタイプされた文字列
	AutoCompletionDlg	*m_autoCompletionDlg;
};
