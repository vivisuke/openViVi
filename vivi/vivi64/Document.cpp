#include <QtGui>
#include <algorithm>
#include "Document.h"
#include "EditView.h"
#include "mainwindow.h"
#include "typeSettings.h"
#include "viewTokenizer.h"
#include "../buffer/Buffer.h"
#include "../buffer/MarkMgr.h"
#include "charEncoding.h"

inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}

Document::Document(QObject *parent)
	: QObject(parent)
	, m_codec(0)
	, m_bomLength(0)
	, m_newLineCode(CharEncoding::CRLF)
	, m_wmSeqNumber(-1)
	, m_wmScale(1.0)
{
	m_buffer = new Buffer();
}
Document::~Document()
{
#if	0
	for(auto* view: m_views) {
		view->onDocumentDeleted();
	}
#endif
	delete m_buffer;
}
void Document::setPathName(const QString &pathName)
{
	m_pathName = pathName;
}
void Document::setTitle(const QString &title)
{
	m_title = title;
}
void Document::setCodecName(const QByteArray &)
{
}
void Document::setBOM(bool bBom)
{
	m_bBom = bBom;
}
void Document::setCharEncoding(uchar charEncoding)
{
	m_charEncoding = charEncoding;
}
void Document::setLastModified(const QDateTime& lastModified)
{
	m_lastModified = lastModified;
}
void Document::setPlainText(const QString& txt)
{
}
