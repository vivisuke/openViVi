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
	}
	//
	QString txt = "abc\nXYZZZ\n";
	buf.insertText(0, (const wchar_t*)&txt[0], txt.size());
	{
		auto b = buf.isEmpty();
		auto sz = buf.size();
		auto ln = buf.lineCount();
		Q_ASSERT( !buf.isEmpty() );
		Q_ASSERT( buf.size() != 0 );
		Q_ASSERT( buf.lineCount() == 2 );
		QString txt2;
		for (int i = 0; i < buf.size(); ++i) {
			txt2 += buf[i];
		}
		Q_ASSERT( txt2 == txt );
	}
}
