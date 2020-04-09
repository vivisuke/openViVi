#pragma once

#include <QObject>
#include <QDateTime>
#include <QPixmap>
#include <vector>

typedef unsigned char uchar;
typedef const char cchar;
typedef unsigned char byte;
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
	Document(QObject *parent = 0);
	~Document();
public:
	void	setPathName(const QString &pathName);
	void	setTitle(const QString &);
	void	setCodecName(const QByteArray &);
	void	setBOM(bool bBom);
	void	setCharEncoding(uchar charEncoding);
private:
	QString	m_pathName;
	QString	m_title;
	Buffer	*m_buffer;
	bool	m_bBom;
	byte	m_newLineCode;
	uchar	m_charEncoding;
	int		m_bomLength;
	mutable QTextCodec	*m_codec;
	//QString	m_fullPathName;
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^QString	m_title;
	mutable QDateTime	m_lastModified;
	std::vector<EditView *>	m_views;
	int			m_wmSeqNumber;		//	全体マップ作成時シリアル番号
	double		m_wmScale;				//	1.0 未満であれば縮小されている
	QPixmap	m_wholeMap;
};
