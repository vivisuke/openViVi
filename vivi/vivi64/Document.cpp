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

#define		MINMAP_LN_WD		4			//	行番号部分幅
#define		MINMAP_WIDTH		80
#define		MAX_MINMAP_HEIGHT	10000		//	ピックスマップ最大高さ

inline bool isNewLine(wchar_t ch)
{
	return ch == '\r' || ch == '\n';
}

Document::Document(QString typeName, QObject *parent)
	: QObject(parent)
	//, m_typeSettings(nullptr)
	, m_codec(0)
	, m_bomLength(0)
	, m_newLineCode(CharEncoding::CRLF)
	, m_mmSeqNumber(-1)
	, m_mmScale(1.0)
{
	m_typeSettings = new TypeSettings(typeName);
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
wchar_t Document::charAt(pos_t pos) const
{
	return m_buffer->charAt(pos);
}
QString Document::typeName() const
{
	return m_typeSettings->name();
}
QString Document::fullPathName() const
{
	return m_pathName;
}
void Document::setTypeSettings(TypeSettings* typeSettings)
{
	m_typeSettings = typeSettings;
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
	buffer()->clear();
	buffer()->insertText(0, (cwchar_t*)txt.data(), txt.size());
	buildMinMap();
}
int Document::positionToLine(pos_t pos) const
{
	return buffer()->positionToLine(pos);
}
void Document::buildMinMap()
{
	if( buffer()->lineCount() > 10000 ) return;		//	最大1万行
	int ht = qMin(MAX_MINMAP_HEIGHT, buffer()->lineCount());
	m_mmScale = (double)ht / buffer()->lineCount();
	m_minMap = QPixmap(MINMAP_WIDTH, ht);
	auto ts = typeSettings();
	m_minMap.fill(ts->color(TypeSettings::BACK_GROUND));
	QPainter painter(&m_minMap);
	painter.fillRect(QRect(0, 0, MINMAP_LN_WD, ht), ts->color(TypeSettings::LINENUM_BG));
	//if( lineCount() > 10000 ) return;
	painter.setPen(ts->color(TypeSettings::TEXT));
	bool inBlockComment = false;
	for (int ln = 0; ln < buffer()->lineCount(); ++ln) {
		int p = buffer()->lineStartPosition(ln);
		int last= buffer()->lineStartPosition(ln+1);
		int px = MINMAP_LN_WD;
		if( buffer()->charAt(p) == '\t' ) {
			while (buffer()->charAt(p) == '\t') {
				++p;
				px += ts->intValue(TypeSettings::TAB_WIDTH);
			}
		} else {
			while (buffer()->charAt(p) == ' ') {
				++p;
				++px;
			}
		}
		if( p >= buffer()->size() || isNewLine(buffer()->charAt(p)) ) continue;
		while( last > p && isNewLine(buffer()->charAt(last - 1)) )
			--last;
		painter.drawLine(px, ln*m_mmScale, px + last - p, ln*m_mmScale);
	}
}
int Document::lineStartPosition(pos_t pos) const
{
	return buffer()->lineStartPosition(pos);
}
