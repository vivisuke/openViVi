#pragma once

#include <QObject>
#include <QDateTime>
#include <QPixmap>
#include <vector>

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
};
