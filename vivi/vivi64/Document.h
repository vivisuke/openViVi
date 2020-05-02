#pragma once

#include <QObject>
#include <QDateTime>
#include <QPixmap>
#include <vector>

typedef unsigned char uchar;
typedef const char cchar;
typedef unsigned char byte;
typedef unsigned int uint;
#ifdef	_WIN64
	typedef __int64 ssize_t;
	typedef __int64 pos_t;
#else
	typedef __int32 ssize_t;
	typedef __int32 pos_t;
#endif
	typedef __int32 line_t;			//	行番号

class Buffer;
struct MarkItem;
class EditView;
class TypeSettings;
class SSSearch;

class Document : public QObject
{
	Q_OBJECT

public:
	Document(QString typeName = QString(), QObject *parent = 0);
	~Document();
public:
	int		size() const;
	wchar_t	charAt(pos_t pos) const;
	bool	bom() const { return m_bBom; }
	uchar	charEncoding() const { return m_charEncoding; }
	QByteArray	codecName() const;
	bool	isEmpty() const;
	bool	isModified() const;		// { return m_modified; }
	bool	canUndo() const;
	bool	canRedo() const;
	bool	saveFile() const;
	int		lineCount() const;
	int		EOFLine() const;
	uint	lineFlags(int ln) const;
	int		mmSeqNumber() const { return m_mmSeqNumber; }
	QString	fullPathName() const { return m_fullPathName; }
	QString	title() const { return m_title; }
	QString	typeName() const;
	QString	text(pos_t pos, ssize_t sz) const;
	QString	lineText(int) const;
	QDateTime	lastModified() const { return m_lastModified; }
public:
	void	setTypeSettings(TypeSettings *typeSettings);
	void	setPathName(const QString &pathName);
	void	setTitle(const QString &);
	void	setCodecName(const QByteArray &);
	void	setBOM(bool bBom);
	void	setCharEncoding(uchar charEncoding);
	void	setModified(bool = true);
	void	setLastModified(const QDateTime&);
	void	setPlainText(const QString&);
	void	buildMinMap();
	QPixmap&	minMap() { return m_minMap; }
	int		lineStartPosition(pos_t pos) const;
	void	setLineFlag(line_t, uint);
	void	resetLineFlag(line_t, uint);
	
	//QString	fullPathName() const;
	//QString	typeName() const;
	TypeSettings	*typeSettings() { return m_typeSettings; }
	const TypeSettings	*typeSettings() const { return m_typeSettings; }
	Buffer	*buffer() { return m_buffer; }
	const Buffer	*buffer() const { return m_buffer; }
	int		positionToLine(pos_t pos) const;
	void	deleteText(pos_t pos, ssize_t sz = 1, bool BS = false);
	void	insertText(pos_t pos, const QString &);
	void	updateView(EditView *);
	void	clearUndoMgr();
	int		undo();
	int		redo();
	void	cut(pos_t pos, ssize_t sz);
	void	copy(pos_t pos, ssize_t sz, bool append = false);
	int		paste(pos_t pos);	//	ペースト後のカーソル位置を返す
private:
	QString	m_fullPathName;
	QString	m_title;
	Buffer	*m_buffer;
	bool	m_bBom;
	byte	m_newLineCode;
	uchar	m_charEncoding;
	int		m_bomLength;
	TypeSettings	*m_typeSettings;
	mutable QTextCodec	*m_codec;
	//QString	m_fullPathName;
	//QString	m_title;
	mutable QDateTime	m_lastModified;
	std::vector<EditView *>	m_views;
	int			m_mmSeqNumber;		//	ミニマップ作成時シリアル番号
	double		m_mmScale;			//	1.0 未満であれば縮小されている
	QPixmap		m_minMap;
	//double		m_wmScale;				//	1.0 未満であれば縮小されている
	//QPixmap	m_wholeMap;
};
