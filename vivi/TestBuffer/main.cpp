#include <iostream>
#include <assert.h>
#include <QtCore/QCoreApplication>
#include <QString>
#include "../buffer/Buffer.h"

using namespace std;

void test_Buffer();

int main(int argc, char *argv[])
{
	//QCoreApplication a(argc, argv);
	//return a.exec();
	
	test_Buffer();
	
	cout << "\nOK\n";
	return 0;
}
void test_Buffer()
{
	Buffer buf;
	{
		//auto b = buf.isEmpty();
		//auto sz = buf.size();
		//auto ln = buf.lineCount();
		Q_ASSERT( buf.isEmpty() );
		Q_ASSERT( buf.size() == 0 );
		Q_ASSERT( buf.lineCount() == 0 );
		Q_ASSERT( !buf.isModified() );
	}
	//
	QString txt = "abc\nXYZZZ\n";
	buf.insertText(0, (const wchar_t*)txt.data(), txt.size());
	{
		auto b = buf.isEmpty();
		auto sz = buf.size();
		auto ln = buf.lineCount();
		Q_ASSERT( !buf.isEmpty() );
		Q_ASSERT( buf.size() == 10 );
		Q_ASSERT( buf.lineCount() == 2 );
		QString txt2;
		for (int i = 0; i < buf.size(); ++i) {
			txt2 += buf[i];
		}
		Q_ASSERT( txt2 == txt );
		Q_ASSERT( buf.isMatched(L"abc", 0) );
		Q_ASSERT( !buf.isMatched(L"abc", 1) );
		Q_ASSERT( !buf.isMatched(L"ABC", 0) );
		Q_ASSERT( buf.isMatched(L"ZZZ", 6) );
		Q_ASSERT( !buf.isMatched(L"ZZZ", 5) );
		Q_ASSERT( !buf.isMatched(L"zzz", 6) );
		Q_ASSERT( buf.isModified() );
	}
	txt = "12345";
	buf.insertText(6, (const wchar_t*)txt.data(), txt.size());
	{
		auto b = buf.isEmpty();
		auto sz = buf.size();
		auto ln = buf.lineCount();
		Q_ASSERT( !buf.isEmpty() );
		Q_ASSERT( buf.size() == 15 );
		Q_ASSERT( buf.lineCount() == 2 );
		QString txt2;
		for (int i = 0; i < buf.size(); ++i) {
			txt2 += buf[i];
		}
		Q_ASSERT( txt2 == "abc\nXY12345ZZZ\n" );
	}
	
	buf.clear();
	{
		//auto b = buf.isEmpty();
		//auto sz = buf.size();
		//auto ln = buf.lineCount();
		Q_ASSERT( buf.isEmpty() );
		Q_ASSERT( buf.size() == 0 );
		Q_ASSERT( buf.lineCount() == 0 );
	}
	txt = "abc\nXYZZZ";		//	EOF行が空でない場合
	buf.insertText(0, (const wchar_t*)txt.data(), txt.size());
	{
		auto b = buf.isEmpty();
		auto sz = buf.size();
		auto ln = buf.lineCount();
		Q_ASSERT( !buf.isEmpty() );
		Q_ASSERT( buf.size() == 9 );
		Q_ASSERT( buf.lineCount() == 2 );
		QString txt2;
		for (int i = 0; i < buf.size(); ++i) {
			txt2 += buf[i];
		}
		Q_ASSERT( txt2 == txt );
		Q_ASSERT( buf.isModified() );
	}
	buf.undo();
	{
		Q_ASSERT( buf.isEmpty() );
		Q_ASSERT( buf.size() == 0 );
		Q_ASSERT( buf.lineCount() == 0 );
		Q_ASSERT( !buf.isModified() );
	}
	buf.redo();
	{
		Q_ASSERT( !buf.isEmpty() );
		Q_ASSERT( buf.size() == 9 );
		Q_ASSERT( buf.lineCount() == 2 );
		QString txt2;
		for (int i = 0; i < buf.size(); ++i) {
			txt2 += buf[i];
		}
		Q_ASSERT( txt2 == txt );
		Q_ASSERT( buf.isModified() );
	}
}
