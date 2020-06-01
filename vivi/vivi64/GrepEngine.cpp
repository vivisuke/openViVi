#include <QtGui>
#include "GrepEngine.h"
#include "globalSettings.h"
#include "charEncoding.h"
#include "../buffer/Buffer.h"
#include "../buffer/SSSearch.h"

GrepEngine::GrepEngine(GlobalSettings *globalSettings, /*SSSearch *sssrc,*/ QObject *parent)
	: QObject(parent)
	, m_globSettings(globalSettings)
	//, m_sssrc(sssrc)
	, m_toTerminate(false)
	, m_terminated(false)
{
	m_sssrc = new SSSearch();
}

GrepEngine::~GrepEngine()
{
	delete m_sssrc;
}
//	dirStr 以下を grep 処理
void GrepEngine::doGrep(QString pat, QString extentions, QString dirStr, QString exclude)
{
	if( dirStr.endsWith('/') ) dirStr = dirStr.left(dirStr.size() - 1);
	uint opt = 0;
	if( m_globSettings->boolValue(GlobalSettings::IGNORE_CASE) )
		opt |= SSSearch::IGNORE_CASE;
	if( m_globSettings->boolValue(GlobalSettings::WHOLE_WORD_ONLY) )
		opt |= SSSearch::WHOLE_WORD_ONLY;
	byte algorithm = SSSearch::SAKUSAKU;
	if( m_globSettings->boolValue(GlobalSettings::REGEXP) )
		algorithm = SSSearch::STD_REGEX;
	int sum = 0;
	if( m_sssrc->setup((const wchar_t *)pat.data(), pat.size(), opt, algorithm) ) {
		sum = doGrepDir(pat, extentions, dirStr, QRegExp(exclude));
	}
	emit finished(sum);
}
int GrepEngine::doGrepDir(QString pat, QString extentions, QString dirStr, const QRegExp &regexp)
{
	if( m_toTerminate ) {
		if( !m_terminated )
			emit doOutput(tr("\n* grep terminated.\n"));
		m_terminated = true;
		return 0;
	}
	emit greppingDir(dirStr);
	QStringList extList = extentions.split(';');
	QDir dir(dirStr);
	QStringList files = dir.entryList(extList, QDir::Files, QDir::Name);
	qSort(files.begin(), files.end(),
				[](const QString &lhs, const QString &rhs) { return lhs.compare(rhs, Qt::CaseInsensitive) < 0; });
	int sum = 0;
	foreach(const QString fileName, files) {
		//qDebug() << fileName;
		//doOutput(dirStr + "/" + fileName + ":\n");
		if( !regexp.isEmpty() && regexp.indexIn(fileName) >= 0 )
			continue;
		QString fullPath = dirStr + "/" + fileName;
		sum += doGrepFile(fullPath, pat, regexp);
		if( m_terminated ) break;
	}
	if( m_globSettings->boolValue(GlobalSettings::GREP_SUB_DIR) ) {
		QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
		qSort(dirs.begin(), dirs.end(),
					[](const QString &lhs, const QString &rhs) { return lhs.compare(rhs, Qt::CaseInsensitive) < 0; });
		foreach(const QString dirName, dirs) {
			QString fullPath = dirStr + "/" + dirName;
			sum += doGrepDir(pat, extentions, fullPath, regexp);
			if( m_terminated ) break;
		}
	}
	return sum;
}
//	ファイル：fullPath から pat を検索
int GrepEngine::doGrepFile(const QString &fullPath, const QString &pat, const QRegExp &regexp)
{
	if( m_toTerminate ) {
		if( !m_terminated )
			emit doOutput(tr("\n* grep terminated.\n"));
		m_terminated = true;
		return 0;
	}
	QString errorString;
	QTextCodec *codec;
	int bomLength;
	byte newLineCode;
	if( !::getTextCodec(fullPath, errorString, codec, bomLength, newLineCode) )
		return 0;
	QFile file(fullPath);
	if( !file.open(QFile::ReadOnly /*| QFile::Text*/) )
		return 0;
	if( bomLength != 0 )
		file.seek(bomLength);
	Buffer buffer;
	QByteArray ba;
	while( !file.atEnd() ) {
		ba = file.read(1024*1024);	//	undone 1M境界処理
		QString buf = codec->toUnicode(ba);
		if( !buffer.basicInsertText(buffer.size(), (wchar_t *)buf.data(), buf.size()) ) {
			return 0;
		}
	}
	file.close();
	
	bool init = true;
	pos_t pos = 0;
	int cnt = 0;
	while( (pos = m_sssrc->strstr(buffer, pos)) >= 0 ) {
		++cnt;
		if( init ) {
			init = false;
			emit doOutput("\n\"" + fullPath + "\":\n");
		}
		int ln = buffer.positionToLine(pos);
		pos_t ls = buffer.lineStartPosition(ln);
		pos_t nxls = buffer.lineStartPosition(ln+1);
		QString text = QString("%1:").arg(ln+1, 6);
		while( ls < nxls ) {
			text += QChar(buffer[ls++]);
		}
		emit doOutput(text);
		pos = nxls;
	}
	return cnt;
}
void GrepEngine::terminate()
{
	m_toTerminate = true;
}
