//----------------------------------------------------------------------
//
//			File:			"charEncoding.cpp"
//			Created:		03-Mar-2011
//			Author:			津田伸秀
//			Description:	文字コード判定関数実装
//
//----------------------------------------------------------------------

/*

	Copyright (C) 2011 by Nobuhide Tsuda

*/

#include <QtCore>
#include <QMessageBox>
#include "charEncoding.h"

#define		EUC_KANA_LEADBYTE		((uchar)0x8e)

bool const DBCStable1[0x100] = {		//	ＤＢ文字のファーストバイトかどうか
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 3 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 5 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 6 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 7 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 8 */	0, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 9 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* a */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* b */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* c */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* d */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* e */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* f */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0, 0,
};

bool const DBCStable2[0x100] = {		//	ＤＢ文字のセカンドバイトかどうか
/* 0 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 3 */	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 5 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 6 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 0,
/* 8 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 9 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* a */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* b */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* c */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* d */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* e */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* f */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 0, 0,
};

uchar const DBCSsizeTable[0x100] = {
/* 0 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 1 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 2 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 3 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 4 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 5 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 6 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* 8 */	1, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2,
/* 9 */	2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2,
/* a */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* b */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* c */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* d */	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
/* e */	2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2,
/* f */	2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 1, 1,
};

bool isDBCSLeadByte(uchar ch)
{
	return DBCStable1[ch];
}
bool isDBCSSecondByte(uchar ch)
{
	return DBCStable2[ch];
}

uchar	UTF8_BOM[UTF8_BOM_LENGTH] = {0xef, 0xbb, 0xbf};
uchar	UTF16LE_BOM[UTF16_BOM_LENGTH] = {0xff, 0xfe};
uchar	UTF16BE_BOM[UTF16_BOM_LENGTH] = {0xfe, 0xff};

inline bool isUTF16leBom(cuchar *ptr, cuchar *endptr)
{
	return ptr + 1 < endptr && ptr[0] == UTF16LE_BOM[0] && ptr[1] == UTF16LE_BOM[1];
	//return ptr + 1 < endptr && ptr[0] == 0xff && ptr[1] == 0xfe;
}

inline bool isUTF16beBom(cuchar *ptr, cuchar *endptr)
{
	return ptr + 1 < endptr && ptr[0] == UTF16BE_BOM[0] && ptr[1] == UTF16BE_BOM[1];
	//return ptr + 1 < endptr && ptr[0] == 0xfe && ptr[1] == 0xff;
}

inline bool isUTF8Bom(cuchar *ptr, cuchar *endptr)
{
	return ptr + 2 < endptr && ptr[0] == UTF8_BOM[0] &&
			ptr[1] == UTF8_BOM[1] && ptr[2] == UTF8_BOM[2];
	//return ptr + 2 < endptr && ptr[0] == 0xef && ptr[1] == 0xbb && ptr[2] == 0xbf;
}

uchar checkCharEncoding(cuchar *ptr, cuchar *endptr, int &bomLength)
{
	if( isUTF8Bom(ptr, endptr) ) {
		bomLength = 3;
		return CharEncoding::UTF8;
	}
	if( isUTF16leBom(ptr, endptr) ) {
		bomLength = 2;
		return CharEncoding::UTF16LE;
	}
	if( isUTF16beBom(ptr, endptr) ) {
		bomLength = 2;
		return CharEncoding::UTF16BE;
	}
	bomLength = 0;
	cuchar *ptr0 = ptr;

	bool	psEUC = true;		//	EUC の可能性あり
	bool	psSJIS = true;		//	SJIS の可能性あり
	bool	psUTF8 = true;		//	UTF-8の可能性あり
	bool	psUTF16LE = true;		//	UTF-16の可能性あり
	bool	psUTF16BE = true;		//	UTF-16の可能性あり
	bool	EUCKana = false;	//	EUC_KANA_LEADBYTE があった場合
	bool	dbSJIS = false;		//	SJISダブルバイト文字発見
	bool	hanKanaSJIS = false;	//	SJIS 半角カナがあった場合
	bool	ansi = false;		//	ANSI 文字があった場合
	int		nthEUC = 0;			//	何文字目か
	int		nthSJIS = 0;
	int		nthUTF8 = 0;
	int		nextNthUTF8;		//	3バイト UTF-8 かどうか（2:3 byte, 0:2byte）
	int	unixRetCount = 0;	//	0x0a での改行の数
	int dosRetCount = 0;	//	0x0d 0x0a での改行の数
	int macRetCount = 0;	//	0x0d での改行の数
	int zeroCount[] = {0, 0};
	int	nByteSJIS = 0;		//	SJIS として解釈可能な2バイト文字のバイト数
	int	nByteEUC = 0;		//	EUC として解釈可能な2バイト文字のバイト数
	int	nByteUTF8 = 0;		//	UTF8 として解釈可能な2バイト以上文字のバイト数

	if( endptr - ptr > 0x100000 )
		endptr = ptr + 0x100000;		//	最大1024キロを検査
	endptr -= 2;	//	ptr + 1, ptr + 2 までアクセスする可能性があるため
	uchar uch;
	while( ptr < endptr ) {
		if( (uch = (uchar)*ptr++) >= 0x80 ) {
			if( psSJIS ) {			//	SJIS の可能性が残っている
				if( nthSJIS == 0 ) {		//	最初のバイト
					if( isDBCSLeadByte(uch) && isDBCSSecondByte((uchar)*ptr) ) {
						dbSJIS = true;
						nthSJIS = 1;
						nByteSJIS += 2;
					} else if( !isHankakuKana(uch) ) {
						psSJIS = false;
						if( !psUTF16LE && !psUTF16BE ) {
							if( !psUTF8 )
								return CharEncoding::EUC;
							if( !psEUC )
								return CharEncoding::UTF8;
						}
					}
				} else
					nthSJIS = 0;
			}
			if( psEUC ) {			//	EUC の可能性が残っている
				if( nthEUC == 0 ) {
					if( uch < 0x80 ) {
						++nByteEUC;
					} else if( isEUCcode(uch) && isEUCcode((uchar)*ptr) ||
						uch == EUC_KANA_LEADBYTE && isHankakuKana((uchar)*ptr) )
					{
						nthEUC = 1;
						nByteEUC += 2;
					} else {
						psEUC = false;
						if( !psUTF16LE && !psUTF16BE ) {
							if( !psUTF8 )
								return CharEncoding::SJIS;
							if( !psSJIS )
								return CharEncoding::UTF8;
						}
					}
				} else {
					//++nByteEUC;
					nthEUC = 0;
				}
			}
			if( psUTF8 ) {			//	UTF-8 の可能性が残っている
				switch( nthUTF8 ) {
				case 0:
					if( (uch & 0xe0) == 0xc0 ) {	//	2バイトコードの1文字目
						if( ((uchar)*ptr & 0xc0) == 0x80 ) {
							nextNthUTF8 = 0;
							nthUTF8 = 1;
							nByteUTF8 += 2;
						} else
							psUTF8 = false;
					} else if( (uch & 0xf0) == 0xe0 ) {	//	3バイトコードの1文字目
						if( ((uchar)*ptr & 0xc0) == 0x80 && ((uchar)ptr[1] & 0xc0) == 0x80) {
							nextNthUTF8 = 2;
							nthUTF8 = 1;
							nByteUTF8 += 3;
						} else
							psUTF8 = false;
					} else
						psUTF8 = false;
					break;
				case 1:
					nthUTF8 = nextNthUTF8;		//	1 or 3
					break;
				case 2:
					nthUTF8 = 0;
					break;
				}
				if( !psUTF8 && !psUTF16LE && !psUTF16BE ) {
					if( !psEUC )
						return CharEncoding::SJIS;
					if( !psSJIS )
						return CharEncoding::EUC;
				}
			}
		} else {	//	uch < 0x80
			nthSJIS = nthEUC = 0;
			if( uch == 0 ) {
				zeroCount[(ptr - ptr0 - 1)&1] += 1;
				if(  ptr[1] == 0 && ptr[3] == 0 && ptr[5] == 0 ) {
					//	0x00 がひとつおきに続く場合は UNICODE ということにしておく	//##
					return zeroCount[0] <= zeroCount[1] ? CharEncoding::UTF16LE : CharEncoding::UTF16BE;
				}
			}
			if( uch >= ' ' && uch < 0x7f )
				ansi = true;
			if( uch < ' ' ) {
				//	UTF-16LE ならは次の文字は 0x00 のはず
				if( ptr < endptr && *ptr != '\0' )
					psUTF16LE = false;
				//	UTF-16LE ならは前の文字は 0x00 のはず
				if( ptr - 2 >= ptr0 && ptr[-2] != '\0' )
					psUTF16LE = false;
			}
		}
		switch( uch ) {
#if 0
		//	JIS は当面サポートしない
		case 0x1b:
			if( ptr + 1 < endptr &&
				(ptr[0] == '(' && (ptr[1] == 'B' || ptr[1] == 'J') ||
				ptr[0] == '$' && (ptr[1] == 'B' || ptr[1] == '@')) )
			{
				return CharEncoding::JIS;
			}
			break;
#endif
		case 0x0a:
			unixRetCount += 1;
			break;
		case 0x0d:
			if( ptr < endptr && *ptr == 0x0a ) {
				ptr += 1;
				dosRetCount += 1;
			} else
				macRetCount += 1;
		}
	}
	if( psUTF8 && nByteUTF8 > nByteSJIS && nByteUTF8 > nByteEUC )
		return CharEncoding::UTF8;
	if( psSJIS && nByteSJIS > nByteEUC && nByteSJIS > nByteUTF8 )
		return CharEncoding::SJIS;
	if( psEUC && nByteEUC > nByteSJIS && nByteEUC > nByteUTF8 )
		return CharEncoding::EUC;
	//if( !psUTF8 )
	//	return dosRetCount >= unixRetCount && dbSJIS ? CharEncoding::SJIS : CharEncoding::EUC;
	if( !psSJIS ) {
		if( psEUC && !psUTF8 ) return CharEncoding::EUC;
		if( !psEUC && psUTF8 ) return CharEncoding::UTF8;
	}

	if( psSJIS && dbSJIS && !psEUC ) return CharEncoding::SJIS;			//	ダブルバイト文字を含んでいた場合
#if 0
	if( stgMgr->getGlobSettings()->getBoolValue(GLOBSTG_ASCII_LF_SJIS) )
		return CharEncoding::SJIS;
#endif
	//return dosRetCount >= unixRetCount ? CharEncoding::SJIS : CharEncoding::EUC;
	return CharEncoding::UTF8;
}

bool getTextCodec(const QString &fileName, QString &errorString, QTextCodec *&codec,
					int &bomLength, byte &newLine)
{
	QDir path(fileName);
	QFile file(path.path());
	if( !file.open(QFile::ReadOnly /*| QFile::Text*/) ) {
		errorString = file.errorString();
		return false;
	}
	QByteArray ba = file.read(4096);
	cuchar *ptr = (uchar *)(ba.data());
	//cuchar *ptr = static_cast<uchar *>(ba.data());
	cuchar *endptr = ptr + ba.size();
	//int bomLength;
	uchar ce = checkCharEncoding(ptr, endptr, bomLength);
	//bom = bomLength != 0;
	cchar *codecName = 0;
	switch( ce ) {
	case CharEncoding::UTF8:
		codecName = "UTF-8";
		break;
	case CharEncoding::UTF16LE:
		codecName = "UTF-16LE";
		break;
	case CharEncoding::UTF16BE:
		codecName = "UTF-16BE";
		break;
	case CharEncoding::EUC:
		codecName = "EUC-JP";
		break;
	case CharEncoding::SJIS:
		codecName = "Shift-JIS";
	}
	codec = codecName ? QTextCodec::codecForName(codecName) : 0;
	if( codec == 0 )
		codec = QTextCodec::codecForLocale();
	if( codec == 0 ) {
		QMessageBox::warning(0, "SakuSakuEditor",
			QObject::tr("No QTextCodec for %1.")
							 .arg(QString(codecName ? codecName : "Locale")));
		return false;
	}
	newLine = checkNewLineCode(ptr, endptr, ce);
	return true;
}
#if	0
bool loadFile(const QString &fileName, QString &buffer, QString &errorString,
				uchar *cePtr,		//	文字エンコーディング
				bool *wbPtr)		//	BOM付き
{
	QDir path(fileName);
	QFile file(path.path());
	if( !file.open(QFile::ReadOnly /*| QFile::Text*/) ) {
		errorString = file.errorString();
		buffer.clear();
		return false;
	}
	QByteArray ba = file.readAll();
	cuchar *ptr = (uchar *)(ba.data());
	//cuchar *ptr = static_cast<uchar *>(ba.data());
	cuchar *endptr = ptr + ba.size();
	int bomLength;
	uchar ce = checkCharEncoding(ptr, endptr, bomLength);
	cchar *codecName = 0;
	switch( ce ) {
	case CharEncoding::UTF8:
		codecName = "UTF-8";
		break;
	case CharEncoding::UTF16LE:
		codecName = "UTF-16LE";
		break;
	case CharEncoding::UTF16BE:
		codecName = "UTF-16BE";
		break;
	case CharEncoding::EUC:
		codecName = "EUC-JP";
		break;
	case CharEncoding::SJIS:
		codecName = "Shift-JIS";
	}
	QTextCodec *codec = codecName ? QTextCodec::codecForName(codecName) : 0;
	if( codec == 0 )
		codec = QTextCodec::codecForLocale();
	if( codec == 0 ) {
		QMessageBox::warning(0, "ViVi 5.0",
			QObject::tr("No QTextCodec for %1.")
							 .arg(QString(codecName ? codecName : "Locale")));
		return false;
	}
	buffer = codec->toUnicode(ba);
	if( cePtr != 0 ) *cePtr = ce;
	if( wbPtr != 0 ) *wbPtr = bomLength != 0;
	return true;
}
#endif
bool loadFile(const QString& pathName, QString& buffer, QString& errorMess,
				uchar& charEncoding,		//	文字エンコーディング
				int& bomLength)		//	BOM付き
{
	QDir path(pathName);
	QFile file(path.path());
	if( !file.open(QFile::ReadOnly /*| QFile::Text*/) ) {
		errorMess = file.errorString();
		buffer.clear();
		return false;
	}
	QByteArray ba = file.readAll();
	cuchar *ptr = (uchar *)(ba.data());
	//cuchar *ptr = static_cast<uchar *>(ba.data());
	cuchar *endptr = ptr + ba.size();
	//int bomLength;
	uchar ce = charEncoding = checkCharEncoding(ptr, endptr, bomLength);
	qDebug() << "charEncoding = " << (int)charEncoding;
	cchar *codecName = 0;
	switch( ce ) {
	case CharEncoding::UTF8:
		codecName = "UTF-8";
		break;
	case CharEncoding::UTF16LE:
		codecName = "UTF-16LE";
		break;
	case CharEncoding::UTF16BE:
		codecName = "UTF-16BE";
		break;
	case CharEncoding::EUC:
		codecName = "EUC-JP";
		break;
	case CharEncoding::SJIS:
		codecName = "Shift-JIS";
	}
	QTextCodec *codec = codecName ? QTextCodec::codecForName(codecName) : 0;
	if( codec == 0 )
		codec = QTextCodec::codecForLocale();
	if( codec == 0 ) {
		QMessageBox::warning(0, "ViVi64",
			QObject::tr("No QTextCodec for %1.")
							 .arg(QString(codecName ? codecName : "Locale")));
		return false;
	}
	buffer = codec->toUnicode(ba);
	//if( cePtr != 0 ) *cePtr = ce;
	//if( wbPtr != 0 ) *wbPtr = bomLength != 0;
	return true;
}
//	改行コード判別
//		0x0d 0x0a を発見した場合 → CharEncoding::CRLF
//		0x0d または 0x0a を先に4個発見 → CharEncoding::CR または CharEncoding::LF
//		それ以外 CharEncoding::UNKNOWN
byte checkNewLineCode(cbyte *ptr, cbyte *endptr, byte charCode)
{
	if( endptr - ptr > 0x10000 )
		endptr = ptr + 0x10000;		//	最大64キロを検査
	int count_0D = 0;
	int count_0A = 0;
	if( charCode == CharEncoding::UTF16LE ) {
		const ushort *wptr = (const ushort *)ptr;
		const ushort *wendptr = (const ushort *)endptr;
		ushort uch;
		while( wptr < wendptr ) {
			if( (uch = *wptr++) == 0x0d ) {
				if( *wptr == 0x0a )
					return CharEncoding::CRLF;
				if( (count_0D += 1) >= 4 )
					return CharEncoding::CR;
			} else if( uch == 0x0a ) {
				if( (count_0A += 1) >= 4 )
					return CharEncoding::LF;
			}
		}
	} else if( charCode == CharEncoding::UTF16BE ) {
		const ushort *wptr = (const ushort *)ptr;
		const ushort *wendptr = (const ushort *)endptr;
		ushort uch;
		while( wptr < wendptr ) {
			if( (uch = *wptr++) == 0x0d00 ) {
				if( *wptr == 0x0a00 )
					return CharEncoding::CRLF;
				if( (count_0D += 1) >= 4 )
					return CharEncoding::CR;
			} else if( uch == 0x0a00 ) {
				if( (count_0A += 1) >= 4 )
					return CharEncoding::LF;
			}
		}
	} else {
		uchar uch;
		while( ptr < endptr ) {
			if( (uch = *ptr++) == 0x0d ) {
				if( *ptr == 0x0a )
					return CharEncoding::CRLF;
				if( (count_0D += 1) >= 4 )
					return CharEncoding::CR;
			} else if( uch == 0x0a ) {
				if( (count_0A += 1) >= 4 )
					return CharEncoding::LF;
			}
		}
	}
	if( count_0A == 0 && count_0D == 0 )
		return CharEncoding::UNKNOWN;
	if( count_0A > count_0D )
		return CharEncoding::LF;
	else
		return CharEncoding::CR;
}
