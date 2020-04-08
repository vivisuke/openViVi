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
