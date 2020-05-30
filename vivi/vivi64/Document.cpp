#include <QtGui>
#include <QMessageBox>
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
	, m_bBom(false)
	//, m_bomLength(0)
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
int Document::size() const
{
	return m_buffer->size();
}
wchar_t Document::charAt(pos_t pos) const
{
	return m_buffer->charAt(pos);
}
bool Document::isEmpty() const
{
	return m_buffer->isEmpty();
}
bool Document::isModified() const		// { return m_modified; }
{
	return m_buffer->isModified();
}
QByteArray Document::codecName() const
{
	if( m_codec == 0 )
		return QByteArray("UTF-8");
	else
		return m_codec->name();
}
bool Document::canUndo() const
{
	return m_buffer->canUndo();
}
bool Document::canRedo() const
{
	return m_buffer->canRedo();
}
void Document::clearUndoMgr()
{
	m_buffer->clearUndoMgr();
}
int Document::undo()
{
	return m_buffer->undo();
}
int Document::redo()
{
	return m_buffer->redo();
}
QString Document::typeName() const
{
	return m_typeSettings->name();
}
int Document::lineCount() const
{
	return m_buffer->lineCount();
}
QString Document::text(pos_t pos, ssize_t sz) const
{
	Q_ASSERT( sz > 0 );
	try {
		QString txt(sz, 0);
		m_buffer->getText(pos, (wchar_t *)txt.data(), sz);
		return txt;
	} catch(...) {
		QMessageBox::warning(0, "vivi64",
								tr("because of No Memory, can't build a string."));
		return QString();
	}
}
QString Document::lineText(int ln) const
{
	if( ln < 0 || ln >= lineCount() ) return QString();
	pos_t pos = m_buffer->lineStartPosition(ln);
	ssize_t sz = m_buffer->lineSize(ln);
	if( sz <= 0 ) return QString();
	return text(pos, sz);
}
void Document::setTypeSettings(TypeSettings* typeSettings)
{
	m_typeSettings = typeSettings;
}
void Document::setPathName(const QString &pathName)
{
	m_fullPathName = pathName;
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
void Document::setModified(bool b)
{
	m_buffer->setModified(b);
}
void Document::setLastModified(const QDateTime& lastModified)
{
	m_lastModified = lastModified;
}
void Document::setPlainText(const QString& txt)
{
	buffer()->clear();
	buffer()->insertText(0, (cwchar_t*)txt.data(), txt.size());
	buffer()->clearLineFlags();
	buffer()->clearUndoMgr();
	buffer()->setModified(false);
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
uint Document::lineFlags(int ln) const
{
	return buffer()->lineFlags(ln);
}
void Document::setLineFlag(line_t ln, uint flags)
{
	buffer()->setLineFlag(ln, flags);
}
void Document::resetLineFlag(line_t ln, uint flags)
{
	buffer()->resetLineFlag(ln, flags);
}
void Document::deleteText(pos_t pos, ssize_t sz, bool BS)
{
	m_buffer->deleteText(pos, sz, BS);
}
void Document::insertText(pos_t pos, const QString &text)
{
	if( !m_buffer->insertText(pos, (const wchar_t *)text.data(), text.size()) ) {
		QMessageBox::warning(0, tr("ViVi64"),
								tr("because of No Memory, can't insert text."));
	}
}
void Document::copy(pos_t pos, ssize_t sz, bool append)
{
	QClipboard *cb = qApp->clipboard();
	QString txt = text(pos, sz);
	if( !txt.isEmpty() ) {
		if( append )
			txt = cb->text() + txt;
		cb->setText(txt);
	}
}
void Document::updateView(EditView *view)
{
}
bool Document::saveFile() const
{
	if( m_fullPathName.isEmpty() ) return false;
	if( m_codec == 0 )
		m_codec = QTextCodec::codecForName("UTF-8");
	QFile file(m_fullPathName);
	if( !file.open(QFile::WriteOnly) ) {
		//errorString = file.errorString();
		return false;
	}
	if( m_bBom ) {
		QString name = m_codec->name();
		QByteArray ba;
		if( name == "UTF-8" ) {
			ba += 0xef;
			ba += 0xbb;
			ba += 0xbf;
		} else if( name == "UTF-16LE" ) {
			ba += 0xff;
			ba += 0xfe;
		} else if( name == "UTF-16BE" ) {
			ba += 0xfe;
			ba += 0xff;
		}
		if( !ba.isEmpty() )
			file.write(ba);
	}
	QTextCodec::ConverterState cs(QTextCodec::IgnoreHeader);
	for(int ln = 0; ln < m_buffer->lineCount(); ++ln) {
		QString txt = lineText(ln);
		QByteArray ba = m_codec->fromUnicode(txt.data(), txt.size(), &cs);
		file.write(ba);
	}
	m_buffer->onSaved();
	file.close();
	m_lastModified = QFileInfo(m_fullPathName).lastModified();
	return true;
}
